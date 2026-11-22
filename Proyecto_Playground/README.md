# Vision Playground con OpenCV 5

- Autor: Gabriel Jaramillo Cuberos
- Materia: Visión Artificial

## Descripción

Vision Playground es un laboratorio interactivo en C++ que permite
experimentar con los principales temas de procesamiento de imágenes
estudiados durante el curso.

La aplicación permite seleccionar una imagen de prueba, cargar una
fotografía o utilizar la cámara. Cada módulo muestra el original y
el resultado del procesamiento.

El proyecto utiliza OpenCV 5.0.0.

## Arquitectura

El programa separa el menú, la interfaz compartida y los algoritmos.

- main.cpp: muestra el menú y ejecuta el módulo elegido.
- Temas.hpp: declara las funciones de los módulos.
- Laboratorio.hpp y Laboratorio.cpp: selección de fuente, controles,
  visualización y guardado de resultados.
- Cada tema tiene su propio archivo .cpp.

Las funciones de procesamiento reciben una imagen y parámetros
interactivos. Devuelven una nueva imagen.

Las guardas #ifndef, #define y #endif evitan que los headers
se procesen repetidamente dentro de una misma unidad de compilación.

## Estructura

```text
VisionPlayground/
├── main.cpp
├── Temas.hpp
├── Laboratorio.hpp
├── Laboratorio.cpp
├── Introduccion.cpp
├── Representacion.cpp
├── Transformaciones.cpp
├── Camara.cpp
├── Filtros.cpp
├── Frecuencia.cpp
├── Histogramas.cpp
├── Umbrales.cpp
├── Morfologia.cpp
├── Segmentacion.cpp
├── Caracterizacion.cpp
├── Redes.cpp
├── calibration_images/
├── camera_calibration.yaml
└── resultados/
```

## Módulos

### Introducción

Permite observar la imagen como matriz, extraer bordes y contar
componentes conectados.

### Representación e iluminación

Incluye escala de grises, canales BGR, brillo, contraste, gamma
y cuantización.

Las modificaciones digitales de brillo no equivalen a cambiar
la apertura, el tiempo de exposición o el ISO del dispositivo.

### Transformaciones

Incluye traslación, rotación, escala, cizallamiento, reflexiones
y transformación de perspectiva.

La transformación afín utiliza una matriz 2x3.
La transformación proyectiva utiliza una matriz 3x3.

### Cámara y calibración

Permite capturar fotografías del tablero, calibrar la cámara,
guardar los parámetros en YAML y corregir imágenes.

Configuración inicial:

- Tablero de 8x8 casillas.
- 7x7 esquinas interiores.
- Casillas de 21 mm.

Estos valores se encuentran al inicio de Camara.cpp.

Se recomienda capturar entre 15 y 25 fotografías con distintas
posiciones e inclinaciones.

El programa calcula:

- Matriz intrínseca K.
- Coeficientes de distorsión D.
- Rotación y traslación del tablero en cada fotografía.
- Error RMS global.
- Error RMS por fotografía.

Los errores por fotografía se guardan en errores_calibracion.csv.

La calibración implementada es para una cámara estándar.
Las vistas de deformación radial y angular son simulaciones
didácticas y no reemplazan una calibración de una lente fisheye.

### Filtros espaciales

Incluye caja, gaussiano, mediana, bilateral, Sobel, Laplaciano,
Canny, Prewitt, Roberts, nitidez, mínimo y máximo.

Los filtros permiten comparar reducción de ruido, suavizado
y realce de cambios de intensidad.

### Frecuencia

Incluye espectro DFT, paso bajo, paso alto, paso banda,
DCT por bloques de 8x8 y una demostración de ortogonalidad.

La demostración de DCT elimina coeficientes de alta frecuencia
y reconstruye cada bloque. No implementa el formato JPEG completo.

Los filtros ideales de frecuencia pueden producir oscilaciones
cerca de bordes fuertes.

### Histogramas

Incluye histogramas de gris y BGR, ecualización global, CLAHE
y visualización del histograma después de ecualizar.

### Umbralización y binarización

Incluye umbral binario, inverso, truncamiento, a cero,
Otsu y umbralización adaptativa por media o gaussiana.

Truncamiento y a cero son operaciones de umbralización,
pero sus resultados no necesariamente son binarios.

### Morfología

Incluye erosión, dilatación, apertura, cierre, gradiente,
top hat, black hat y reconstrucción por dilatación geodésica.

Permite trabajar en gris o sobre una máscara binaria.
El elemento estructurante implementado es plano y elíptico.

La dilatación y erosión ilustran operaciones de crecimiento
y reducción de conjuntos relacionadas con la morfología de Minkowski.

### Segmentación

