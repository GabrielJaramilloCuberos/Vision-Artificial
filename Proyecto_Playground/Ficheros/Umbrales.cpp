#include "Temas.h"
#include "Laboratorio.h"

void umbrales() {

    laboratorio(
        "08_Umbrales",
        {
            "Binaria",
            "Inversa",
            "Truncamiento",
            "A cero",
            "Otsu",
            "Adaptativa media",
            "Adaptativa gaussiana"
        },
        "B=umbral global. Adaptativa: "
        "bloque=3+2*(A/4), constante C=(B-128)/10.",

        [](const cv::Mat& m, int op, int a, int b) {

            cv::Mat g = gris(m);
            cv::Mat resultado;

            if (op < 4) {

                const int tipos[] = {
                    cv::THRESH_BINARY,
                    cv::THRESH_BINARY_INV,
                    cv::THRESH_TRUNC,
                    cv::THRESH_TOZERO
                };

                cv::threshold(
                    g,
                    resultado,
                    b,
                    255,
                    tipos[op]
                );
            }
            else if (op == 4) {

                /*
                 * Otsu calcula el umbral automáticamente
                 * a partir de la distribución de intensidades.
                 */
                cv::threshold(
                    g,
                    resultado,
                    0,
                    255,
                    cv::THRESH_BINARY |
                    cv::THRESH_OTSU
                );
            }
            else {

                // El tamaño de bloque debe ser impar y mayor que 1.
                int bloque =
                    3 + 2 * (a / 4);

                int metodo =
                    op == 5
                    ? cv::ADAPTIVE_THRESH_MEAN_C
                    : cv::ADAPTIVE_THRESH_GAUSSIAN_C;

                cv::adaptiveThreshold(
                    g,
                    resultado,
                    255,
                    metodo,
                    cv::THRESH_BINARY,
                    bloque,
                    (b - 128) / 10.0
                );
            }

            return resultado;
        }
    );
}