#include "Temas.h"
#include "Laboratorio.h"

#include <algorithm>
#include <cmath>

void redes() {

    laboratorio(
        "12_Redes",
        {
            "Convolucion y ReLU",
            "Max pooling 2x2",
            "Recurrencia RNN",
            "Parches ViT",
            "Atencion didactica"
        },
        "Operaciones didacticas con pesos fijos, "
        "SIN entrenamiento ni inferencia de modelos.\n"
        "RNN: A=memoria; B>=128 invierte el orden de lectura. "
        "Atencion: A=parche consulta; B>=128 agrega posicion.\n"
        "No implementa CNN/ViT completos, CLS entrenado, "
        "Mask R-CNN ni Mask2Former.",

        [](const cv::Mat& m, int op, int a, int b) {

            cv::Mat g = gris(m);
            cv::Mat resultado = m.clone();

            if (op <= 1) {

                cv::Mat flotante;
                cv::Mat activacion;

                g.convertTo(
                    flotante,
                    CV_32F,
                    1.0 / 255
                );

                cv::Mat kernel = cv::Mat_<float>({ 3, 3 }, {
    -1, 0, 1,
    -1, 0, 1,
    -1, 0, 1
                    });

                cv::filter2D(
                    flotante,
                    activacion,
                    CV_32F,
                    kernel
                );

                // ReLU: reemplazar valores negativos por cero.
                cv::max(
                    activacion,
                    0,
                    activacion
                );

                if (op == 0) {
                    return visible(activacion);
                }

                /*
                 * Max pooling real:
                 * cada bloque 2x2 se reemplaza por su máximo.
                 */
                cv::Mat agrupada(
                    activacion.rows / 2,
                    activacion.cols / 2,
                    CV_32F
                );

                for (
                    int y = 0;
                    y < agrupada.rows;
                    ++y
                    ) {
                    for (
                        int x = 0;
                        x < agrupada.cols;
                        ++x
                        ) {
                        double maximo;

                        cv::minMaxLoc(
                            activacion(
                                cv::Rect(
                                    2 * x,
                                    2 * y,
                                    2,
                                    2
                                )
                            ),
                            nullptr,
                            &maximo
                        );

                        agrupada.at<float>(y, x) =
                            static_cast<float>(
                                maximo
                                );
                    }
                }

                return visible(agrupada);
            }

            /*
             * Dividir la imagen en 64 parches.
             *
             * El embedding didáctico de cada parche contiene:
             * B, G, R promedio y dos coordenadas de posición.
             */
            cv::Mat pequena;

            cv::resize(
                m, pequena, { 256, 256 }
            );

            cv::Mat tokens(
                64, 5, CV_32F
            );

            for (int i = 0; i < 64; ++i) {

                cv::Scalar media =
                    cv::mean(
                        pequena(
                            cv::Rect(
                                i % 8 * 32,
                                i / 8 * 32,
                                32,
                                32
                            )
                        )
                    );

                for (
                    int canal = 0;
                    canal < 3;
                    ++canal
                    ) {
                    tokens.at<float>(i, canal) =
                        static_cast<float>(
                            media[canal] / 255
                            );
                }

                tokens.at<float>(i, 3) =
                    b >= 128
                    ? (i % 8) / 7.0f
                    : 0;

                tokens.at<float>(i, 4) =
                    b >= 128
                    ? (i / 8) / 7.0f
                    : 0;
            }

            cv::Mat mapa(
                8, 8, CV_32F
            );

            if (op == 2) {

                /*
                 * Recurrencia escalar con pesos fijos:
                 *
                 * h_actual = tanh(w*h_anterior + entrada).
                 *
                 * Permite observar dependencia del orden y memoria.
                 */
                float estado = 0;

                for (int paso = 0; paso < 64; ++paso) {

                    int i =
                        b >= 128
                        ? 63 - paso
                        : paso;

                    float entrada =
                        (
                            tokens.at<float>(i, 0) +
                            tokens.at<float>(i, 1) +
                            tokens.at<float>(i, 2)
                            ) / 3;

                    estado = std::tanh(
                        a / 100.0f * estado +
                        2 * entrada - 1
                    );

                    mapa.at<float>(
                        i / 8, i % 8
                    ) = estado;
                }
            }
            else if (op == 3) {

                for (int i = 0; i < 64; ++i) {

                    cv::rectangle(
                        resultado,
                        cv::Rect(
                            i % 8 * m.cols / 8,
                            i / 8 * m.rows / 8,
                            m.cols / 8,
                            m.rows / 8
                        ),
                        { 0, 255, 255 },
                        1
                    );
                }

                texto(
                    resultado,
                    "64 parches; embedding didactico de color y posicion",
                    1
                );

                return resultado;
            }
            else {

                /*
                 * Atención para un parche de consulta.
                 *
                 * Q y K se toman directamente de los tokens.
                 * No son proyecciones aprendidas.
                 */
                int consulta =
                    a * 63 / 100;

                float suma = 0;

                for (int i = 0; i < 64; ++i) {

                    float similitud =
                        static_cast<float>(
                            tokens.row(consulta).dot(
                                tokens.row(i)
                            ) /
                            std::sqrt(5.0)
                            );

                    float peso =
                        std::exp(similitud);

                    mapa.at<float>(
                        i / 8, i % 8
                    ) = peso;

                    suma += peso;
                }

                /*
                 * Softmax:
                 * pesos positivos cuya suma es 1.
                 */
                mapa /= suma;
            }

            cv::Mat vista = visible(mapa);

            cv::resize(
                vista,
                resultado,
                m.size(),
                0, 0,
                cv::INTER_NEAREST
            );

            texto(
                resultado,
                op == 2
                ? "Estado recurrente h=tanh(w*h+x)"
                : "Pesos de atencion (reescalados para mostrar)",
                1
            );

            return resultado;
        }
    );
}