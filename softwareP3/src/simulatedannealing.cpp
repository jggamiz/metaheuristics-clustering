#include "simulatedannealing.h"
#include "problemPAR.h"
#include "random.hpp"

#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>

using namespace std;

ResultMH<int> SimulatedAnnealing::optimize(Problem<int>& problem, int maxevals) {

    ProblemPAR* par_problem = dynamic_cast<ProblemPAR*>(&problem);
    int n = (int)par_problem->getSolutionSize();
    auto bounds = par_problem->getSolutionDomainRange();
    int k = bounds.second + 1;   // clusters in [0, k-1]

    // Parameters
    const double mu = 0.2;
    const double phi = 0.3;
    const double Tf = 1e-3;

    int max_vecinos = 10 * n;   // max neighbours generated per step
    int max_exitos = 1 * n; // max neighbours accepted  per step
    int M = maxevals / max_vecinos; // number of cooling steps

    // Generate initial solution (same strategy as LocalSearch)
    // Guarantees every cluster has at least one element
    tSolution<int> current_sol(n, -1);
    vector<int> cluster_counts(k, 0);

    vector<int> available(n);
    iota(available.begin(), available.end(), 0);
    Random::shuffle(available.begin(), available.end());

    // One forced element per cluster
    for (int c = 0; c < k; ++c) {
        int inst = available[c];
        current_sol[inst] = c;
        cluster_counts[c]++;
    }
    
    // Rest assigned at random
    for (int i = k; i < n; ++i) {
        int inst = available[i];
        int c = Random::get<int>(0, k - 1);
        current_sol[inst] = c;
        cluster_counts[c]++;
    }

    double current_fitness = problem.fitness(current_sol);
    unsigned int evaluations = 1;

    // Temperatures
    double T0 = (mu * current_fitness) / (-log(phi));

    // T0 must be strictly greater than Tf
    if (T0 <= Tf) {
        T0 = Tf * 10.0;
        cerr << "[ES] Warning: T0 was <= Tf; clamped to " << T0 << endl;
    }

    // Beta for modified Cauchy cooling
    double beta = (T0 - Tf) / ((double)M * T0 * Tf);

    double T = T0;

    tSolution<int> best_sol = current_sol;
    double best_fitness = current_fitness;

    // Instance order for neighbourhood without repetition
    vector<int> instance_order(n);
    iota(instance_order.begin(), instance_order.end(), 0);

    // Main loop (outer: one step per cooling)
    while (evaluations < (unsigned int)maxevals) {
        int n_exitos = 0;
        int n_vecinos = 0;

        // Shuffle instances at the start of each temperature step
        Random::shuffle(instance_order.begin(), instance_order.end());
        int move_idx = 0;

        // Inner loop L(T)
        while (n_exitos < max_exitos  && n_vecinos < max_vecinos && evaluations < (unsigned int)maxevals) {
            // Recycle the shuffled list if exhausted within this step
            if (move_idx >= n) {
                Random::shuffle(instance_order.begin(), instance_order.end());
                move_idx = 0;
            }

            int inst = instance_order[move_idx++];
            int old_cluster = current_sol[inst];

            // Skip moves that would leave a cluster empty (not counted as vecino)
            if (cluster_counts[old_cluster] <= 1) continue;

            // Pick a random new cluster different from the current one
            int new_cluster;
            do {
                new_cluster = Random::get<int>(0, k - 1);
            } while (new_cluster == old_cluster);

            // Evaluate neighbour
            tSolution<int> neighbor = current_sol;
            neighbor[inst] = new_cluster;

            double neighbor_fitness = problem.fitness(neighbor);
            ++evaluations;
            ++n_vecinos;

            // Acceptance criterion (minimisation):
            //   delta_f < 0  => always accept  (improvement)
            //   delta_f >= 0 => accept with probability exp(-delta_f / T)
            double delta_f = neighbor_fitness - current_fitness;

            if (delta_f < 0.0 || Random::get<double>(0.0, 1.0) <= exp(-delta_f / T)) {

                current_sol = neighbor;
                current_fitness = neighbor_fitness;
                cluster_counts[old_cluster]--;
                cluster_counts[new_cluster]++;
                ++n_exitos;

                if (current_fitness < best_fitness) {
                    best_sol = current_sol;
                    best_fitness = current_fitness;
                }
            }
        } // end inner loop

        // Modified Cauchy cooling
        T = T / (1.0 + beta * T);

        // We stop if no solution was accepted in this step
        if (n_exitos == 0) break;

    } // end outer loop

    return ResultMH<int>(best_sol, best_fitness, evaluations);
}