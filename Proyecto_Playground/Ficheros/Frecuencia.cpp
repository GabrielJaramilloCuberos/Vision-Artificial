#include "Temas.h"
#include "Laboratorio.h"

#include <algorithm>
#include <cmath>

void frecuencia() {

    laboratorio(
        "06_Frecuencia",
        {
            "Espectro DFT",
            "Paso bajo",
            "Paso alto",
            "Paso banda",
            "DCT bloques 8x8",
            "Ortogonalidad"
        },
        "DFT: A=radio exterior, B=radio interior "
        "(% Nyquist). DCT: A=coeficientes conservados.\n"
        "La DCT demuestra compresion con perdida; "
        "no implementa un codificador JPEG.",

        [](const cv::Mat& m, int op, int a, int b) {

            cv::Mat g = gris(m);
            cv::Mat flotante;

            g.convertTo(
                flotante, CV_32F
            );

            if (op == 5) {

                /*
                 * Producto interno discreto entre seno y coseno.
                 * En un período completo debe acercarse a cero.
                 */
                double producto = 0;

                for (int i = 0; i < 128; ++i) {

                    double t =
                        2 * CV_PI * i / 128;

                    producto +=
                        std::sin(t) * std::cos(t);
                }

                cv::Mat resultado = m.clone();

                texto(
                    resultado,
                    "Suma sin(t)*cos(t): " +
                    std::to_string(producto),
                    1
                );

                texto(
                    resultado,
                    "128 muestras de un periodo: producto cercano a cero",
                    2
                );

                return resultado;
            }

            if (op == 4) {

                /*
                 * Procesar bloques 8x8.
                 * Se conservan únicamente coeficientes
                 * de baja frecuencia en cada bloque.
                 */
                cv::Mat rellena;

                cv::copyMakeBorder(
                    flotante,
                    rellena,
                    0,
                    (8 - flotante.rows % 8) % 8,
                    0,
                    (8 - flotante.cols % 8) % 8,
                    cv::BORDER_REFLECT
                );

                int conservados =
                    1 + a * 7 / 100;

                for (
                    int y = 0;
                    y < rellena.rows;
                    y += 8
                    ) {
                    for (
                        int x = 0;
                        x < rellena.cols;
                        x += 8
                        ) {
                        cv::Mat bloque =
                            rellena(
                                cv::Rect(x, y, 8, 8)
                            );

                        cv::Mat coeficientes;
                        cv::Mat reconstruido;

                        cv::dct(
                            bloque, coeficientes
                        );

                        for (int v = 0; v < 8; ++v) {
                            for (int u = 0; u < 8; ++u) {

                                if (
                                    u >= conservados ||
                                    v >= conservados
                                    ) {
                                    coeficientes.at<float>(
                                        v, u
                                    ) = 0;
                                }
                            }
                        }

                        cv::idct(
                            coeficientes,
                            reconstruido
                        );

                        reconstruido.copyTo(bloque);
                    }
                }

                cv::Mat resultado;

                rellena(
                    cv::Rect(
                        0, 0, g.cols, g.rows
                    )
                ).convertTo(
                    resultado, CV_8U
                );

                return resultado;
            }

            /*
             * Ajustar las dimensiones para una DFT eficiente.
             * getOptimalDFTSize no exige potencias de dos.
             */
            cv::Mat rellena;

            cv::copyMakeBorder(
                flotante,
                rellena,
                0,
                cv::getOptimalDFTSize(
                    flotante.rows
                ) - flotante.rows,
                0,
                cv::getOptimalDFTSize(
                    flotante.cols
                ) - flotante.cols,
                cv::BORDER_CONSTANT,
                0
            );

            cv::Mat espectro;

            cv::dft(
                rellena,
                espectro,
                cv::DFT_COMPLEX_OUTPUT
            );

            if (op == 0) {

                cv::Mat canales[2];
                cv::Mat magnitud;

                cv::split(
                    espectro, canales
                );

                cv::magnitude(
                    canales[0],
                    canales[1],
                    magnitud
                );

                // Compresión logarítmica para visualizar.
                magnitud += 1;
                cv::log(magnitud, magnitud);

                cv::Mat centrado(
                    magnitud.size(), CV_32F
                );

                // fftshift válido también para dimensiones impares.
                for (
                    int y = 0;
                    y < magnitud.rows;
                    ++y
                    ) {
                    for (
                        int x = 0;
                        x < magnitud.cols;
                        ++x
                        ) {
                        centrado.at<float>(y, x) =
                            magnitud.at<float>(
                                (
                                    y +
                                    (magnitud.rows + 1) / 2
                                    ) % magnitud.rows,
                                (
                                    x +
                                    (magnitud.cols + 1) / 2
                                    ) % magnitud.cols
                            );
                    }
                }

                return visible(centrado);
            }

            double exterior =
                std::max(0.01, a / 200.0);

            double interior =
                std::min(b / 510.0, exterior);

            /*
             * Filtrar el espectro SIN desplazarlo.
             * En esta representación la frecuencia cero está
             * en (0,0), y las frecuencias negativas se envuelven.
             */
            for (
                int y = 0;
                y < espectro.rows;
                ++y
                ) {
                for (
                    int x = 0;
                    x < espectro.cols;
                    ++x
                    ) {
                    double u =
                        static_cast<double>(
                            std::min(
                                x, espectro.cols - x
                            )
                            ) / espectro.cols;

                    double v =
                        static_cast<double>(
                            std::min(
                                y, espectro.rows - y
                            )
                            ) / espectro.rows;

                    double distancia =
                        std::sqrt(u * u + v * v);

                    bool conservar;

                    if (op == 1) {

                        conservar =
                            distancia <= exterior;
                    }
                    else if (op == 2) {

                        conservar =
                            distancia >= exterior;
                    }
                    else {

                        conservar =
                            distancia >= interior &&
                            distancia <= exterior;
                    }

                    if (!conservar) {

                        espectro.at<cv::Vec2f>(y, x) =
                            cv::Vec2f(0, 0);
                    }
                }
            }

            cv::Mat reconstruida;
            cv::Mat resultado;

            cv::idft(
                espectro,
                reconstruida,
                cv::DFT_REAL_OUTPUT |
                cv::DFT_SCALE
            );

            reconstruida =
                reconstruida(
                    cv::Rect(
                        0, 0, g.cols, g.rows
                    )
                );

            /*
             * El paso alto puede producir negativos.
             * Mostramos su valor absoluto.
             */
            if (op == 2 || op == 3) {

                cv::convertScaleAbs(
                    reconstruida,
                    resultado
                );
            }
            else {

                reconstruida.convertTo(
                    resultado,
                    CV_8U
                );
            }

            return resultado;
        }
    );
}