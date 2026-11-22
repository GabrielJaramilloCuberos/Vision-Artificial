#include "Temas.h"
#include "Laboratorio.h"

#include <algorithm>

namespace {

    cv::Vec3b color(int numero) {

        // Colores deterministas para distinguir etiquetas.
        return cv::Vec3b(
            static_cast<uchar>(
                40 + numero * 53 % 200
                ),
            static_cast<uchar>(
                40 + numero * 97 % 200
                ),
            static_cast<uchar>(
                40 + numero * 137 % 200
                )
        );
    }

}

void segmentacion() {

    laboratorio(
        "10_Segmentacion",
        {
            "Mascara HSV",
            "Componentes",
            "Watershed",
            "K-means",
            "GrabCut central",
            "Clases por color",
            "Instancias por color",
            "Panoptica didactica"
        },
        "HSV: A=tono central (0..179), B=semiancho (1..60). "
        "Componentes: Otsu.\n"
        "Clases/instancias/panoptica: reglas rojo-verde-azul-fondo, "
        "SIN reconocimiento aprendido.\n"
        "Watershed separa objetos claros; GrabCut presupone "
        "un objeto dentro del rectangulo central.",

        [](const cv::Mat& m, int op, int a, int b) {

            cv::Mat g = gris(m);
            cv::Mat mascara;
            cv::Mat hsv;
            cv::Mat resultado = m.clone();

            cv::cvtColor(
                m, hsv, cv::COLOR_BGR2HSV
            );

            if (op == 0) {

                int centro = a * 179 / 100;
                int ancho = 1 + b * 59 / 255;

                mascara =
                    cv::Mat::zeros(
                        m.size(), CV_8U
                    );

                /*
                 * El tono H es circular.
                 * Así el rojo próximo a 0 y el próximo a 179
                 * pueden pertenecer a la misma selección.
                 */
                for (int y = 0; y < m.rows; ++y) {
                    for (int x = 0; x < m.cols; ++x) {

                        auto pixel =
                            hsv.at<cv::Vec3b>(y, x);

                        int distancia =
                            std::abs(pixel[0] - centro);

                        if (
                            std::min(
                                distancia,
                                180 - distancia
                            ) <= ancho &&
                            pixel[1] > 80 &&
                            pixel[2] > 40
                            ) {
                            mascara.at<uchar>(y, x) =
                                255;
                        }
                    }
                }

                resultado =
                    cv::Mat::zeros(
                        m.size(), m.type()
                    );

                m.copyTo(
                    resultado, mascara
                );

                return resultado;
            }

            if (op == 3) {

                /*
                 * Agrupar píxeles por similitud de color.
                 * Reducimos esta demo para controlar el costo.
                 */
                cv::Mat pequena;

                cv::resize(
                    m, pequena, { 320, 240 }
                );

                cv::Mat datos =
                    pequena.reshape(
                        1,
                        pequena.rows * pequena.cols
                    );

                cv::Mat flotantes;
                cv::Mat etiquetas;
                cv::Mat centros;

                datos.convertTo(
                    flotantes, CV_32F
                );

                int grupos = 2 + a * 4 / 100;

                cv::kmeans(
                    flotantes,
                    grupos,
                    etiquetas,
                    cv::TermCriteria(
                        cv::TermCriteria::COUNT |
                        cv::TermCriteria::EPS,
                        20,
                        0.5
                    ),
                    1,
                    cv::KMEANS_PP_CENTERS,
                    centros
                );

                for (
                    int i = 0;
                    i < flotantes.rows;
                    ++i
                    ) {
                    int grupo =
                        etiquetas.at<int>(i);

                    for (int canal = 0; canal < 3; ++canal) {

                        pequena.at<cv::Vec3b>(
                            i / pequena.cols,
                            i % pequena.cols
                        )[canal] =
                            cv::saturate_cast<uchar>(
                                centros.at<float>(
                                    grupo, canal
                                )
                            );
                    }
                }

                cv::resize(
                    pequena,
                    resultado,
                    m.size(),
                    0, 0,
                    cv::INTER_NEAREST
                );

                return resultado;
            }

            if (op == 4) {

                /*
                 * GrabCut usa un rectángulo inicial.
                 * El exterior se considera fondo seguro.
                 */
                cv::Mat modeloFondo;
                cv::Mat modeloObjeto;

                int margenX =
                    std::max(1, m.cols / 10);

                int margenY =
                    std::max(1, m.rows / 10);

                cv::grabCut(
                    m,
                    mascara,
                    {
                        margenX,
                        margenY,
                        m.cols - 2 * margenX,
                        m.rows - 2 * margenY
                    },
                    modeloFondo,
                    modeloObjeto,
                    3,
                    cv::GC_INIT_WITH_RECT
                );

                mascara =
                    (mascara == cv::GC_FGD) |
                    (mascara == cv::GC_PR_FGD);

                resultado =
                    cv::Mat::zeros(
                        m.size(), m.type()
                    );

                m.copyTo(
                    resultado, mascara
                );

                return resultado;
            }

            if (op >= 5) {

                /*
                 * Clases definidas por reglas:
                 *
                 * 0 = fondo o color no seleccionado.
                 * 1 = rojo.
                 * 2 = verde.
                 * 3 = azul.
                 *
                 * Es una representación didáctica de clases.
                 */
                cv::Mat clases =
                    cv::Mat::zeros(
                        m.size(), CV_8U
                    );

                for (int y = 0; y < m.rows; ++y) {
                    for (int x = 0; x < m.cols; ++x) {

                        auto pixel =
                            hsv.at<cv::Vec3b>(y, x);

                        if (
                            pixel[1] > 80 &&
                            pixel[2] > 40
                            ) {
                            uchar clase = 0;

                            if (
                                pixel[0] < 15 ||
                                pixel[0] > 165
                                ) {
                                clase = 1;
                            }
                            else if (
                                pixel[0] >= 35 &&
                                pixel[0] < 90
                                ) {
                                clase = 2;
                            }
                            else if (
                                pixel[0] >= 90 &&
                                pixel[0] < 140
                                ) {
                                clase = 3;
                            }

                            clases.at<uchar>(y, x) =
                                clase;
                        }
                    }
                }

                resultado =
                    cv::Mat::zeros(
                        m.size(), CV_8UC3
                    );

                int acumulado = 0;

                for (int clase = 1; clase <= 3; ++clase) {

                    cv::Mat etiquetas;

                    int cantidad =
                        cv::connectedComponents(
                            clases == clase,
                            etiquetas
                        );

                    for (int y = 0; y < m.rows; ++y) {
                        for (int x = 0; x < m.cols; ++x) {

                            int etiqueta =
                                etiquetas.at<int>(y, x);

                            if (etiqueta == 0) {
                                continue;
                            }

                            /*
                             * Modo clases: todos los objetos de
                             * una clase tienen el mismo color.
                             *
                             * Modo instancias: cada componente
                             * recibe un identificador diferente.
                             */
                            resultado.at<cv::Vec3b>(y, x) =
                                op == 5
                                ? color(clase)
                                : color(acumulado + etiqueta);
                        }
                    }

                    acumulado += cantidad - 1;
                }

                if (op == 7) {

                    // En la vista panóptica también se etiqueta el fondo.
                    resultado.setTo(
                        cv::Scalar(65, 65, 65),
                        clases == 0
                    );

                    texto(
                        resultado,
                        "Fondo + clase de color + ID de componente",
                        1
                    );
                }

                texto(
                    resultado,
                    op == 5
                    ? "Clases: rojo, verde, azul"
                    : "Instancias: " +
                    std::to_string(acumulado),
                    2
                );

                return resultado;
            }

            // Máscara inicial para componentes y watershed.
            cv::threshold(
                g,
                mascara,
                0, 255,
                cv::THRESH_BINARY |
                cv::THRESH_OTSU
            );

            cv::Mat etiquetas;
            cv::Mat estadisticas;
            cv::Mat centroides;

            int cantidad =
                cv::connectedComponentsWithStats(
                    mascara,
                    etiquetas,
                    estadisticas,
                    centroides
                );

            if (op == 2) {

                /*
                 * Watershed necesita marcadores.
                 *
                 * El distance transform permite encontrar
                 * zonas interiores seguras de los objetos.
                 */
                cv::Mat distancia;
                cv::Mat seguro;
                cv::Mat fondo;

                cv::distanceTransform(
                    mascara,
                    distancia,
                    cv::DIST_L2,
                    3
                );

                double maximo;

                cv::minMaxLoc(
                    distancia,
                    nullptr,
                    &maximo
                );

                cv::threshold(
                    distancia,
                    seguro,
                    maximo * (0.1 + a * 0.008),
                    255,
                    cv::THRESH_BINARY
                );

                seguro.convertTo(
                    seguro, CV_8U
                );

                cv::connectedComponents(
                    seguro, etiquetas
                );

                etiquetas += 1;

                cv::dilate(
                    mascara,
                    fondo,
                    cv::Mat(),
                    { -1, -1 },
                    3
                );

                cv::Mat desconocido;

                cv::subtract(
                    fondo,
                    seguro,
                    desconocido
                );

                etiquetas.setTo(
                    0, desconocido
                );

                cv::watershed(
                    m, etiquetas
                );
            }

            resultado =
                cv::Mat::zeros(
                    m.size(), CV_8UC3
                );

            for (int y = 0; y < m.rows; ++y) {
                for (int x = 0; x < m.cols; ++x) {

                    int etiqueta =
                        etiquetas.at<int>(y, x);

                    if (etiqueta == -1) {

                        // Watershed marca los límites con -1.
                        resultado.at<cv::Vec3b>(y, x) =
                        { 255, 255, 255 };
                    }
                    else if (
                        etiqueta > (op == 2 ? 1 : 0)
                        ) {
                        resultado.at<cv::Vec3b>(y, x) =
                            color(etiqueta);
                    }
                }
            }

            if (op == 1) {

                texto(
                    resultado,
                    "Componentes: " +
                    std::to_string(cantidad - 1),
                    1
                );
            }

            return resultado;
        }
    );
}