#include "CameraCalibration.hpp"
#include "VisionUtils.hpp"

#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

/*
 * Esta función solo se utiliza dentro de este archivo.
 *
 * Por eso se coloca dentro de un namespace anónimo:
 * evita que pueda utilizarse accidentalmente desde otros
 * archivos del proyecto.
 */
namespace {

/*
 * Buscar todas las fotografías JPG y PNG guardadas en
 * la carpeta de calibración.
 */
std::vector<cv::String> findCalibrationImages(
    const CalibrationConfig& config
) {
    std::vector<cv::String> files;

    if (!fs::exists(config.imagesFolder)) {
        return files;
    }

    std::vector<cv::String> jpgFiles;
    std::vector<cv::String> pngFiles;

    /*
     * glob() obtiene las rutas de los archivos.
     * No abre las imágenes.
     */
    cv::glob(
        config.imagesFolder + "/*.jpg",
        jpgFiles,
        false
    );

    cv::glob(
        config.imagesFolder + "/*.png",
        pngFiles,
        false
    );

    files.insert(
        files.end(),
        jpgFiles.begin(),
        jpgFiles.end()
    );

    files.insert(
        files.end(),
        pngFiles.begin(),
        pngFiles.end()
    );

    std::sort(
        files.begin(),
        files.end()
    );

    return files;
}

/*
 * Crear el número utilizado en el nombre del archivo:
 *
 * 1  se convierte en "01"
 * 2  se convierte en "02"
 * 10 se convierte en "10"
 *
 * Esto evita utilizar cv::format(), que produjo
 * ambigüedad con tu versión de OpenCV.
 */
std::string createImageNumber(
    int number
) {
    if (number < 10) {
        return "0" + std::to_string(number);
    }

    return std::to_string(number);
}

} // Fin del namespace anónimo.

// =========================================================
// OPCIÓN 1: CAPTURAR LAS FOTOGRAFÍAS
// =========================================================

void captureCalibrationImages(
    const CalibrationConfig& config
) {
    /*
     * Crear la carpeta si todavía no existe.
     * Si ya existe, no borra su contenido.
     */
    fs::create_directories(
        config.imagesFolder
    );

    std::cout
        << "\nLas imagenes se guardaran en:\n"
        << fs::absolute(config.imagesFolder).string()
        << "\n";

    cv::VideoCapture camera(
        config.cameraIndex
    );

    if (!camera.isOpened()) {
        std::cerr
            << "No se pudo abrir la camara.\n";

        return;
    }

    int savedImages = 0;

    std::cout
        << "\nHaz clic sobre la ventana de la camara.\n"
        << "ESPACIO: guardar una imagen.\n"
        << "ESC: regresar al menu.\n";

    while (true) {
        cv::Mat frame;

        // Leer el siguiente frame de la cámara.
        camera.read(frame);

        if (frame.empty()) {
            std::cerr
                << "Se recibio un frame vacio.\n";

            break;
        }

        std::vector<cv::Point2f> corners;

        bool found = detectCorners(
            frame,
            corners,
            config
        );

        /*
         * Se crea una copia para dibujar sobre ella.
         * Así la fotografía guardada permanece limpia.
         */
        cv::Mat preview = frame.clone();

        if (found) {
            /*
             * Esta función no detecta el tablero.
             * Solamente dibuja las esquinas que encontró
             * detectCorners().
             */
            cv::drawChessboardCorners(
                preview,
                config.patternSize,
                corners,
                true
            );

            drawText(
                preview,
                "Tablero detectado",
                1,
                cv::Scalar(0, 255, 0)
            );
        }
        else {
            drawText(
                preview,
                "Tablero no detectado",
                1,
                cv::Scalar(0, 0, 255)
            );
        }

        drawText(
            preview,
            "ESPACIO: guardar    ESC: menu",
            0
        );

        drawText(
            preview,
            "Imagenes guardadas: " +
                std::to_string(savedImages),
            2
        );

        cv::imshow(
            "Captura para calibracion",
            preview
        );

        /*
         * waitKeyEx() recibe las teclas cuando la ventana
         * de OpenCV tiene el foco.
         */
        int key = cv::waitKeyEx(30);
        int basicKey = key & 0xFF;

        // ESC tiene el código 27.
        if (basicKey == 27) {
            break;
        }

        // La barra espaciadora tiene el código 32.
        if (basicKey == 32) {

            if (!found) {
                std::cout
                    << "No se guardo la imagen porque "
                    << "el tablero no fue detectado.\n";

                continue;
            }

            savedImages++;

            std::string filename =
                config.imagesFolder +
                "/calibration_" +
                createImageNumber(savedImages) +
                ".jpg";

            /*
             * Guardar el frame original sin los puntos
             * ni los mensajes dibujados.
             */
            bool saved = cv::imwrite(
                filename,
                frame
            );

            if (saved) {
                std::cout
                    << "Imagen guardada: "
                    << fs::absolute(filename).string()
                    << "\n";
            }
            else {
                savedImages--;

                std::cerr
                    << "No se pudo guardar la imagen.\n";
            }
        }
    }

    camera.release();
    cv::destroyAllWindows();
}

