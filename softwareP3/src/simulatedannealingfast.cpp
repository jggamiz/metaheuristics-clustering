#include "simulatedannealingfast.h"
#include "problemPAR.h"
#include "random.hpp"
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

ResultMH<int> SimulatedAnnealingFast::optimize(Problem<int>& problem, int maxevals) {

    ProblemPAR* par = dynamic_cast<ProblemPAR*>(&problem);
    int n = (int)par->getSolutionSize();
    auto bounds = par->getSolutionDomainRange();
    int k = bounds.second + 1;

    // Parameters (identical to SimulatedAnnealing)
    const double mu  = 0.2;
    const double phi = 0.3;
    const double Tf = 1e-3;

    int max_vecinos = 10 * n;
    int max_exitos =  1 * n;
    int M = maxevals / max_vecinos;

    // Initial solution (same seeding as LocalSearch)
    tSolution<int> current_sol(n, -1);
    vector<int> cluster_counts(k, 0);
    {
        vector<int> available(n);
        iota(available.begin(), available.end(), 0);
        Random::shuffle(available.begin(), available.end());
        for (int c = 0; c < k; ++c) {
            current_sol[available[c]] = c;
            cluster_counts[c]++;
        }
        for (int i = k; i < n; ++i) {
            int c = Random::get<int>(0, k - 1);
            current_sol[available[i]] = c;
            cluster_counts[c]++;
        }
    }

    // Initial fitness (full evaluation)
    double current_dev = par->calculateDeviation(current_sol);
    int    current_infeas = par->calculateInfeasibility(current_sol);
    double current_fit = current_dev + par->getLambda() * current_infeas;
    unsigned int evaluations = 1;

    // Temperature
    double T0 = (mu * current_fit) / (-log(phi));
    if (T0 <= Tf) { T0 = Tf * 10.0; cerr << "[ES-Fast] T0 clamped\n"; }
    double beta = (T0 - Tf) / ((double)M * T0 * Tf);
    double T = T0;

    tSolution<int> best_sol = current_sol;
    double best_fitness = current_fit;

    vector<int> instance_order(n);
    iota(instance_order.begin(), instance_order.end(), 0);

    // Main loop
    while (evaluations < (unsigned int)maxevals) {

        int n_exitos = 0;
        int n_vecinos = 0;

        Random::shuffle(instance_order.begin(), instance_order.end());
        int move_idx = 0;

        while (n_exitos < max_exitos  && n_vecinos < max_vecinos && evaluations < (unsigned int)maxevals) {

            if (move_idx >= n) {
                Random::shuffle(instance_order.begin(), instance_order.end());
                move_idx = 0;
            }

            int inst = instance_order[move_idx++];
            int old_cluster = current_sol[inst];
            if (cluster_counts[old_cluster] <= 1) continue;

            int new_cluster;
            do { new_cluster = Random::get<int>(0, k - 1); }
            while (new_cluster == old_cluster);

            // Incremental infeasibility delta
            // O(|constraints of inst|) instead of O(n²)
            int infeas_delta = par->calculateInfeasibilityDelta(
                                   current_sol, inst, old_cluster, new_cluster);
            int new_infeas = current_infeas + infeas_delta;

            // Deviation requires full recalc (centroids shift)
            // We approximate the fitness delta using ONLY the infeasibility
            // term for the acceptance test, then recompute deviation only
            // on accepted moves.  This keeps rejected moves O(constraints).
            double lambda = par->getLambda();

            // Approximate delta_f (used for acceptance criterion only)
            double delta_f_approx = lambda * infeas_delta;
            // If infeasibility alone already worsens things enough, use it
            // Otherwise we need the deviation contribution too — compute it
            double delta_f;
            if (delta_f_approx > 0.0 && Random::get<double>(0.0, 1.0) > exp(-delta_f_approx / T)) {
                // Rejected on infeasibility alone — skip full deviation eval
                ++evaluations;
                ++n_vecinos;
                continue;
            }

            // Full evaluation needed (either promising or borderline)
            tSolution<int> neighbor = current_sol;
            neighbor[inst] = new_cluster;
            double new_dev = par->calculateDeviation(neighbor);
            double new_fit = new_dev + lambda * new_infeas;
            delta_f = new_fit - current_fit;

            ++evaluations;
            ++n_vecinos;

            if (delta_f < 0.0 || Random::get<double>(0.0, 1.0) <= exp(-delta_f / T)) {

                current_sol = neighbor;
                current_dev = new_dev;
                current_infeas = new_infeas;
                current_fit = new_fit;
                cluster_counts[old_cluster]--;
                cluster_counts[new_cluster]++;
                ++n_exitos;

                if (current_fit < best_fitness) {
                    best_sol = current_sol;
                    best_fitness = current_fit;
                }
            }
        }

        T = T / (1.0 + beta * T);
        if (n_exitos == 0) break;
    }

    return ResultMH<int>(best_sol, best_fitness, evaluations);
}