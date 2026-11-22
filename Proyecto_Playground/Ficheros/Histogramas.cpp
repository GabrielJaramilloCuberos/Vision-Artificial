#include "Temas.h"
#include "Laboratorio.h"

namespace {

    cv::Mat histograma(const cv::Mat& imagen) {

        cv::Mat lienzo(
            360, 512, CV_8UC3,
            cv::Scalar(20, 20, 20)
        );

        int bins = 256;

        float rango[] = { 0, 256 };
        const float* rangos[] = { rango };

        for (
            int canal = 0;
            canal < imagen.channels();
            ++canal
            ) {
            cv::Mat hist;

            cv::calcHist(
                &imagen,
                1,
                &canal,
                cv::Mat(),
                hist,
                1,
                &bins,
                rangos
            );

            /*
             * Reescalar solo la altura del gráfico.
             * No altera la imagen ni su distribución.
             */
            cv::normalize(
                hist, hist,
                0, 300,
                cv::NORM_MINMAX
            );

            cv::Scalar color;

            if (imagen.channels() == 1) {
                color = cv::Scalar(255, 255, 255);
            }
            else if (canal == 0) {
                color = cv::Scalar(255, 0, 0);
            }
            else if (canal == 1) {
                color = cv::Scalar(0, 255, 0);
            }
            else {
                color = cv::Scalar(0, 0, 255);
            }

            for (int i = 1; i < 256; ++i) {

                cv::line(
                    lienzo,
                    {
                        2 * (i - 1),
                        330 - cvRound(
                            hist.at<float>(i - 1)
                        )
                    },
                {
                    2 * i,
                    330 - cvRound(
                        hist.at<float>(i)
                    )
                },
                    color,
                    1
                );
            }
        }

        texto(
            lienzo,
            "X: intensidad 0..255 | Y: frecuencia reescalada",
            0
        );

        return lienzo;
    }

}

void histogramas() {

    laboratorio(
        "07_Histogramas",
        {
            "Histograma gris",
            "Histogramas BGR",
            "Ecualizacion",
            "CLAHE",
            "Histograma ecualizado"
        },
        "A=limite CLAHE (0.1..10.1). B sin uso.",

        [](const cv::Mat& m, int op, int a, int) {

            cv::Mat g = gris(m);
            cv::Mat resultado;

            if (op == 0) {
                return histograma(g);
            }

            if (op == 1) {
                return histograma(m);
            }

            if (op == 2 || op == 4) {

                cv::equalizeHist(
                    g, resultado
                );
            }

            if (op == 3) {

                /*
                 * CLAHE realiza ecualización local
                 * y limita la amplificación del contraste.
                 */
                cv::createCLAHE(
                    0.1 + a / 10.0,
                    { 8, 8 }
                )->apply(g, resultado);
            }

            if (op == 4) {
                return histograma(resultado);
            }

            return resultado;
        }
    );
}