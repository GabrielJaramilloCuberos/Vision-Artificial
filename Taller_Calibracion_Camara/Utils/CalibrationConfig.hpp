#ifndef CALIBRATION_CONFIG_HPP
#define CALIBRATION_CONFIG_HPP

#include <opencv2/core.hpp>

#include <cstddef>
#include <string>

/*
 * Esta estructura reúne los parámetros que pueden cambiar:
 *
 * - Tamaño del tablero.
 * - Medida de las casillas.
 * - Nombre de las carpetas.
 * - Cámara que se utilizará.
 *
 * Tenerlos en un solo archivo evita repetir valores en diferentes
 * partes del programa.
 */
struct CalibrationConfig {

    /*
     * El tablero físico tiene 8 x 8 casillas.
     *
     * OpenCV necesita el número de ESQUINAS INTERIORES:
     *
     * 8 x 8 casillas = 7 x 7 esquinas interiores.
     */
    cv::Size patternSize{7, 7};

    /*
     * Tamaño real del lado de cada casilla.
     *
     * En tu caso se estableció en 21 milímetros.
     * Verifica esta medida con una regla.
     */
    float squareSizeMm{21.0f};

    // Carpeta donde se guardarán las fotografías.
    std::string imagesFolder{"calibration_images"};

    // Archivo donde se guardará el resultado.
    std::string calibrationFile{
        "camera_calibration.yaml"
    };

    /*
     * Índice de la cámara:
     *
     * 0 = cámara predeterminada.
     * 1 = segunda cámara.
     * 2 = tercera cámara.
     */
    int cameraIndex{0};

    /*
     * Cantidad mínima aceptada por nuestro programa.
     *
     * Aunque se permiten 10, conviene capturar entre
     * 15 y 25 imágenes diferentes.
     */
    std::size_t minimumValidImages{10};
};

#endif