// =========================================================
// OPCIÓN 2: CALIBRAR LA CÁMARA
// =========================================================

void calibrateCameraFromImages(
    const CalibrationConfig& config
) {
    std::vector<cv::String> imageFiles =
        findCalibrationImages(config);

    if (imageFiles.empty()) {
        std::cerr
            << "No se encontraron imagenes en "
            << config.imagesFolder
            << ".\n";

        std::cerr
            << "Usa primero la opcion 1.\n";

        return;
    }

    /*
     * Puntos conocidos del tablero en el mundo real.
     */
    std::vector<cv::Point3f> boardPoints =
        createObjectPoints(config);

    /*
     * objectPoints:
     * coordenadas reales del tablero en milímetros.
     *
     * imagePoints:
     * coordenadas detectadas en píxeles.
     */
    std::vector<std::vector<cv::Point3f>>
        objectPoints;

    std::vector<std::vector<cv::Point2f>>
        imagePoints;

    std::vector<cv::String> acceptedFiles;

    cv::Size imageSize;

    for (
        const cv::String& filename :
        imageFiles
    ) {
        cv::Mat image = cv::imread(filename);

        if (image.empty()) {
            std::cerr
                << "No se pudo leer: "
                << filename
                << "\n";

            continue;
        }

        /*
         * Todas las fotografías deben tener la misma
         * resolución.
         */
        if (imageSize.empty()) {
            imageSize = image.size();
        }
        else if (image.size() != imageSize) {
            std::cerr
                << "Imagen ignorada por tener otra "
                << "resolucion: "
                << filename
                << "\n";

            continue;
        }

        std::vector<cv::Point2f> corners;

        bool found = detectCorners(
            image,
            corners,
            config
        );

        if (!found) {
            std::cerr
                << "No se detecto el tablero en: "
                << filename
                << "\n";

            continue;
        }

        /*
         * Asociar el mismo tablero real con las esquinas
         * observadas en esta fotografía.
         */
        objectPoints.push_back(
            boardPoints
        );

        imagePoints.push_back(
            corners
        );

        acceptedFiles.push_back(
            filename
        );

        std::cout
            << "Imagen aceptada: "
            << filename
            << "\n";
    }

    if (
        imagePoints.size() <
        config.minimumValidImages
    ) {
        std::cerr
            << "\nSolo hay "
            << imagePoints.size()
            << " imagenes validas.\n";

        std::cerr
            << "Se requieren al menos "
            << config.minimumValidImages
            << " y se recomiendan entre 15 y 25.\n";

        return;
    }

    /*
     * Matriz intrínseca:
     *
     * [ fx   0  cx ]
     * [  0  fy  cy ]
     * [  0   0   1 ]
     */
    cv::Mat cameraMatrix =
        cv::Mat::eye(3, 3, CV_64F);

    /*
     * Coeficientes habituales:
     *
     * k1, k2, p1, p2, k3
     */
    cv::Mat distortionCoefficients;

    /*
     * Posición e inclinación del tablero en cada foto.
     */
    std::vector<cv::Mat> rotationVectors;
    std::vector<cv::Mat> translationVectors;

    /*
     * Esta es la función matemática principal.
     *
     * Relaciona los puntos reales del tablero con las
     * coordenadas observadas en todas las imágenes.
     */
    double rms = cv::calibrateCamera(
        objectPoints,
        imagePoints,
        imageSize,
        cameraMatrix,
        distortionCoefficients,
        rotationVectors,
        translationVectors
    );

    // Calcular el error medio de reproyección.
    double totalSquaredError = 0.0;
    std::size_t totalPointCount = 0;

    for (
        std::size_t i = 0;
        i < objectPoints.size();
        i++
    ) {
        std::vector<cv::Point2f>
            projectedPoints;

        /*
         * Predecir dónde deberían aparecer las esquinas
         * usando los parámetros calculados.
         */
        cv::projectPoints(
            objectPoints[i],
            rotationVectors[i],
            translationVectors[i],
            cameraMatrix,
            distortionCoefficients,
            projectedPoints
        );

        /*
         * Comparar las posiciones predichas con las
         * esquinas realmente detectadas.
         */
        double error = cv::norm(
            imagePoints[i],
            projectedPoints,
            cv::NORM_L2
        );

        totalSquaredError += error * error;

        totalPointCount +=
            objectPoints[i].size();
    }

    double meanReprojectionError =
        std::sqrt(
            totalSquaredError /
            static_cast<double>(
                totalPointCount
            )
        );

    /*
     * Abrir el YAML en modo escritura.
     */
    cv::FileStorage output(
        config.calibrationFile,
        cv::FileStorage::WRITE
    );

    if (!output.isOpened()) {
        std::cerr
            << "No se pudo crear "
            << config.calibrationFile
            << ".\n";

        return;
    }

    // Guardar información general.
    output
        << "image_width"
        << imageSize.width;

    output
        << "image_height"
        << imageSize.height;

    output
        << "pattern_columns"
        << config.patternSize.width;

    output
        << "pattern_rows"
        << config.patternSize.height;

    output
        << "square_size_mm"
        << config.squareSizeMm;

    output
        << "valid_images"
        << static_cast<int>(
            imagePoints.size()
        );

    // Guardar los parámetros importantes.
    output
        << "camera_matrix"
        << cameraMatrix;

    output
        << "distortion_coefficients"
        << distortionCoefficients;

    output
        << "rms"
        << rms;

    output
        << "mean_reprojection_error"
        << meanReprojectionError;

    output.release();

    /*
     * Crear una comparación usando la primera imagen
     * válida de la calibración.
     */
    cv::Mat sample =
        cv::imread(acceptedFiles.front());

    cv::Mat corrected;

    cv::undistort(
        sample,
        corrected,
        cameraMatrix,
        distortionCoefficients
    );

    cv::Mat comparison;

    cv::hconcat(
        sample,
        corrected,
        comparison
    );

    cv::imwrite(
        "calibration_comparison.jpg",
        comparison
    );

    // Mostrar los resultados en la consola.
    std::cout
        << std::fixed
        << std::setprecision(6);

    std::cout
        << "\n=== CALIBRACION TERMINADA ===\n";

    std::cout
        << "Imagenes validas: "
        << imagePoints.size()
        << "\n";

    std::cout
        << "RMS: "
        << rms
        << " px\n";

    std::cout
        << "Error medio de reproyeccion: "
        << meanReprojectionError
        << " px\n";

    std::cout
        << "\nMatriz K:\n"
        << cameraMatrix
        << "\n";

    std::cout
        << "\nCoeficientes de distorsion:\n"
        << distortionCoefficients
        << "\n";

    std::cout
        << "\nParametros guardados en: "
        << config.calibrationFile
        << "\n";

    std::cout
        << "Comparacion guardada en: "
        << "calibration_comparison.jpg\n";
}

