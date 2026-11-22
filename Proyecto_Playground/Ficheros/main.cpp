#include "Temas.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <string>

int main() {

    const char* nombres[] = {
        "Introduccion",
        "Representacion e iluminacion",
        "Transformaciones",
        "Camara y calibracion",
        "Filtros espaciales",
        "Fourier y DCT",
        "Histogramas",
        "Umbralizacion y binarizacion",
        "Morfologia",
        "Segmentacion",
        "Caracterizacion",
        "CNN, RNN y ViT didacticos"
    };

    /*
     * Cada posición contiene la dirección de la función
     * que corresponde a una opción del menú.
     */
    void (*acciones[])() = {
        introduccion,
        representacion,
        transformaciones,
        camara,
        filtros,
        frecuencia,
        histogramas,
        umbrales,
        morfologia,
        segmentacion,
        caracterizacion,
        redes
    };

    while (true) {

        std::cout
            << "\nPLAYGROUND | OpenCV "
            << CV_VERSION
            << "\n";

        for (int i = 0; i < 12; ++i) {

            std::cout
                << i + 1
                << ". "
                << nombres[i]
                << "\n";
        }

        std::cout
            << "0. Salir\n"
            << "Seleccion: ";

        std::string opcion;

        if (
            !std::getline(std::cin, opcion) ||
            opcion == "0"
            ) {
            return 0;
        }

        try {

            size_t fin;

            int numero =
                std::stoi(opcion, &fin);

            if (
                fin != opcion.size() ||
                numero < 1 ||
                numero > 12
                ) {
                std::cout
                    << "Opcion invalida.\n";

                continue;
            }

            // Ejecutar el módulo elegido.
            acciones[numero - 1]();
        }
        catch (const std::exception& error) {

            cv::destroyAllWindows();

            std::cout
                << "No se pudo completar: "
                << error.what()
                << "\n";
        }
    }
}