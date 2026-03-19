#include "localsearch.h"
#include "problemPAR.h"
#include "random.hpp"
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

ResultMH<int> LocalSearch::optimize(Problem<int>& problem, int maxevals) {
    ProblemPAR* par_problem = dynamic_cast<ProblemPAR*>(&problem);
    int n = par_problem->getSolutionSize();
    auto bounds = par_problem->getSolutionDomainRange();
    int k_clusters = bounds.second + 1; // Dominio [0, k-1]

    unsigned int evaluations = 0;

    // Generar Solución Inicial Aleatoria (Asegurando clústeres no vacíos)
    tSolution<int> current_sol(n, -1);
    vector<int> cluster_counts(k_clusters, 0);

    // Primero asignamos 1 instancia aleatoria a cada cluster para asegurar que no están vacíos
    vector<int> available_instances(n);
    for (int i=0; i<n; i++) available_instances[i] = i;
    Random::shuffle(available_instances.begin(), available_instances.end());

    for (int c=0; c<k_clusters; c++) {
        int inst = available_instances[c];
        current_sol[inst] = c;
        cluster_counts[c]++;
    }

    // Luego asignamos el resto de forma totalmente aleatoria
    for (int i=k_clusters; i<n; i++) {
        int inst = available_instances[i];
        int random_cluster = Random::get<int>(0, k_clusters-1);
        current_sol[inst] = random_cluster;
        cluster_counts[random_cluster]++;
    }

    float current_fitness = problem.fitness(current_sol);
    evaluations++;

    bool improvement = true;

    // Bucle principal de la Búsqueda Local
    while (improvement && evaluations < (unsigned int)maxevals) {
        improvement = false;

        // Exploración en orden aleatorio: generamos todas las parejas (instancia, nuevo_cluster)
        vector<pair<int, int>> neighborhood;
        for (int i=0; i<n; i++) {
            for (int c=0; c<k_clusters; c++) {
                if (current_sol[i] != c) {
                    neighborhood.push_back({i, c});
                }
            }
        }
        Random::shuffle(neighborhood.begin(), neighborhood.end());

        // Explorar el vecindario (Estrategia "Primer Mejor")
        for (const auto& move : neighborhood) {
            if (evaluations >= (unsigned int)maxevals) break;

            int inst = move.first;
            int new_cluster = move.second;
            int old_cluster = current_sol[inst];

            // Restricción: No dejar ningún clúster vacío
            if (cluster_counts[old_cluster] <= 1) continue;

            // Aplicamos el movimiento (creamos solución vecina)
            tSolution<int> neighbor_sol = current_sol;
            neighbor_sol[inst] = new_cluster;

            float neighbor_fitness = problem.fitness(neighbor_sol);
            evaluations++;

            // Criterio de aceptación: estricta mejora
            if (neighbor_fitness < current_fitness) {
                // Aceptamos el Primer Mejor y dejamos de buscar en este entorno
                current_sol = neighbor_sol;
                current_fitness = neighbor_fitness;
                
                // Actualizamos contadores
                cluster_counts[old_cluster]--;
                cluster_counts[new_cluster]++;
                
                improvement = true;
                break; // Rompemos el for interno para regenerar el entorno desde la nueva solución
            }
        }
    }

    return ResultMH<int>(current_sol, current_fitness, evaluations);
}