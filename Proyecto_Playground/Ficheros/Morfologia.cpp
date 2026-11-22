#include "Temas.h"
#include "Laboratorio.h"

void morfologia() {

    laboratorio(
        "09_Morfologia",
        {
            "Erosion",
            "Dilatacion",
            "Apertura",
            "Cierre",
            "Gradiente",
            "Top hat",
            "Black hat",
            "Reconstruccion"
        },
        "A=radio (0..10). "
        "B<128: gris; B>=128: binaria Otsu.\n"
        "EE eliptico plano. Erosion=minimo, dilatacion=maximo. "
        "No implementa EE no planos.",

        [](const cv::Mat& m, int op, int a, int b) {

            cv::Mat g = gris(m);
            cv::Mat resultado;

            int k =
                1 + 2 * (a / 10);

            if (b >= 128) {

                cv::threshold(
                    g, g,
                    0, 255,
                    cv::THRESH_BINARY |
                    cv::THRESH_OTSU
                );
            }

            cv::Mat elemento =
                cv::getStructuringElement(
                    cv::MORPH_ELLIPSE,
                    { k, k }
                );

            if (op == 0) {

                cv::erode(
                    g, resultado, elemento
                );
            }
            else if (op == 1) {

                cv::dilate(
                    g, resultado, elemento
                );
            }
            else if (op < 7) {

                const int tipos[] = {
                    cv::MORPH_OPEN,
                    cv::MORPH_CLOSE,
                    cv::MORPH_GRADIENT,
                    cv::MORPH_TOPHAT,
                    cv::MORPH_BLACKHAT
                };

                cv::morphologyEx(
                    g,
                    resultado,
                    tipos[op - 2],
                    elemento
                );
            }
            else {

                /*
                 * Reconstrucción por dilatación:
                 *
                 * 1. Erosionar para obtener un marcador.
                 * 2. Dilatar repetidamente.
                 * 3. Limitar cada expansión mediante la imagen original.
                 * 4. Detenerse cuando ya no cambia.
                 */
                cv::erode(
                    g, resultado, elemento
                );

                cv::Mat previo;

                cv::Mat paso =
                    cv::getStructuringElement(
                        cv::MORPH_CROSS,
                        { 3, 3 }
                    );

                do {

                    previo = resultado.clone();

                    cv::dilate(
                        resultado,
                        resultado,
                        paso
                    );

                    cv::min(
                        resultado,
                        g,
                        resultado
                    );

                } while (
                    cv::norm(
                        resultado,
                        previo,
                        cv::NORM_INF
                    ) > 0
                    );
            }

            return resultado;
        }
    );
}