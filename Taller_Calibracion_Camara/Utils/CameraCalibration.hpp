#ifndef CAMERA_CALIBRATION_HPP
#define CAMERA_CALIBRATION_HPP

#include "CalibrationConfig.hpp"

/*
 * Opción 1:
 * abre la cámara, detecta el tablero y guarda fotografías.
 */
void captureCalibrationImages(
    const CalibrationConfig& config
);

/*
 * Opción 2:
 * carga las fotografías, detecta las esquinas,
 * calibra la cámara y guarda el YAML.
 */
void calibrateCameraFromImages(
    const CalibrationConfig& config
);

/*
 * Opción 3:
 * carga el YAML y aplica la corrección en tiempo real.
 */
void testCalibration(
    const CalibrationConfig& config
);

#endif