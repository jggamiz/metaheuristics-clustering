#include "ils_es.h"
#include "ils.h"           // ILS::mutate
#include "problemPAR.h"
#include "random.hpp"
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

// Public entry point
ResultMH<int> ILS_ES::optimize(Problem<int>& problem, int maxevals) {

    ProblemPAR* par_problem = dynamic_cast<ProblemPAR*>(&problem);
    int n = (int)par_problem->getSolutionSize();
    auto bounds = par_problem->getSolutionDomainRange();
    int k = bounds.second + 1;

    const int num_iters         = 5;
    const int evals_per_restart = maxevals / num_iters; // 20000

    unsigned int total_evals = 0;

    // Random initial solution (no empty clusters)
    tSolution<int> initial_sol(n, -1);
    {
        vector<int> available(n);
        iota(available.begin(), available.end(), 0);
        Random::shuffle(available.begin(), available.end());
        for (int c = 0; c < k; ++c) 
            initial_sol[available[c]] = c;
        for (int i = k; i < n; ++i)
            initial_sol[available[i]] = Random::get<int>(0, k - 1);
    }

    // First ES
    ResultMH<int> result = runES(problem, initial_sol, evals_per_restart);
    total_evals += result.evaluations;

    tSolution<int> best_sol = result.solution;
    tFitness best_fitness = result.fitness;

    // Mutate best -> ES -> update best  (4 more times)
    for (int iter = 1; iter < num_iters; ++iter) {
        tSolution<int> mutated = ILS::mutate(best_sol, k);
        result = runES(problem, mutated, evals_per_restart);
        total_evals += result.evaluations;

        if (result.fitness < best_fitness) {
            best_fitness = result.fitness;
            best_sol = result.solution;
        }
    }

    return ResultMH<int>(best_sol, best_fitness, total_evals);
}

// Private: one ES run from a given initial solution
// Parameters identical to standalone SimulatedAnnealing
ResultMH<int> ILS_ES::runES(Problem<int>& problem, tSolution<int> current_sol, int maxevals) {

    ProblemPAR* par_problem = dynamic_cast<ProblemPAR*>(&problem);
    int n = (int)par_problem->getSolutionSize();
    auto bounds = par_problem->getSolutionDomainRange();
    int k = bounds.second + 1;

    // ES parameters
    const double mu = 0.2;
    const double phi = 0.3;
    const double Tf = 1e-3;

    int max_vecinos = 10 * n;
    int max_exitos =  1 * n;
    int M = maxevals / max_vecinos; // number of cooling steps

    // Build cluster counts from the provided solution
    vector<int> cluster_counts(k, 0);
    for (int i = 0; i < n; ++i)
        if (current_sol[i] >= 0 && current_sol[i] < k)
            cluster_counts[current_sol[i]]++;

    double current_fitness = problem.fitness(current_sol);
    unsigned int evaluations = 1;

    // Initial temperature
    double T0 = (mu * current_fitness) / (-log(phi));
    if (T0 <= Tf) {
        T0 = Tf * 10.0;
        cerr << "[ILS-ES] Warning: T0 was <= Tf; clamped to " << T0 << endl;
    }
    double beta = (T0 - Tf) / ((double)M * T0 * Tf);
    double T = T0;

    // Global best for this ES run
    tSolution<int> best_sol = current_sol;
    double best_fitness = current_fitness;

    // Instance order for neighbour generation without repetition
    vector<int> instance_order(n);
    iota(instance_order.begin(), instance_order.end(), 0);

    // Main loop
    while (evaluations < (unsigned int)maxevals) {
        int n_exitos = 0;
        int n_vecinos = 0;

        Random::shuffle(instance_order.begin(), instance_order.end());
        int move_idx = 0;

        while (n_exitos < max_exitos && n_vecinos < max_vecinos && evaluations < (unsigned int)maxevals) {

            // Recycle shuffled list if exhausted within this step
            if (move_idx >= n) {
                Random::shuffle(instance_order.begin(), instance_order.end());
                move_idx = 0;
            }

            int inst        = instance_order[move_idx++];
            int old_cluster = current_sol[inst];

            if (cluster_counts[old_cluster] <= 1) continue;

            int new_cluster;
            do { new_cluster = Random::get<int>(0, k - 1); }
            while (new_cluster == old_cluster);

            tSolution<int> neighbor = current_sol;
            neighbor[inst] = new_cluster;

            double neighbor_fitness = problem.fitness(neighbor);
            ++evaluations;
            ++n_vecinos;

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
        } // inner loop

        // Modified Cauchy cooling
        T = T / (1.0 + beta * T);

        if (n_exitos == 0) break;

    } // outer loop

    return ResultMH<int>(best_sol, best_fitness, evaluations);
}