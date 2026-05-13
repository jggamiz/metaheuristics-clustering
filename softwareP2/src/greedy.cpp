#include "greedy.h"
#include "problemPAR.h"
#include "random.hpp"
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>

using namespace std;

ResultMH<int> GreedySearch::optimize(Problem<int>& problem, int maxevals) {
    (void)maxevals; // Solo evaluamos la solución final al terminar

    ProblemPAR* par_problem = dynamic_cast<ProblemPAR*>(&problem);
    const auto& features = par_problem->getFeatures();
    const auto& constraints = par_problem->getConstraints();
    
    int num_instances = features.size();
    int num_features = features[0].size();
    int k_clusters = par_problem->getSolutionDomainRange().second + 1;

    // Averiguar el dominio de cada dimensión (máximos y mínimos)
    vector<pair<double, double>> bounds(num_features, {numeric_limits<double>::infinity(), -numeric_limits<double>::infinity()});
    for (int i=0; i < num_instances; ++i) {
        for (int f=0; f<num_features; ++f) {
            if (features[i][f] < bounds[f].first) bounds[f].first = features[i][f];
            if (features[i][f] > bounds[f].second) bounds[f].second = features[i][f];
        }
    }

    // Generar k centroides iniciales aleatorios dentro del dominio
    vector<vector<double>> centroids(k_clusters, vector<double>(num_features));
    for (int c=0; c<k_clusters; ++c) {
        for (int f=0; f<num_features; ++f) {
            // Genera un double aleatorio entre el min y max de esa característica
            centroids[c][f] = Random::get<double>(bounds[f].first, bounds[f].second);
        }
    }

    tSolution<int> sol(num_instances, -1);
    bool changed = true;

    vector<int> indices(num_instances);
    for (int i=0; i<num_instances; i++) indices[i] = i;

    // BUCLE K-MEANS
    while (changed) {
        changed = false;
        
        // Barajamos los índices de las instancias
        Random::shuffle(indices.begin(), indices.end());
        
        // Asignación de instancias
        for (int i : indices) {
            int best_cluster = -1;
            int min_violations = numeric_limits<int>::max();
            double min_dist = numeric_limits<double>::infinity();
            
            for (int c=0; c<k_clusters; ++c) {
                // Distancia Euclidea de la instancia al centroide 'c'
                double dist = 0.0;
                for (int f=0; f<num_features; ++f) {
                    double diff = features[i][f] - centroids[c][f];
                    dist += diff * diff;
                }
                dist = sqrt(dist);
                
                // Inviabilidades si asignamos 'i' al cluster 'c'
                int violations = 0;
                for (int j=0; j<num_instances; ++j) {
                    // Solo comprobamos con instancias ya asignadas y distintas de i
                    if (i==j || sol[j]==-1) continue; 
                    
                    int res = constraints[i][j];
                    if (res != 0) {
                        bool same_cluster = (c == sol[j]);
                        if (res == 1 && !same_cluster) violations++;
                        if (res == -1 && same_cluster) violations++;
                    }
                }
                
                // Criterio de selección: menor inviabilidad y desempate por distancia
                if (violations < min_violations || (violations == min_violations && dist < min_dist)) {
                    min_violations = violations;
                    min_dist = dist;
                    best_cluster = c;
                }
            }
            
            // Si la instancia ha cambiado de grupo, marcamos para seguir repitiendo
            if (sol[i] != best_cluster) {
                sol[i] = best_cluster;
                changed = true;
            }
        }
        
        // Actualización de los centroides con el promedio
        vector<int> counts(k_clusters, 0);
        vector<vector<double>> new_centroids(k_clusters, vector<double>(num_features, 0.0));
        
        for (int i=0; i<num_instances; ++i) {
            int c = sol[i];
            if (c != -1) {
                counts[c]++;
                for (int f=0; f<num_features; ++f) {
                    new_centroids[c][f] += features[i][f];
                }
            }
        }
        
        for (int c=0; c<k_clusters; ++c) {
            if (counts[c] > 0) {
                for (int f=0; f<num_features; ++f) {
                    centroids[c][f] = new_centroids[c][f] / counts[c];
                }
            }
        }
    }

    // Una vez converge evaluamos la solución final
    float fitness_final = par_problem->fitness(sol);

    return ResultMH<int>(sol, fitness_final, 1);
}