#ifndef VISION_UTILS_HPP
#define VISION_UTILS_HPP

#include "CalibrationConfig.hpp"

#include <opencv2/core.hpp>

#include <string>
#include <vector>

/*
 * Genera las coordenadas reales de las esquinas del tablero.
 *
 * Los puntos se expresan en milímetros y tienen Z = 0
 * porque el tablero es una superficie plana.
 */
std::vector<cv::Point3f> createObjectPoints(
    const CalibrationConfig& config
);

/*
 * Busca las esquinas interiores del tablero en una imagen.
 *
 * Si encuentra el patrón completo:
 * - Refina las esquinas con cornerSubPix().
 * - Guarda las coordenadas en corners.
 * - Devuelve true.
 *
 * Si no lo encuentra, devuelve false.
 */
bool detectCorners(
    const cv::Mat& image,
    std::vector<cv::Point2f>& corners,
    const CalibrationConfig& config
);

/*
 * Escribe una línea de texto sobre una imagen.
 *
 * line indica la posición vertical del texto.
 */
void drawText(
    cv::Mat& image,
    const std::string& text,
    int line,
    const cv::Scalar& color =
        cv::Scalar(255, 255, 255)
);

#endif