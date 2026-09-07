#include "Utils/CalibrationConfig.hpp"
#include "Utils/CameraCalibration.hpp"

#include <iostream>

int main() {

    /*
     * Crear la configuración utilizada por todos los
     * módulos del programa.
     */
    CalibrationConfig config;

    while (true) {
        std::cout
            << "\n=============================\n";

        std::cout
            << "   CALIBRACION DE CAMARA\n";

        std::cout
            << "=============================\n";

        std::cout
            << "1. Capturar imagenes del tablero\n";

        std::cout
            << "2. Calibrar y guardar parametros\n";

        std::cout
            << "3. Probar correccion en vivo\n";

        std::cout
            << "0. Salir\n";

        std::cout
            << "Seleccion: ";

        int option;

        if (!(std::cin >> option)) {
            /*
             * Recuperar cin si el usuario escribe texto
             * en lugar de un número.
             */
            std::cin.clear();
            std::cin.ignore(10000, '\n');

            std::cout
                << "Opcion invalida.\n";

            continue;
        }

        switch (option) {

            case 1:
                captureCalibrationImages(
                    config
                );
                break;

            case 2:
                calibrateCameraFromImages(
                    config
                );
                break;

            case 3:
                testCalibration(
                    config
                );
                break;

            case 0:
                return 0;

            default:
                std::cout
                    << "Opcion invalida.\n";
                break;
        }
    }
}