Incluye máscaras HSV, componentes conectados, watershed,
K-means y GrabCut.

También contiene una demostración basada en reglas de color:

- Clases: agrupa objetos rojos, verdes y azules.
- Instancias: distingue componentes dentro de cada clase.
- Vista panóptica didáctica: combina clase, identificador
  de componente y fondo.

Estas vistas no utilizan una red neuronal entrenada ni reconocen
categorías semánticas generales como persona, coche o carretera.

### Caracterización

Incluye área en píxeles, perímetro de contorno, circularidad,
centroides, orientación mediante PCA, media, desviación estándar,
entropía y textura GLCM horizontal con 16 niveles.

También incluye Harris, SIFT, ORB y correspondencias entre una
imagen y una copia transformada.

SIFT usa distancia L2. ORB usa distancia Hamming.

La geometría se expresa en píxeles. No constituye una medición
física en milímetros de objetos arbitrarios.

### CNN, RNN y ViT

Contiene demostraciones de:

- Filtrado seguido de ReLU.
- Max pooling 2x2.
- Recurrencia con estado.
- División de imágenes en parches.
- Embeddings de color y posición.
- Pesos de atención normalizados mediante softmax.

Son operaciones didácticas con parámetros fijos.
No implementan entrenamiento, backpropagation ni modelos completos
como U-Net, Mask R-CNN o Mask2Former.

## Compatibilidad

La aplicación utiliza funciones presentes en OpenCV 5.0.0.

Se utilizan los headers geometry.hpp, calib.hpp y features.hpp
cuando corresponde a la organización de OpenCV 5.

No utiliza CascadeClassifier, Facemark, SURF ni el extractor BRIEF
independiente, que requieren módulos adicionales en esta instalación.

## Compilación en Visual Studio

1. Crear un proyecto de consola C++.
2. Agregar todos los archivos .cpp y .hpp.
3. Seleccionar x64.
4. Seleccionar C++17 o posterior.
5. Configurar las rutas de OpenCV.
6. En Debug enlazar opencv_world500d.lib.
7. En Release enlazar opencv_world500.lib.
8. Agregar la carpeta bin de OpenCV al PATH.
9. Mantener un único main() dentro del proyecto.

## Controles

En la consola:

- Seleccionar un tema.
- Elegir 0 para demo, 1 para archivo o 2 para cámara.
- Cuando se solicite una ruta, escribirla sin comillas.

En la ventana:

- Operacion: algoritmo que se desea explorar.
- A y B: parámetros descritos en la consola.
- ESC: regresar al menú.
- S: guardar resultado, original y parámetros.
- P: pausar o reanudar la captura de cámara.

En captura del tablero:

- ESPACIO: guardar una fotografía cuando el patrón está detectado.
- ESC: regresar al submenú.

Para usar el teclado debe estar seleccionada una ventana de OpenCV.

## Guardado de resultados

La carpeta resultados se crea automáticamente.

Cada evidencia contiene:

- Imagen original.
- Imagen procesada.
- YAML con el módulo, la operación y los valores de A y B.

Los identificadores temporales permiten guardar varias evidencias
sin reutilizar el mismo nombre.

Las rutas relativas parten del directorio de trabajo del programa.
Este directorio puede configurarse en las propiedades de depuración
de Visual Studio.

## Pruebas recomendadas

1. Usar la demo y comprobar que el original se mantiene.
2. Separar los canales B, G y R.
3. Comparar mediana y gaussiano.
4. Observar el espectro de Fourier.
5. Reducir los coeficientes conservados en la DCT.
6. Comparar ecualización global y CLAHE.
7. Comparar Otsu con umbralización adaptativa.
8. Comparar erosión y dilatación en binario y gris.
9. Seleccionar el rojo con HSV.
10. Contar componentes y probar watershed.
11. Calcular geometría y visualizar correspondencias SIFT/ORB.
12. Guardar evidencias con S.
13. Cargar el YAML de calibración con la misma resolución y cámara.

Un RMS bajo es una medida del ajuste a las observaciones.
También debe comprobarse la corrección visual con líneas rectas
y fotografías distintas de las usadas para calibrar.

## Limitaciones

- GrabCut presupone que el objeto está dentro del rectángulo central.
- La segmentación por componentes no separa por sí sola objetos unidos.
- Las clases por color dependen de la iluminación.
- Las operaciones costosas pueden reducir la fluidez de la cámara.
- La calibración pertenece a una cámara y configuración determinadas.
- Los ejemplos de redes no realizan reconocimiento aprendido.
- Algunos conceptos avanzados de las diapositivas se explican mediante
  sus operaciones fundamentales, sin implementar todos los modelos
  o aplicaciones mencionados.