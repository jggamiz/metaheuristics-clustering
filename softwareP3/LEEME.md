En el directorio `software` encontramos:

- `common`: Contiene las clases base y utilidades generales (`mh.h`, `problem.h`, `solution.h`, `random.hpp`, etc.). Estos archivos proporcionan la interfaz fundamental de la que heredan nuestros algoritmos y representaciones.
- `data`: es el directorio que contiene los conjuntos de datos sobre los que trabaja el programa (`bupa_set`, `glass_set` y `zoo_set`), junto con sus correspondientes archivos de restricciones (`_const_15.dat` y `_const_30.dat`).
- `graficas`: es un directorio destinado a almacenar las imágenes y diagramas de caja (boxplots) generados automáticamente tras la ejecución de los experimentos.
- `inc`: es un directorio incluido en la plantilla que hemos modificado para incluir las cabeceras de nuestro problema y de todos los algoritmos implementados:
    - `bmb.h`: contiene la declaración de la clase `BMB` (Búsqueda Multi-Arranque Básica), heredada de `MH`.
    - `geneticalgorithm.h`: contiene la declaración de la clase `GeneticAlgorithm`, que implementa el Algoritmo Memético AM-BEST (y variantes base). Permite configurar el operador de cruce, tamaño de población, probabilidades de cruce/mutación y búsqueda local.
    - `grasp.h`: contiene la declaración de la clase `GRASP`, heredada de `MH`.
    - `greedy.h`: contiene la declaración de la clase `GreedySearch`, heredada de `MH`. Incluye el método `optimize` para el enfoque constructivo.
    - `ils.h`: contiene la declaración de la clase `ILS` (Búsqueda Local Iterada), heredada de `MH`.
    - `ils_es.h`: contiene la declaración de la clase `ILS_ES` (Híbrido de ILS con Enfriamiento Simulado), heredada de `MH`.
    - `localsearch.h`: contiene la declaración de la clase `LocalSearch`, heredada de `MH`. Incluye el método `optimize` para explorar el vecindario.
    - `problemPAR.h`: contiene la declaración de la clase `ProblemPAR`, heredada de `Problem`. Aquí definimos nuestro Problema de Asignación con Restricciones. Incluye los atributos del problema (instancias y matriz de restricciones), constructores, y métodos clave como el cálculo de la desviación (`calculateDeviation`) y la infeasibility (`calculateInfeasibility`).
    - `randomsearch.h`: contiene la declaración de la clase `RandomSearch`, heredada de `MH`.
    - `simulatedannealing.h`: contiene la declaración de la clase `SimulatedAnnealing` (Enfriamiento Simulado), heredada de `MH`.
    - `simulatedannealingfast.h`: contiene la declaración de la clase `SimulatedAnnealingFast` (variante rápida voluntaria de ES), heredada de `MH`.
- `src`: es el directorio que contiene las implementaciones de las clases definidas en `inc`:
    - `bmb.cpp`, `geneticalgorithm.cpp`, `grasp.cpp`, `greedy.cpp`, `ils.cpp`, `ils_es.cpp`, `localsearch.cpp`, `randomsearch.cpp`, `simulatedannealing.cpp`, `simulatedannealingfast.cpp`: contienen la implementación completa de la lógica de cada algoritmo y de su respectivo método `optimize`.
    - `problemPAR.cpp`: contiene la implementación de los constructores, la lectura de ficheros y las funciones de evaluación (fitness).
- `bin`: es la carpeta destino donde el compilador genera los archivos objeto (`.o`) y el ejecutable final resultante (`par_solver`).

También se incluyen los siguientes archivos en la raíz del proyecto: 
- `main.cpp`: Punto de entrada del programa que gestiona los parámetros por consola y lanza la optimización.
- `Makefile`: Archivo de configuración para automatizar la compilación de forma optimizada simplemente usando el comando `make`.
- `run_all_tests.sh`: Script ejecutable en Bash para la ejecución en lote de toda la batería de pruebas (algoritmos, datasets, k=15 y k=30, y las 50 semillas).
- `graf.py`: Script en Python encargado de leer los resultados y generar las gráficas comparativas (boxplots) automáticamente.
- `wilcoxon.py`: Script en Python que realiza el test de los rangos signados de Wilcoxon para evaluar la significancia de los resultados.
- `det_results.csv` y `avg_results.csv`: archivos generados automáticamente que guardan los resultados detallados y promedios de las ejecuciones, respectivamente.