#include "randomsearch.h"
#include <limits> // Para usar infinito

using namespace std;

ResultMH<int> RandomSearch::optimize(Problem<int>& problem, int maxevals) {
    tSolution<int> best_solution;
    // Empezamos con el peor fitness posible (infinito positivo)
    tFitness best_fitness = numeric_limits<tFitness>::infinity(); 
    unsigned int evaluations = 0;

    for (int i=0; i<maxevals; i++) {
        // Usar la interfaz del problema para crear una solución aleatoria
        tSolution<int> current_solution = problem.createSolution();
        
        // Evaluar la solución
        tFitness current_fitness = problem.fitness(current_solution);
        evaluations++;

        // Si es estrictamente mejor, actualizamos
        if (current_fitness < best_fitness) {
            best_fitness = current_fitness;
            best_solution = current_solution;
        }
    }

    // Devolver la estructura ResultMH
    return ResultMH<int>(best_solution, best_fitness, evaluations);
}