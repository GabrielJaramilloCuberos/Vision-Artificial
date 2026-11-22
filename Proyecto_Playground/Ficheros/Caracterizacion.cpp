#include "Temas.h"
#include "Laboratorio.h"

#include <opencv2/features.hpp>

#include <algorithm>
#include <cmath>

void caracterizacion() {

    laboratorio(
        "11_Caracterizacion",
        {
            "Geometria y PCA",
            "Estadisticas y textura",
            "Harris",
            "SIFT",
            "ORB",
            "Matching SIFT",
            "Matching ORB"
        },
        "Geometria: B=umbral de objetos claros; "
        "A=area minima en decenas de pixeles.\n"
        "Matching: comparar con copia rotada, angulo=A-50. "
        "SIFT=L2, ORB=Hamming.\n"
        "SURF/BRIEF independientes requieren contrib; "
        "ORB utiliza BRIEF orientado.",

        [](const cv::Mat& m, int op, int a, int b) {

            cv::Mat g = gris(m);
            cv::Mat resultado = m.clone();

            if (op == 0) {

                cv::Mat binaria;
                cv::Mat etiquetas;
                cv::Mat estadisticas;
                cv::Mat centroides;

                cv::threshold(
                    g, binaria,
                    b, 255,
                    cv::THRESH_BINARY
                );

                int cantidad =
                    cv::connectedComponentsWithStats(
                        binaria,
                        etiquetas,
                        estadisticas,
                        centroides
                    );

                int cuenta = 0;

                for (int id = 1; id < cantidad; ++id) {

                    // Área exacta del componente: cantidad de píxeles.
                    int area = estadisticas.at<int>(
                        id, cv::CC_STAT_AREA
                    );

                    if (area < 10 * a + 1) {
                        continue;
                    }

                    cv::Mat mascara =
                        etiquetas == id;

                    std::vector<std::vector<cv::Point>>
                        contornos;

                    cv::findContours(
                        mascara,
                        contornos,
                        cv::RETR_EXTERNAL,
                        cv::CHAIN_APPROX_NONE
                    );

                    if (contornos.empty()) {
                        continue;
                    }

                    auto mayor = std::max_element(
                        contornos.begin(),
                        contornos.end(),
                        [](const auto& x, const auto& y) {
                            return
                                cv::contourArea(x) <
                                cv::contourArea(y);
                        }
                    );

                    /*
                     * El área geométrica del contorno y el
                     * recuento de píxeles no son idénticos.
                     *
                     * Circularidad = 4*pi*área_contorno/perímetro².
                     */
                    double areaContorno =
                        cv::contourArea(*mayor);

                    double perimetro =
                        cv::arcLength(*mayor, true);

                    double circularidad =
                        perimetro > 0
                        ? 4 * CV_PI * areaContorno /
                        (perimetro * perimetro)
                        : 0;

                    /*
                     * PCA obtiene la dirección principal
                     * de distribución de los píxeles del objeto.
                     */
                    std::vector<cv::Point> puntos;

                    cv::findNonZero(
                        mascara, puntos
                    );

                    cv::Mat datos(
                        static_cast<int>(puntos.size()),
                        2,
                        CV_64F
                    );

                    for (int i = 0; i < datos.rows; ++i) {

                        datos.at<double>(i, 0) =
                            puntos[i].x;

                        datos.at<double>(i, 1) =
                            puntos[i].y;
                    }

                    cv::PCA pca(
                        datos,
                        cv::Mat(),
                        cv::PCA::DATA_AS_ROW
                    );

                    cv::Point centro(
                        cvRound(
                            centroides.at<double>(id, 0)
                        ),
                        cvRound(
                            centroides.at<double>(id, 1)
                        )
                    );

                    cv::Point extremo(
                        centro.x + cvRound(
                            50 *
                            pca.eigenvectors.at<double>(0, 0)
                        ),
                        centro.y + cvRound(
                            50 *
                            pca.eigenvectors.at<double>(0, 1)
                        )
                    );

                    cv::line(
                        resultado,
                        centro,
                        extremo,
                        { 0, 255, 255 },
                        2
                    );

                    cv::drawContours(
                        resultado,
                        contornos,
                        -1,
                        { 255, 0, 255 },
                        2
                    );

                    cv::putText(
                        resultado,
                        "A=" + std::to_string(area) +
                        " C=" +
                        std::to_string(
                            circularidad
                        ).substr(0, 4),
                        centro,
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.45,
                        { 255, 255, 255 },
                        1
                    );

                    ++cuenta;
                }

                texto(
                    resultado,
                    "Objetos: " +
                    std::to_string(cuenta) +
                    " | A=pixeles, C=circularidad",
                    1
                );

                return resultado;
            }

            if (op == 1) {

                cv::Scalar media;
                cv::Scalar desviacion;

                cv::meanStdDev(
                    g, media, desviacion
                );

                double histograma[256] = {};
                double glcm[16][16] = {};

                double entropia = 0;
                double contraste = 0;
                double homogeneidad = 0;
                double uniformidad = 0;

                for (int y = 0; y < g.rows; ++y) {
                    for (int x = 0; x < g.cols; ++x) {

                        int valor =
                            g.at<uchar>(y, x);

                        ++histograma[valor];

                        /*
                         * GLCM horizontal con 16 niveles:
                         * contar pares de intensidades vecinas.
                         */
                        if (x + 1 < g.cols) {

                            ++glcm[
                                valor / 16
                            ][
                                g.at<uchar>(y, x + 1) / 16
                            ];
                        }
                    }
                }

                for (double frecuencia : histograma) {

                    if (frecuencia > 0) {

                        double p =
                            frecuencia / g.total();

                        entropia -=
                            p * std::log2(p);
                    }
                }

                double pares =
                    static_cast<double>(g.rows) *
                    (g.cols - 1);

                for (int i = 0; i < 16; ++i) {
                    for (int j = 0; j < 16; ++j) {

                        double p =
                            glcm[i][j] / pares;

                        contraste +=
                            (i - j) * (i - j) * p;

                        homogeneidad +=
                            p / (1 + std::abs(i - j));

                        uniformidad += p * p;
                    }
                }

                texto(
                    resultado,
                    "Media=" + std::to_string(media[0]) +
                    " SD=" + std::to_string(desviacion[0]),
                    1
                );

                texto(
                    resultado,
                    "Entropia (bits)=" +
                    std::to_string(entropia),
                    2
                );

                texto(
                    resultado,
                    "GLCM horizontal 16 niveles, contraste=" +
                    std::to_string(contraste),
                    3
                );

                texto(
                    resultado,
                    "Homogeneidad=" +
                    std::to_string(homogeneidad) +
                    " ASM=" +
                    std::to_string(uniformidad),
                    4
                );

                return resultado;
            }

            if (op == 2) {

                cv::Mat respuesta;

                cv::cornerHarris(
                    g, respuesta,
                    2, 3, 0.04
                );

                double maximo;

                cv::minMaxLoc(
                    respuesta,
                    nullptr,
                    &maximo
                );

                for (
                    int y = 0;
                    y < respuesta.rows;
                    ++y
                    ) {
                    for (
                        int x = 0;
                        x < respuesta.cols;
                        ++x
                        ) {
                        if (
                            respuesta.at<float>(y, x) >
                            maximo * (0.01 + a / 100.0)
                            ) {
                            cv::circle(
                                resultado,
                                { x, y },
                                2,
                                { 0, 0, 255 }
                            );
                        }
                    }
                }

                return resultado;
            }

            bool usarOrb =
                op == 4 || op == 6;

            cv::Ptr<cv::Feature2D> detector;

            if (usarOrb) {
                detector = cv::ORB::create(500);
            }
            else {
                detector = cv::SIFT::create(500);
            }

            std::vector<cv::KeyPoint> puntos1;
            std::vector<cv::KeyPoint> puntos2;

            cv::Mat descriptores1;
            cv::Mat descriptores2;

            detector->detectAndCompute(
                g,
                cv::noArray(),
                puntos1,
                descriptores1
            );

            if (op == 3 || op == 4) {

                cv::drawKeypoints(
                    m,
                    puntos1,
                    resultado
                );

                return resultado;
            }

            /*
             * Crear una segunda vista conocida para probar
             * correspondencias ante rotación y escala.
             */
            cv::Mat transformacion =
                cv::getRotationMatrix2D(
                    {
                        m.cols / 2.0f,
                        m.rows / 2.0f
                    },
                    a - 50,
                    0.9
                );

            cv::Mat segunda;

            cv::warpAffine(
                m,
                segunda,
                transformacion,
                m.size()
            );

            detector->detectAndCompute(
                gris(segunda),
                cv::noArray(),
                puntos2,
                descriptores2
            );

            if (
                descriptores1.empty() ||
                descriptores2.rows < 2
                ) {
                texto(
                    resultado,
                    "Faltan descriptores: prueba otra imagen",
                    1
                );

                return resultado;
            }

            /*
             * SIFT: descriptores float, distancia L2.
             * ORB: descriptores binarios, distancia Hamming.
             */
            cv::BFMatcher matcher(
                usarOrb
                ? cv::NORM_HAMMING
                : cv::NORM_L2
            );

            std::vector<std::vector<cv::DMatch>>
                candidatos;

            matcher.knnMatch(
                descriptores1,
                descriptores2,
                candidatos,
                2
            );

            std::vector<cv::DMatch> buenos;

            // Ratio test: descartar coincidencias ambiguas.
            for (const auto& pareja : candidatos) {

                if (
                    pareja.size() == 2 &&
                    pareja[0].distance <
                    0.75f * pareja[1].distance
                    ) {
                    buenos.push_back(
                        pareja[0]
                    );
                }
            }

            // Limitar líneas para mantener legible la comparación.
            if (buenos.size() > 70) {
                buenos.resize(70);
            }

            cv::drawMatches(
                m,
                puntos1,
                segunda,
                puntos2,
                buenos,
                resultado
            );

            return resultado;
        }
    );
}