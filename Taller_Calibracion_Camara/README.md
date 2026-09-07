# Calibración de cámara con OpenCV

Este proyecto implementa la calibración de una cámara estándar mediante fotografías de un tablero de ajedrez. La aplicación permite capturar las imágenes, calcular los parámetros internos de la cámara, guardar los resultados y probar la corrección de distorsión en tiempo real.

## Estructura del proyecto

```text
CalibracionCamara/
├── main.cpp
├── CalibrationConfig.hpp
├── VisionUtils.hpp
├── VisionUtils.cpp
├── CameraCalibration.hpp
├── CameraCalibration.cpp
├── calibration_images/
└── camera_calibration.yaml
```

### `main.cpp`

Contiene el menú principal y llama las funciones correspondientes a cada opción.

### `CalibrationConfig.hpp`

Contiene la configuración compartida:

- Número de esquinas interiores.
- Tamaño real de las casillas.
- Carpeta de fotografías.
- Nombre del YAML.
- Índice de la cámara.
- Cantidad mínima de imágenes.

### `VisionUtils.hpp` y `VisionUtils.cpp`

Contienen funciones auxiliares para:

- Generar las coordenadas reales del tablero.
- Detectar las esquinas interiores.
- Refinar las esquinas con precisión subpíxel.
- Dibujar información sobre las imágenes.

### `CameraCalibration.hpp` y `CameraCalibration.cpp`

Contienen las funciones principales:

- Capturar fotografías.
- Calibrar la cámara.
- Guardar los parámetros.
- Leer el YAML.
- Corregir la distorsión en tiempo real.

## Configuración del tablero

El tablero utilizado tiene 8 × 8 casillas, por lo que contiene 7 × 7 esquinas interiores:

```cpp
cv::Size patternSize{7, 7};
```

Cada casilla mide 21 mm:

```cpp
float squareSizeMm{21.0f};
```

Si se utiliza otro tablero, ambos valores deben modificarse en `CalibrationConfig.hpp`.

## Uso

### 1. Capturar las imágenes

Ejecutar la aplicación y seleccionar:

```text
1. Capturar imagenes del tablero
```

Cuando el tablero sea detectado aparecerán puntos y líneas de colores. Presionar `ESPACIO` para guardar la imagen.

Se recomienda capturar entre 15 y 25 imágenes del mismo tablero desde diferentes posiciones e inclinaciones. El tablero debe aparecer completo, enfocado y plano.

Las fotografías se guardan en:

```text
calibration_images/
```

### 2. Calibrar la cámara

Regresar al menú con `ESC` y seleccionar:

```text
2. Calibrar y guardar parametros
```

El programa busca las fotografías mediante `cv::glob()`, las abre con `cv::imread()` y detecta las esquinas mediante `cv::findChessboardCorners()`.

`cv::cornerSubPix()` refina las posiciones detectadas. Después, `cv::calibrateCamera()` relaciona las coordenadas reales del tablero con las coordenadas observadas en píxeles.

Como resultado se obtienen:

- Matriz intrínseca de la cámara.
- Coeficientes de distorsión.
- Vectores de rotación y traslación.
- Error RMS de reproyección.

Los resultados se guardan en:

```text
camera_calibration.yaml
```

También se crea:

```text
calibration_comparison.jpg
```

Esta imagen contiene una fotografía original y su versión corregida.

### 3. Probar la corrección

Seleccionar:

```text
3. Probar correccion en vivo
```

El programa abre el YAML mediante `cv::FileStorage` y recupera la matriz intrínseca y los coeficientes de distorsión.

`cv::initUndistortRectifyMap()` crea los mapas de corrección y `cv::remap()` los aplica a cada frame de la cámara. La aplicación muestra la imagen original y la corregida simultáneamente.

Presionar `ESC` para regresar al menú.

## Funciones principales

### `cv::VideoCapture()`

Abre la cámara seleccionada.

### `cv::findChessboardCorners()`

Detecta las esquinas interiores del tablero.

### `cv::cornerSubPix()`

Refina las coordenadas de las esquinas con precisión subpíxel.

### `cv::drawChessboardCorners()`

Dibuja los puntos y líneas que permiten comprobar la detección.

### `cv::glob()`

Busca las rutas de las fotografías guardadas.

### `cv::calibrateCamera()`

Calcula la matriz intrínseca, los coeficientes de distorsión y la posición del tablero en cada fotografía.

### `cv::FileStorage`

Guarda y recupera los parámetros en formato YAML.

### `cv::initUndistortRectifyMap()`

Calcula cómo deben reorganizarse los píxeles para corregir la distorsión.

### `cv::remap()`

Aplica los mapas de corrección a cada frame en tiempo real.

## Interpretación del error

El error de reproyección expresa la diferencia entre las esquinas detectadas y las posiciones predichas por el modelo calibrado.

Como referencia:

- Menor de 0.5 píxeles: resultado muy bueno.
- Entre 0.5 y 1.0 píxeles: resultado generalmente aceptable.
- Mayor de 1.0 píxeles: conviene mejorar las fotografías.

## Recomendaciones

- Utilizar siempre la misma cámara.
- Mantener la misma resolución y nivel de zoom.
- No doblar el tablero.
- Evitar fotografías desenfocadas.
- Cubrir el centro, los bordes y las esquinas del frame.
- Evitar capturas casi idénticas.