#include "Temas.h"
#include "Laboratorio.h"

void introduccion() {

    laboratorio(
        "01_Introduccion",
        {
            "Representar",
            "Procesar",
            "Analizar"
        },
        "0: imagen digital. 1: bordes. "
        "2: contar componentes brillantes. "
        "A y B sin uso.",

        [](const cv::Mat& m, int op, int, int) {

            cv::Mat g = gris(m);
            cv::Mat resultado = m.clone();

            if (op == 0) {

                texto(
                    resultado,
                    "Imagen = matriz de pixeles BGR",
                    1
                );

                texto(
                    resultado,
                    std::to_string(m.cols) +
                    " x " +
                    std::to_string(m.rows) +
                    " pixeles",
                    2
                );
            }
            else if (op == 1) {

                cv::Canny(
                    g, resultado, 70, 150
                );
            }
            else {

                cv::threshold(
                    g, g,
                    0, 255,
                    cv::THRESH_BINARY |
                    cv::THRESH_OTSU
                );

                cv::Mat etiquetas;
                cv::Mat estadisticas;
                cv::Mat centroides;

                int cantidad =
                    cv::connectedComponentsWithStats(
                        g,
                        etiquetas,
                        estadisticas,
                        centroides
                    );

                // La etiqueta 0 corresponde al fondo.
                texto(
                    resultado,
                    "Componentes (sin fondo): " +
                    std::to_string(cantidad - 1),
                    1
                );
            }

            return resultado;
        }
    );
}