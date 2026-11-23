#include "Temas.h"
#include "Laboratorio.h"

void filtros() {

    laboratorio(
        "05_Filtros",
        {
            "Caja",
            "Gaussiano",
            "Mediana",
            "Bilateral",
            "Sobel",
            "Laplaciano",
            "Canny",
            "Prewitt",
            "Roberts",
            "Nitidez",
            "Minimo",
            "Maximo"
        },
        "A=tamano impar del kernel (3..15). "
        "En Canny A=umbral bajo y B=alto.\n"
        "filter2D realiza correlacion; los kernels permiten "
        "observar suavizado, derivadas y realce.",

        [](const cv::Mat& m, int op, int a, int b) {

            cv::Mat g = gris(m);
            cv::Mat resultado;
            cv::Mat x;
            cv::Mat y;

            // Siempre produce un tamaño impar entre 3 y 15.
            int k = 3 + 2 * (a / 16);

            if (op == 0) {
                cv::blur(m, resultado, { k, k });
            }

            if (op == 1) {
                cv::GaussianBlur(
                    m, resultado, { k, k }, 0
                );
            }

            if (op == 2) {
                cv::medianBlur(
                    m, resultado, k
                );
            }

            if (op == 3) {
                cv::bilateralFilter(
                    m, resultado, k, 75, 75
                );
            }

            if (op == 4) {

                // Gradientes horizontal y vertical.
                cv::Sobel(g, x, CV_32F, 1, 0);
                cv::Sobel(g, y, CV_32F, 0, 1);

                cv::magnitude(x, y, resultado);
                resultado = visible(resultado);
            }

            if (op == 5) {

                cv::Laplacian(
                    g, x, CV_16S, 3
                );

                cv::convertScaleAbs(
                    x, resultado
                );
            }

            if (op == 6) {

                cv::Canny(
                    g,
                    resultado,
                    a,
                    b > a ? b : a + 1
                );
            }

            if (op == 7 || op == 8) {

                cv::Mat kernel;

                if (op == 7) {

                    // Prewitt horizontal.
                    kernel = cv::Mat_<float>({ 3, 3 }, {
                        -1, 0, 1,
                        -1, 0, 1,
                        -1, 0, 1
                        });
                }
                else {

                    // Primera diagonal del operador Roberts.
                    kernel = cv::Mat_<float>({ 2, 2 }, {
                        1,  0,
                        0, -1
                        });
                }

                cv::filter2D(
                    g, x, CV_32F, kernel
                );

                cv::Mat otro;

                if (op == 7) {
                    otro = kernel.t();
                }
                else {
                    otro = cv::Mat_<float>({ 2, 2 }, {
                         0, 1,
                        -1, 0
                        });
                }

                cv::filter2D(
                    g, y, CV_32F, otro
                );

                cv::magnitude(x, y, resultado);
                resultado = visible(resultado);
            }

            if (op == 9) {

                // Kernel de realce de nitidez.
                cv::Mat kernel = cv::Mat_<float>({ 3, 3 }, {
     0, -1,  0,
    -1,  5, -1,
     0, -1,  0
                    });

                cv::filter2D(
                    m, resultado, -1, kernel
                );
            }

            if (op >= 10) {

                cv::Mat elemento =
                    cv::getStructuringElement(
                        cv::MORPH_RECT,
                        { k, k }
                    );

                if (op == 10) {

                    // Mínimo local.
                    cv::erode(
                        g, resultado, elemento
                    );
                }
                else {

                    // Máximo local.
                    cv::dilate(
                        g, resultado, elemento
                    );
                }
            }

            return resultado;
        }
    );
}
