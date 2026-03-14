En el directorio `software` encontramos:

- `common`: es el mismo directorio de la plantilla proporcionada en la asignatura. Incluye archivos base como `solution.h`, `problem.h`, `mh.h` y `random.hpp`.
- `data`: es el directorio que contiene los conjuntos de datos sobre los que trabaja el programa (`bupa_set`, `glass_set` y `zoo_set`), junto con sus correspondientes archivos de restricciones (`_const_15.dat` y `_const_30.dat`).
- `graficas`: es un directorio destinado a almacenar las imágenes y diagramas de caja (boxplots) generados automáticamente tras la ejecución de los experimentos.
- `inc`: es un directorio incluido en la plantilla. Lo hemos modificado para incluir las cabeceras de nuestros algoritmos y problema:
    - `greedy.h`: contiene la declaración de la clase `GreedySearch`, heredada de `MH`. Incluye el método `optimize` para el enfoque constructivo.
    - `localsearch.h`: contiene la declaración de la clase `LocalSearch`, heredada de `MH`. Incluye el método `optimize` para explorar el vecindario.
    - `problemPAR.h`: contiene la declaración de la clase `ProblemPAR`, heredada de `Problem`. Aquí definimos nuestro Problema de Asignación con Restricciones. Incluye los atributos del problema (instancias y matriz de restricciones), los constructores, y métodos clave como el cálculo de la desviación (`calculateDeviation`) y la infeasibility (`calculateInfeasibility`).
    - `randomsearch.h`: contiene la declaración de la clase `RandomSearch`, heredada de `MH`, con su correspondiente método `optimize`.
- `src`: es un directorio incluido en la plantilla. Lo hemos modificado para incluir las implementaciones de las clases definidas en `inc`:
    - `greedy.cpp`: contiene la implementación de `optimize` para el algoritmo greedy.
    - `localsearch.cpp`: contiene la implementación de `optimize` para el algoritmo de búsqueda local.
    - `problemPAR.cpp`: contiene la implementación de los constructores, la lectura de ficheros y las funciones de evaluación (fitness).
    - `randomsearch.cpp`: contiene la implementación de `optimize` para el algoritmo de búsqueda aleatoria.
- `bin`: es el directorio donde el proceso de compilación guarda los archivos objeto (`.o`) y el ejecutable final resultante (`par_solver`).

También se incluyen los siguientes archivos en la raíz del proyecto: 
- `main.cpp`: donde se lleva a cabo la ejecución principal del programa y la gestión de argumentos por terminal.
- `Makefile`: para compilar todo el proyecto de forma automática y optimizada simplemente usando el comando `make`.
- `run_all_tests.sh`: script ejecutable en Bash que automatiza el lanzamiento de toda la batería de pruebas (algoritmos, datasets, k=15 y k=30, y las 50 semillas).
- `graf.py`: script en Python encargado de leer los resultados y generar las gráficas comparativas.
- `det_results.csv` y `avg_results.csv`: archivos generados automáticamente que guardan los resultados detallados y promedios de las ejecuciones, respectivamente.