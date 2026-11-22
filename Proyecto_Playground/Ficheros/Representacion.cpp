#include "Temas.h"
#include "Laboratorio.h"

#include <cmath>

void representacion() {

    laboratorio(
        "02_Representacion",
        {
            "Gris",
            "Canal B",
            "Canal G",
            "Canal R",
            "Brillo y contraste",
            "Gamma",
            "Cuantizacion"
        },
        "Brillo/contraste: ganancia=A/25, "
        "desplazamiento=B-128. Gamma=A/25+0.1.\n"
        "Cuantizacion: niveles=2+A. "
        "La correccion digital no cambia ISO, apertura u obturador.",

        [](const cv::Mat& m, int op, int a, int b) {

            cv::Mat resultado;

            if (op == 0) {

                resultado = gris(m);
            }
            else if (op <= 3) {

                // OpenCV almacena normalmente los canales como BGR.
                cv::extractChannel(
                    m,
                    resultado,
                    op - 1
                );
            }
            else if (op == 4) {

                // salida = ganancia * entrada + desplazamiento.
                m.convertTo(
                    resultado,
                    CV_8U,
                    a / 25.0,
                    b - 128
                );
            }
            else if (op == 5) {

                /*
                 * LUT: tabla que asigna un nuevo valor
                 * a cada intensidad posible.
                 */
                cv::Mat lut(
                    1, 256, CV_8U
                );

                double gamma =
                    a / 25.0 + 0.1;

                for (int i = 0; i < 256; ++i) {

                    lut.at<uchar>(0, i) =
                        cv::saturate_cast<uchar>(
                            255 * std::pow(
                                i / 255.0,
                                gamma
                            )
                        );
                }

                cv::LUT(
                    m, lut, resultado
                );
            }
            else {

                /*
                 * Reducir el número de niveles de gris
                 * permite observar la cuantización.
                 */
                int niveles = 2 + a;

                resultado = gris(m);

                for (
                    int y = 0;
                    y < resultado.rows;
                    ++y
                    ) {
                    for (
                        int x = 0;
                        x < resultado.cols;
                        ++x
                        ) {
                        int valor =
                            resultado.at<uchar>(y, x);

                        resultado.at<uchar>(y, x) =
                            static_cast<uchar>(
                                std::round(
                                    valor *
                                    (niveles - 1) /
                                    255.0
                                ) *
                                255 /
                                (niveles - 1)
                                );
                    }
                }
            }

            return resultado;
        }
    );
}