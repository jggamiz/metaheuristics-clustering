#include "greedy.h"
#include "problemPAR.h"
#include "random.hpp"
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <numeric>

using namespace std;

ResultMH<int> GreedySearch::optimize(Problem<int>& problem, int maxevals) {
    (void)maxevals; // Solo evaluamos la solución final al terminar

    ProblemPAR* par_problem = dynamic_cast<ProblemPAR*>(&problem);
    const auto& features    = par_problem->getFeatures();
    const auto& constraints = par_problem->getConstraints();

    int num_instances = (int)features.size();
    int num_features  = (int)features[0].size();
    int k_clusters    = par_problem->getSolutionDomainRange().second + 1;

    // Generar k centroides iniciales aleatorios dentro del dominio
    vector<int> idx(num_instances);
    iota(idx.begin(), idx.end(), 0);
    Random::shuffle(idx.begin(), idx.end());

    vector<vector<double>> centroids(k_clusters, vector<double>(num_features, 0.0));
    tSolution<int>         sol(num_instances, -1);
    vector<int>            cluster_counts(k_clusters, 0);

    for (int c = 0; c < k_clusters; ++c) {
        int inst = idx[c];
        centroids[c]   = features[inst];
        sol[inst]      = c;
        cluster_counts[c]++;
    }

    // Barajamos los índices de las instancias
    vector<int> indices(num_instances);
    iota(indices.begin(), indices.end(), 0);

    bool changed = true;

    // BUCLE K-MEANS
    while (changed) {
        changed = false;
        Random::shuffle(indices.begin(), indices.end());

        for (int i : indices) {
            int current_cluster = sol[i];

            // Cannot move the last element of a cluster bc that would leave it empty (invalid solution)
            if (current_cluster != -1 && cluster_counts[current_cluster] <= 1) 
                continue;

            int best_cluster = -1;
            int min_violations = numeric_limits<int>::max();
            double min_dist = numeric_limits<double>::infinity();

            for (int c = 0; c < k_clusters; ++c) {
                // Euclidean distance to centroid c
                double dist = 0.0;
                for (int f = 0; f < num_features; ++f) {
                    double diff = features[i][f] - centroids[c][f];
                    dist += diff * diff;
                }
                dist = sqrt(dist);

                // Infeasibility if we assign i to cluster c
                int violations = 0;
                for (int j = 0; j < num_instances; ++j) {
                    if (i == j || sol[j] == -1) continue;
                    int res = constraints[i][j];
                    if (res != 0) {
                        bool same = (c == sol[j]);
                        if (res ==  1 && !same) violations++;
                        if (res == -1 &&  same) violations++;
                    }
                }

                // Primary: fewer violations; secondary: closer centroid
                if (violations < min_violations ||
                   (violations == min_violations && dist < min_dist)) {
                    min_violations = violations;
                    min_dist = dist;
                    best_cluster = c;
                }
            }

            // Apply the move and update counts
            if (best_cluster != -1 && sol[i] != best_cluster) {
                if (current_cluster != -1)
                    cluster_counts[current_cluster]--;
                cluster_counts[best_cluster]++;
                sol[i] = best_cluster;
                changed = true;
            }
        }

        //  Centroid update
        vector<vector<double>> new_centroids(k_clusters, vector<double>(num_features, 0.0));
        vector<int> counts(k_clusters, 0);

        for (int i = 0; i < num_instances; ++i) {
            int c = sol[i];
            if (c == -1) continue;
            counts[c]++;
            for (int f = 0; f < num_features; ++f)
                new_centroids[c][f] += features[i][f];
        }

        for (int c = 0; c < k_clusters; ++c) {
            if (counts[c] > 0) {
                for (int f = 0; f < num_features; ++f)
                    centroids[c][f] = new_centroids[c][f] / counts[c];
            }
        }
    }

    // Final validity check (defensive)
    par_problem->fix(sol);
    float fitness_final = par_problem->fitness(sol);
    return ResultMH<int>(sol, fitness_final, 1);
}