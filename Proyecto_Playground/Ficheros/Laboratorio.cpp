#include "Laboratorio.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>

cv::Mat demo() {

    /*
     * Escena determinista con:
     * - Gradiente de iluminación.
     * - Figuras rojas, verdes y azules.
     * - Texto y una región clara.
     *
     * Sirve para probar canales, bordes, segmentación y textura.
     */
    cv::Mat m(480, 640, CV_8UC3);

    for (int y = 0; y < m.rows; ++y) {
        for (int x = 0; x < m.cols; ++x) {

            uchar v = static_cast<uchar>(
                25 + 80 * x / m.cols
                );

            m.at<cv::Vec3b>(y, x) =
                cv::Vec3b(v, v, v);
        }
    }

    cv::rectangle(
        m, { 55, 80, 130, 130 },
        { 0, 0, 240 }, cv::FILLED
    );

    cv::circle(
        m, { 335, 150 }, 65,
        { 0, 240, 0 }, cv::FILLED
    );

    cv::circle(
        m, { 515, 150 }, 55,
        { 240, 0, 0 }, cv::FILLED
    );

    cv::rectangle(
        m, { 100, 290, 420, 90 },
        { 230, 230, 230 }, cv::FILLED
    );

    cv::putText(
        m, "OpenCV 5", { 130, 350 },
        cv::FONT_HERSHEY_SIMPLEX,
        1.4, { 10, 10, 10 }, 3
    );

    return m;
}

cv::Mat gris(const cv::Mat& m) {

    cv::Mat g;

    if (m.channels() == 1) {
        g = m.clone();
    }
    else {
        cv::cvtColor(
            m, g, cv::COLOR_BGR2GRAY
        );
    }

    return g;
}

cv::Mat visible(const cv::Mat& m) {

    cv::Mat u;
    cv::Mat b;

    /*
     * Algunos algoritmos producen valores float o negativos.
     * Para mostrarlos, los reescalamos a 0..255.
     *
     * Esta conversión es para visualización, no para medición.
     */
    if (m.depth() != CV_8U) {

        cv::normalize(
            m, u, 0, 255,
            cv::NORM_MINMAX, CV_8U
        );
    }
    else {
        u = m;
    }

    if (u.channels() == 1) {

        cv::cvtColor(
            u, b, cv::COLOR_GRAY2BGR
        );
    }
    else {
        b = u.clone();
    }

    return b;
}

void texto(
    cv::Mat& m,
    const std::string& mensaje,
    int linea
) {
    cv::Point posicion(
        10,
        25 + 25 * linea
    );

    // Contorno negro para mejorar la lectura.
    cv::putText(
        m, mensaje, posicion,
        cv::FONT_HERSHEY_SIMPLEX,
        0.55, { 0, 0, 0 }, 3, cv::LINE_AA
    );

    cv::putText(
        m, mensaje, posicion,
        cv::FONT_HERSHEY_SIMPLEX,
        0.55, { 255, 255, 255 }, 1, cv::LINE_AA
    );
}

namespace {

    cv::Mat panel(const cv::Mat& m) {

        cv::Mat b = visible(m);
        cv::Mat resized;

        /*
         * Ajustar la vista a un panel conservando su proporción.
         * Esto solo cambia la presentación; el algoritmo utiliza
         * la imagen con su resolución original.
         */
        double escala = std::min(
            480.0 / b.cols,
            360.0 / b.rows
        );

        cv::resize(
            b, resized, {},
            escala, escala
        );

        cv::Mat p(
            360, 480, CV_8UC3,
            cv::Scalar(25, 25, 25)
        );

        cv::Rect destino(
            (480 - resized.cols) / 2,
            (360 - resized.rows) / 2,
            resized.cols,
            resized.rows
        );

        resized.copyTo(p(destino));

        return p;
    }

}

