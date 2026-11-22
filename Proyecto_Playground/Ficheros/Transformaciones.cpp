#include "Temas.h"
#include "Laboratorio.h"

void transformaciones() {

    laboratorio(
        "03_Transformaciones",
        {
            "Traslacion",
            "Rotacion",
            "Escala",
            "Cizalla",
            "Reflejo horizontal",
            "Reflejo vertical",
            "Perspectiva"
        },
        "A=desplazamiento/angulo/escala/cizalla/perspectiva. "
        "B=traslacion vertical.\n"
        "La salida conserva el lienzo: "
        "las partes fuera de el se recortan.",

        [](const cv::Mat& m, int op, int a, int b) {

            cv::Mat resultado;
            cv::Mat transformacion;

            cv::Point2f centro(
                m.cols / 2.0f,
                m.rows / 2.0f
            );

            if (op == 4 || op == 5) {

                cv::flip(
                    m,
                    resultado,
                    op == 4 ? 1 : 0
                );

                return resultado;
            }

            if (op == 6) {

                /*
                 * Una homografía utiliza cuatro pares
                 * de puntos y una matriz 3x3.
                 */
                float ancho =
                    static_cast<float>(m.cols - 1);

                float alto =
                    static_cast<float>(m.rows - 1);

                float desplazamiento =
                    ancho * a / 250.0f;

                cv::Point2f origen[] = {
                    {0, 0},
                    {ancho, 0},
                    {ancho, alto},
                    {0, alto}
                };

                cv::Point2f destino[] = {
                    {desplazamiento, 0},
                    {ancho - desplazamiento, 0},
                    {ancho, alto},
                    {0, alto}
                };

                transformacion =
                    cv::getPerspectiveTransform(
                        origen, destino
                    );

                cv::warpPerspective(
                    m,
                    resultado,
                    transformacion,
                    m.size()
                );
            }
            else {

                // Las transformaciones afines usan una matriz 2x3.
                if (op == 0) {

                    transformacion = cv::Mat_<double>({ 2, 3 }, {
    1.0, 0.0, (a - 50) * m.cols / 100.0,
    0.0, 1.0, (b - 128) * m.rows / 256.0
                        });
                }

                if (op == 1) {

                    transformacion =
                        cv::getRotationMatrix2D(
                            centro,
                            (a - 50) * 3.6,
                            1
                        );
                }

                if (op == 2) {

                    transformacion =
                        cv::getRotationMatrix2D(
                            centro,
                            0,
                            0.1 + a / 50.0
                        );
                }

                if (op == 3) {

                    transformacion = cv::Mat_<double>({ 2, 3 }, {
    1.0, (a - 50) / 100.0, 0.0,
    0.0, 1.0,             0.0
                        });
                }

                cv::warpAffine(
                    m,
                    resultado,
                    transformacion,
                    m.size()
                );
            }

            return resultado;
        }
    );
}