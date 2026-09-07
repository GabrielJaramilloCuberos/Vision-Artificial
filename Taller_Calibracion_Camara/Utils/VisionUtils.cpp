#include "VisionUtils.hpp"

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

/*
 * Crear las coordenadas 3D conocidas del tablero.
 */
std::vector<cv::Point3f> createObjectPoints(
    const CalibrationConfig& config
) {
    std::vector<cv::Point3f> points;

    points.reserve(
        config.patternSize.area()
    );

    /*
     * Para un tablero de 7 x 7 esquinas y casillas
     * de 21 mm, los primeros puntos serán:
     *
     * (0, 0, 0)
     * (21, 0, 0)
     * (42, 0, 0)
     * ...
     * (0, 21, 0)
     *
     * Z siempre es 0 porque el tablero es plano.
     */
    for (
        int row = 0;
        row < config.patternSize.height;
        row++
    ) {
        for (
            int column = 0;
            column < config.patternSize.width;
            column++
        ) {
            points.emplace_back(
                column * config.squareSizeMm,
                row * config.squareSizeMm,
                0.0f
            );
        }
    }

    return points;
}

/*
 * Detectar las esquinas interiores del tablero.
 */
bool detectCorners(
    const cv::Mat& image,
    std::vector<cv::Point2f>& corners,
    const CalibrationConfig& config
) {
    if (image.empty()) {
        return false;
    }

    cv::Mat gray;

    /*
     * La detección de esquinas no necesita el color.
     * Por eso convertimos el frame a escala de grises.
     */
    cv::cvtColor(
        image,
        gray,
        cv::COLOR_BGR2GRAY
    );

    /*
     * Opciones utilizadas por el detector:
     *
     * ADAPTIVE_THRESH:
     * ayuda cuando la iluminación no es uniforme.
     *
     * NORMALIZE_IMAGE:
     * mejora el contraste antes de buscar el patrón.
     *
     * FAST_CHECK:
     * permite descartar rápidamente imágenes donde
     * claramente no aparece el tablero.
     */
    int flags =
        cv::CALIB_CB_ADAPTIVE_THRESH |
        cv::CALIB_CB_NORMALIZE_IMAGE |
        cv::CALIB_CB_FAST_CHECK;

    bool found = cv::findChessboardCorners(
        gray,
        config.patternSize,
        corners,
        flags
    );

    if (found) {

        /*
         * findChessboardCorners() entrega posiciones
         * aproximadas.
         *
         * cornerSubPix() las mejora usando coordenadas
         * decimales, por ejemplo:
         *
         * Aproximada: (315, 204)
         * Refinada:   (315.38, 203.76)
         */
        cv::cornerSubPix(
            gray,
            corners,
            cv::Size(11, 11),
            cv::Size(-1, -1),
            cv::TermCriteria(
                cv::TermCriteria::EPS |
                cv::TermCriteria::COUNT,
                30,
                0.001
            )
        );
    }

    return found;
}

/*
 * Mostrar texto con un borde negro para mantenerlo
 * visible sobre zonas claras y oscuras.
 */
void drawText(
    cv::Mat& image,
    const std::string& text,
    int line,
    const cv::Scalar& color
) {
    cv::Point position(
        15,
        30 + line * 30
    );

    // Dibujar primero un borde negro grueso.
    cv::putText(
        image,
        text,
        position,
        cv::FONT_HERSHEY_SIMPLEX,
        0.7,
        cv::Scalar(0, 0, 0),
        4,
        cv::LINE_AA
    );

    // Dibujar encima el texto del color solicitado.
    cv::putText(
        image,
        text,
        position,
        cv::FONT_HERSHEY_SIMPLEX,
        0.7,
        color,
        2,
        cv::LINE_AA
    );
}