// =========================================================
// OPCIÓN 3: PROBAR LA CORRECCIÓN EN TIEMPO REAL
// =========================================================

void testCalibration(
    const CalibrationConfig& config
) {
    /*
     * Abrir el YAML en modo lectura.
     */
    cv::FileStorage input(
        config.calibrationFile,
        cv::FileStorage::READ
    );

    if (!input.isOpened()) {
        std::cerr
            << "No se encontro "
            << config.calibrationFile
            << ".\n";

        std::cerr
            << "Usa primero la opcion 2.\n";

        return;
    }

    cv::Mat cameraMatrix;
    cv::Mat distortionCoefficients;

    int calibratedWidth = 0;
    int calibratedHeight = 0;

    /*
     * Extraer los datos del YAML.
     *
     * Los nombres deben coincidir exactamente con los
     * utilizados cuando se escribió el archivo.
     */
    input["camera_matrix"]
        >> cameraMatrix;

    input["distortion_coefficients"]
        >> distortionCoefficients;

    input["image_width"]
        >> calibratedWidth;

    input["image_height"]
        >> calibratedHeight;

    input.release();

    if (
        cameraMatrix.empty() ||
        distortionCoefficients.empty()
    ) {
        std::cerr
            << "El archivo de calibracion "
            << "esta incompleto.\n";

        return;
    }

    cv::VideoCapture camera(
        config.cameraIndex
    );

    if (!camera.isOpened()) {
        std::cerr
            << "No se pudo abrir la camara.\n";

        return;
    }

    /*
     * Solicitar la misma resolución usada durante la
     * calibración.
     */
    if (
        calibratedWidth > 0 &&
        calibratedHeight > 0
    ) {
        camera.set(
            cv::CAP_PROP_FRAME_WIDTH,
            calibratedWidth
        );

        camera.set(
            cv::CAP_PROP_FRAME_HEIGHT,
            calibratedHeight
        );
    }

    cv::Mat mapX;
    cv::Mat mapY;

    cv::Size previousSize;

    std::cout
        << "\nESC: regresar al menu.\n";

    while (true) {
        cv::Mat original;

        camera.read(original);

        if (original.empty()) {
            break;
        }

        /*
         * Calcular los mapas al recibir el primer frame.
         * Solo se recalculan si cambia la resolución.
         */
        if (
            original.size() != previousSize
        ) {
            previousSize = original.size();

            /*
             * alpha = 1.0 conserva más imagen y puede
             * producir bordes negros.
             *
             * alpha = 0.0 recorta las zonas inválidas.
             */
            cv::Mat newCameraMatrix =
                cv::getOptimalNewCameraMatrix(
                    cameraMatrix,
                    distortionCoefficients,
                    previousSize,
                    1.0,
                    previousSize
                );

            /*
             * Crear los mapas que indican cómo debe
             * reorganizarse cada píxel.
             */
            cv::initUndistortRectifyMap(
                cameraMatrix,
                distortionCoefficients,
                cv::Mat(),
                newCameraMatrix,
                previousSize,
                CV_32FC1,
                mapX,
                mapY
            );
        }

        cv::Mat corrected;

        /*
         * Aplicar los mapas a cada frame.
         *
         * Esta operación se repite en tiempo real.
         */
        cv::remap(
            original,
            corrected,
            mapX,
            mapY,
            cv::INTER_LINEAR
        );

        cv::Mat originalWithText =
            original.clone();

        drawText(
            originalWithText,
            "Original",
            0,
            cv::Scalar(0, 200, 255)
        );

        drawText(
            corrected,
            "Corregida",
            0,
            cv::Scalar(0, 255, 0)
        );

        cv::Mat comparison;

        cv::hconcat(
            originalWithText,
            corrected,
            comparison
        );

        drawText(
            comparison,
            "ESC: regresar al menu",
            1
        );

        cv::imshow(
            "Prueba de calibracion",
            comparison
        );

        int key =
            cv::waitKeyEx(30) & 0xFF;

        if (key == 27) {
            break;
        }
    }

    camera.release();
    cv::destroyAllWindows();
}