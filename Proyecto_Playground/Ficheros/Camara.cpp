#include "Temas.h"
#include "Laboratorio.h"

#include <opencv2/calib.hpp>
#include <opencv2/objdetect.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace {

    /*
     * Configuración del tablero.
     *
     * 8x8 casillas físicas = 7x7 esquinas interiores.
     * Cambia estos valores si utilizas otro tablero.
     */
    const cv::Size patron(7, 7);
    const float lado = 21.0f;

    const std::string carpeta =
        "calibration_images";

    const std::string archivo =
        "camera_calibration.yaml";

    bool esquinas(
        const cv::Mat& imagen,
        std::vector<cv::Point2f>& puntos
    ) {
        cv::Mat g = gris(imagen);

        bool encontrado =
            cv::findChessboardCorners(
                g,
                patron,
                puntos,
                cv::CALIB_CB_ADAPTIVE_THRESH |
                cv::CALIB_CB_NORMALIZE_IMAGE
            );

        if (encontrado) {

            cv::cornerSubPix(
                g,
                puntos,
                { 11, 11 },
                { -1, -1 },
                cv::TermCriteria(
                    cv::TermCriteria::COUNT |
                    cv::TermCriteria::EPS,
                    30,
                    0.001
                )
            );
        }

        return encontrado;
    }

    std::vector<std::string> fotos() {

        std::vector<std::string> rutas;

        if (!fs::exists(carpeta)) {
            return rutas;
        }

        /*
         * Recorrer la carpeta permite aceptar JPG, JPEG
         * y PNG, incluso con extensión en mayúsculas.
         */
        for (
            const auto& entrada :
            fs::directory_iterator(carpeta)
            ) {
            std::string extension =
                entrada.path().extension().string();

            std::transform(
                extension.begin(),
                extension.end(),
                extension.begin(),
                [](unsigned char c) {
                    return static_cast<char>(
                        std::tolower(c)
                        );
                }
            );

            if (
                entrada.is_regular_file() &&
                (
                    extension == ".jpg" ||
                    extension == ".png" ||
                    extension == ".jpeg"
                    )
                ) {
                rutas.push_back(
                    entrada.path().string()
                );
            }
        }

        std::sort(
            rutas.begin(), rutas.end()
        );

        return rutas;
    }

    void capturar() {

        fs::create_directories(carpeta);

        cv::VideoCapture cap(0);

        if (!cap.isOpened()) {
            throw std::runtime_error(
                "No se pudo abrir la camara."
            );
        }

        std::cout
            << "Destino: "
            << fs::absolute(carpeta)
            << "\n";

        int numero = 1;

        while (true) {

            cv::Mat imagen;

            if (!cap.read(imagen)) {
                break;
            }

            std::vector<cv::Point2f> puntos;

            bool encontrado =
                esquinas(imagen, puntos);

            cv::Mat vista = imagen.clone();

            if (encontrado) {

                cv::drawChessboardCorners(
                    vista,
                    patron,
                    puntos,
                    true
                );
            }

            texto(
                vista,
                encontrado
                ? "Detectado | ESPACIO: guardar"
                : "Tablero no detectado",
                0
            );

            texto(
                vista,
                "ESC: menu | Haz clic en esta ventana para usar teclado",
                1
            );

            cv::imshow("Tablero", vista);

            int tecla = cv::waitKey(20) & 255;

            if (
                tecla == 27 ||
                cv::getWindowProperty(
                    "Tablero",
                    cv::WND_PROP_VISIBLE
                ) < 1
                ) {
                break;
            }

            if (
                tecla == 32 &&
                encontrado
                ) {
                std::string destino;

                /*
                 * Buscar un nombre libre.
                 * Las nuevas sesiones no sobrescriben fotos.
                 */
                do {
                    destino =
                        carpeta +
                        "/calibration_" +
                        std::to_string(numero++) +
                        ".png";
                } while (fs::exists(destino));

                // Guardar la imagen sin dibujos ni texto.
                if (
                    cv::imwrite(
                        destino,
                        imagen
                    )
                    ) {
                    std::cout
                        << "Guardada "
                        << destino
                        << "\n";
                }
            }
        }

        cap.release();
        cv::destroyAllWindows();
    }

    void calibrar() {

        std::vector<cv::Point3f> objeto;

        // Coordenadas conocidas del tablero, expresadas en mm.
        for (int y = 0; y < patron.height; ++y) {
            for (int x = 0; x < patron.width; ++x) {

                objeto.emplace_back(
                    x * lado,
                    y * lado,
                    0.0f
                );
            }
        }

        std::vector<std::vector<cv::Point3f>> objetos;
        std::vector<std::vector<cv::Point2f>> imagenes;

        std::vector<std::string> aceptadas;

        cv::Size tamano;

        for (const auto& ruta : fotos()) {

            cv::Mat imagen = cv::imread(ruta);

            if (imagen.empty()) {
                continue;
            }

            if (
                !tamano.empty() &&
                imagen.size() != tamano
                ) {
                std::cout
                    << "Otra resolucion: "
                    << ruta << "\n";

                continue;
            }

            std::vector<cv::Point2f> puntos;

            if (!esquinas(imagen, puntos)) {

                std::cout
                    << "Sin patron: "
                    << ruta << "\n";

                continue;
            }

            // La resolución se fija con la primera fotografía válida.
            tamano = imagen.size();

            objetos.push_back(objeto);
            imagenes.push_back(puntos);
            aceptadas.push_back(ruta);
        }

        if (imagenes.size() < 10) {

            throw std::runtime_error(
                "Faltan fotos validas: minimo 10; "
                "recomendado 15 a 25."
            );
        }

        cv::Mat K;
        cv::Mat D;

        std::vector<cv::Mat> rotaciones;
        std::vector<cv::Mat> traslaciones;

        /*
         * Calibración estándar de una webcam.
         *
         * K: matriz intrínseca.
         * D: coeficientes de distorsión.
         */
        double rms = cv::calibrateCamera(
            objetos,
            imagenes,
            tamano,
            K,
            D,
            rotaciones,
            traslaciones
        );

        if (
            !std::isfinite(rms) ||
            !cv::checkRange(K) ||
            !cv::checkRange(D)
            ) {
            throw std::runtime_error(
                "Calibracion numericamente invalida."
            );
        }

        /*
         * Guardar el error por fotografía ayuda a localizar
         * capturas problemáticas.
         */
        std::ofstream csv(
            "errores_calibracion.csv"
        );

        csv << "foto,rms_px\n";

        for (
            size_t i = 0;
            i < imagenes.size();
            ++i
            ) {
            std::vector<cv::Point2f> proyectados;

            cv::projectPoints(
                objetos[i],
                rotaciones[i],
                traslaciones[i],
                K,
                D,
                proyectados
            );

            double error =
                cv::norm(
                    imagenes[i],
                    proyectados,
                    cv::NORM_L2
                ) /
                std::sqrt(
                    static_cast<double>(
                        imagenes[i].size()
                        )
                );

            csv
                << '"' << aceptadas[i]
                << "\"," << error << "\n";
        }

        cv::FileStorage salida(
            archivo,
            cv::FileStorage::WRITE
        );

        if (!salida.isOpened()) {
            throw std::runtime_error(
                "No se pudo escribir el YAML."
            );
        }

        salida
            << "camera_matrix" << K
            << "distortion_coefficients" << D
            << "image_width" << tamano.width
            << "image_height" << tamano.height
            << "pattern_columns" << patron.width
            << "pattern_rows" << patron.height
            << "square_size_mm" << lado
            << "rms" << rms
            << "valid_images"
            << static_cast<int>(imagenes.size())
            << "rotation_vectors" << rotaciones
            << "translation_vectors" << traslaciones;

        salida.release();

        std::cout
            << "RMS=" << rms << " px\n"
            << "K=\n" << K
            << "\nD=" << D
            << "\nGuardado " << archivo << "\n";
    }

    void corregir() {

        cv::FileStorage entrada(
            archivo,
            cv::FileStorage::READ
        );

        if (!entrada.isOpened()) {
            throw std::runtime_error(
                "Primero calibra o copia tu YAML aqui."
            );
        }

        cv::Mat K;
        cv::Mat D;

        int ancho = 0;
        int alto = 0;

        entrada["camera_matrix"] >> K;
        entrada["distortion_coefficients"] >> D;
        entrada["image_width"] >> ancho;
        entrada["image_height"] >> alto;

        if (
            K.rows != 3 ||
            K.cols != 3 ||
            D.empty() ||
            ancho <= 0 ||
            alto <= 0
            ) {
            throw std::runtime_error(
                "YAML incompleto: requiere K, D y resolucion."
            );
        }

        cv::Mat mapaX;
        cv::Mat mapaY;

        /*
         * Los mapas se calculan una sola vez.
         * K pertenece a la resolución guardada en el YAML.
         */
        cv::initUndistortRectifyMap(
            K,
            D,
            cv::Mat(),
            K,
            { ancho, alto },
            CV_32FC1,
            mapaX,
            mapaY
        );

        laboratorio(
            "04_Correccion",
            {
                "Corregida",
                "Diferencia"
            },
            "Usa la MISMA camara/resolucion del YAML "
            "o una foto de esa camara.\n"
            "Operacion 1 muestra diferencia absoluta. "
            "A y B sin uso.",

            [=](const cv::Mat& m, int op, int, int) {

                if (
                    m.size() !=
                    cv::Size(ancho, alto)
                    ) {
                    throw std::runtime_error(
                        "La resolucion no coincide con el YAML."
                    );
                }

                cv::Mat resultado;

                // Aplicar la corrección a cada imagen o frame.
                cv::remap(
                    m,
                    resultado,
                    mapaX,
                    mapaY,
                    cv::INTER_LINEAR
                );

                if (op == 1) {

                    cv::absdiff(
                        m,
                        resultado,
                        resultado
                    );
                }

                return resultado;
            }
        );
    }

}

