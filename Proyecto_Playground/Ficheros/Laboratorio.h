#ifndef LABORATORIO_HPP
#define LABORATORIO_HPP

/*
 * Headers disponibles en OpenCV 5.
 *
 * geometry.hpp es necesario para varias operaciones geométricas
 * que antes estaban incluidas en otros módulos.
 */
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/geometry.hpp>

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

 /*
  * Todas las operaciones siguen la misma interfaz:
  *
  * entrada: imagen BGR de 8 bits.
  * operacion: posición seleccionada en el control.
  * A, B: parámetros interactivos.
  *
  * El resultado es una nueva imagen.
  */
using Procesador =
std::function<cv::Mat(const cv::Mat&, int, int, int)>;

// Genera una escena de prueba sin necesitar archivos externos.
cv::Mat demo();

// Convierte una imagen BGR a escala de grises.
cv::Mat gris(const cv::Mat& imagen);

// Prepara resultados float o de un canal para visualizarlos.
cv::Mat visible(const cv::Mat& imagen);

// Escribe información sobre una imagen.
void texto(
    cv::Mat& imagen,
    const std::string& mensaje,
    int linea
);

/*
 * Infraestructura compartida por los temas:
 *
 * - Selección de imagen, cámara o demo.
 * - Controles Operacion, A y B.
 * - Comparación original/resultado.
 * - Guardado de evidencias.
 */
void laboratorio(
    const std::string& nombre,
    const std::vector<std::string>& operaciones,
    const std::string& ayuda,
    Procesador procesar
);

#endif