void laboratorio(
    const std::string& nombre,
    const std::vector<std::string>& operaciones,
    const std::string& ayuda,
    Procesador procesar
) {
    std::cout
        << "\n" << nombre
        << "\n" << ayuda
        << "\nFuente: 0=demo, 1=imagen, 2=camara: ";

    std::string fuente;
    std::getline(std::cin, fuente);

    cv::Mat entrada;
    cv::VideoCapture cap;

    bool vivo = fuente == "2";
    bool pausa = false;

    if (vivo) {

        cap.open(0);

        if (!cap.isOpened()) {
            std::cout
                << "No se pudo abrir la camara.\n";
            return;
        }
    }
    else if (fuente == "1") {

        std::cout
            << "Ruta de la imagen, sin comillas: ";

        std::string ruta;
        std::getline(std::cin, ruta);

        entrada = cv::imread(ruta);

        if (entrada.empty()) {
            std::cout
                << "No se pudo leer la imagen.\n";
            return;
        }
    }
    else {
        entrada = demo();
    }

    cv::namedWindow(
        nombre,
        cv::WINDOW_AUTOSIZE
    );

    /*
     * Cada trackbar almacena su posición internamente.
     * Leemos su valor con getTrackbarPos().
     */
    cv::createTrackbar(
        "Operacion", nombre, nullptr,
        static_cast<int>(operaciones.size()) - 1
    );

    cv::createTrackbar(
        "A", nombre, nullptr, 100
    );

    cv::createTrackbar(
        "B", nombre, nullptr, 255
    );

    cv::setTrackbarPos("A", nombre, 50);
    cv::setTrackbarPos("B", nombre, 128);

    while (true) {

        if (
            vivo &&
            (!pausa || entrada.empty())
            ) {
            if (
                !cap.read(entrada) ||
                entrada.empty()
                ) {
                break;
            }
        }

        if (
            entrada.cols < 16 ||
            entrada.rows < 16
            ) {
            throw std::runtime_error(
                "Usa una imagen de al menos 16x16 pixeles."
            );
        }

        int operacion = cv::getTrackbarPos(
            "Operacion", nombre
        );

        int a = cv::getTrackbarPos(
            "A", nombre
        );

        int b = cv::getTrackbarPos(
            "B", nombre
        );

        int64 inicio = cv::getTickCount();

        /*
         * Aquí se llama al algoritmo del módulo actual.
         * Laboratorio no necesita conocer sus detalles.
         */
        cv::Mat salida = procesar(
            entrada, operacion, a, b
        );

        if (salida.empty()) {
            throw std::runtime_error(
                "El modulo devolvio una imagen vacia."
            );
        }

        double ms =
            1000.0 *
            (cv::getTickCount() - inicio) /
            cv::getTickFrequency();

        cv::Mat izquierda = panel(entrada);
        cv::Mat derecha = panel(salida);
        cv::Mat vista;

        texto(izquierda, "Original", 0);

        texto(
            derecha,
            operaciones.at(operacion),
            0
        );

        cv::hconcat(
            izquierda, derecha, vista
        );

        cv::copyMakeBorder(
            vista, vista,
            0, 55, 0, 0,
            cv::BORDER_CONSTANT,
            { 25, 25, 25 }
        );

        cv::putText(
            vista,
            "ESC: menu | S: guardar | P: pausar camara | ms: "
            + std::to_string(ms),
            { 10, 390 },
            cv::FONT_HERSHEY_SIMPLEX,
            0.55,
            { 255, 255, 255 },
            1
        );

        cv::imshow(nombre, vista);

        int tecla = cv::waitKey(20) & 255;

        if (
            tecla == 27 ||
            cv::getWindowProperty(
                nombre,
                cv::WND_PROP_VISIBLE
            ) < 1
            ) {
            break;
        }

        if (
            tecla == 'p' ||
            tecla == 'P'
            ) {
            pausa = !pausa;
        }

        if (
            tecla == 's' ||
            tecla == 'S'
            ) {
            std::filesystem::create_directories(
                "resultados"
            );

            /*
             * Identificador temporal para no sobrescribir
             * las evidencias guardadas anteriormente.
             */
            auto id =
                std::chrono::system_clock::now()
                .time_since_epoch().count();

            std::string base =
                "resultados/" + nombre + "_" +
                std::to_string(id);

            cv::imwrite(
                base + ".png",
                visible(salida)
            );

            cv::imwrite(
                base + "_original.png",
                entrada
            );

            // Guardar los parámetros de la evidencia.
            cv::FileStorage archivo(
                base + ".yaml",
                cv::FileStorage::WRITE
            );

            archivo
                << "modulo" << nombre
                << "operacion"
                << operaciones.at(operacion)
                << "A" << a
                << "B" << b;

            std::cout
                << "Guardado: "
                << base
                << "\n";
        }
    }

    cap.release();
    cv::destroyAllWindows();
}