void camara() {

    while (true) {

        std::cout
            << "\n1 Capturar"
            << " | 2 Calibrar"
            << " | 3 Corregir desde YAML"
            << " | 4 Modelos didacticos"
            << " | 0 Volver: ";

        std::string opcion;
        std::getline(std::cin, opcion);

        if (
            opcion == "0" ||
            !std::cin
            ) {
            return;
        }

        if (opcion == "1") {
            capturar();
        }
        else if (opcion == "2") {
            calibrar();
        }
        else if (opcion == "3") {
            corregir();
        }
        else if (opcion == "4") {

            laboratorio(
                "04_Modelos",
                {
                    "Radial artificial",
                    "Angular artificial"
                },
                "Simulaciones geometricas; NO son calibraciones reales. "
                "A controla deformacion.\n"
                "Pinhole: r=f*tan(theta); equidistante: r=f*theta. "
                "Webcam normal: calibracion estandar.",

                [](const cv::Mat& m, int op, int a, int) {

                    cv::Mat mapaX(
                        m.size(), CV_32F
                    );

                    cv::Mat mapaY(
                        m.size(), CV_32F
                    );

                    cv::Mat resultado;

                    float focal =
                        static_cast<float>(
                            std::max(m.cols, m.rows)
                            );

                    /*
                     * Dos remapeos artificiales permiten observar
                     * la diferencia entre deformaciones radiales
                     * y angulares. No estiman parámetros de una lente.
                     */
                    for (int y = 0; y < m.rows; ++y) {
                        for (int x = 0; x < m.cols; ++x) {

                            float u =
                                (x - m.cols / 2.0f) / focal;

                            float v =
                                (y - m.rows / 2.0f) / focal;

                            float distancia =
                                std::sqrt(u * u + v * v);

                            float factor = 1;

                            if (op == 0) {

                                factor =
                                    1 +
                                    (a - 50) / 25.0f *
                                    distancia * distancia;
                            }
                            else if (distancia > 0) {

                                float escala =
                                    1 + a / 25.0f;

                                factor =
                                    std::atan(
                                        distancia * escala
                                    ) /
                                    (distancia * escala);
                            }

                            mapaX.at<float>(y, x) =
                                m.cols / 2.0f +
                                u * focal * factor;

                            mapaY.at<float>(y, x) =
                                m.rows / 2.0f +
                                v * focal * factor;
                        }
                    }

                    cv::remap(
                        m,
                        resultado,
                        mapaX,
                        mapaY,
                        cv::INTER_LINEAR
                    );

                    return resultado;
                }
            );
        }
    }
}