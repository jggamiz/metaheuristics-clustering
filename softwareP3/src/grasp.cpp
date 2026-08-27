#include "grasp.h"
#include "localsearch.h"
#include "problemPAR.h"
#include "random.hpp"
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace std;

// Public entry point
ResultMH<int> GRASP::optimize(Problem<int>& problem, int maxevals) {

    ProblemPAR* par = dynamic_cast<ProblemPAR*>(&problem);
    auto bounds = par->getSolutionDomainRange();
    int k = bounds.second + 1;

    const int num_iters = 5;
    const int evals_per_restart = maxevals / num_iters; // 20 000
    const double alpha = 0.3; // LRC = top 30 % of LC (by distance)

    tSolution<int> best_sol;
    tFitness best_fitness = numeric_limits<tFitness>::max();
    unsigned int total_evals  = 0;

    LocalSearch ls;

    for (int iter = 0; iter < num_iters; ++iter) {
        // Greedy randomised construction
        tSolution<int> constructed = constructSolution(problem, k, alpha);

        // Local search from the constructed solution
        ResultMH<int> result = ls.optimize(problem, evals_per_restart, constructed);
        total_evals += result.evaluations;

        if (result.fitness < best_fitness) {
            best_fitness = result.fitness;
            best_sol = result.solution;
        }
    }

    return ResultMH<int>(best_sol, best_fitness, total_evals);
}

// Greedy Randomized Construction
tSolution<int> GRASP::constructSolution(Problem<int>& problem, int k, double alpha) {

    ProblemPAR* par = dynamic_cast<ProblemPAR*>(&problem);
    int n = (int)par->getSolutionSize();

    const auto& features = par->getFeatures();  // const ref
    const auto& ml = par->getMLConstraints();
    const auto& cl = par->getCLConstraints();
    int num_features = (int)features[0].size();

    tSolution<int> sol(n, -1);

    // Running centroids: sum of coordinates and count per cluster
    vector<vector<double>> centroid_sum(k, vector<double>(num_features, 0.0));
    vector<int> centroid_cnt(k, 0);

    // Shuffle all indices
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    Random::shuffle(order.begin(), order.end());

    // One element per cluster (first k in shuffled order)
    for (int c = 0; c < k; ++c) {
        int inst = order[c];
        sol[inst] = c;
        for (int f = 0; f < num_features; ++f)
            centroid_sum[c][f] += features[inst][f];
        centroid_cnt[c]++;
    }

    // Assign the rest
    for (int idx = k; idx < n; ++idx) {
        int inst = order[idx];

        // Compute infeasibility increment for each cluster
        // Only counts constraints with already-assigned neighbours.
        vector<int> infeas_inc(k, 0);
        for (int c = 0; c < k; ++c) {
            for (int j : ml[inst]) {
                if (sol[j] != -1 && sol[j] != c) infeas_inc[c]++;
            }
            for (int j : cl[inst]) {
                if (sol[j] != -1 && sol[j] == c) infeas_inc[c]++;
            }
        }

        // LC = clusters achieving the minimum increment
        int min_inc = *min_element(infeas_inc.begin(), infeas_inc.end());
        vector<int> lc;
        for (int c = 0; c < k; ++c)
            if (infeas_inc[c] == min_inc) lc.push_back(c);

        // Sort LC by distance from inst to current cluster centroid (ascending)
        sort(lc.begin(), lc.end(), [&](int ca, int cb) {
            // Distance to centroid of ca
            double dist_a = 0.0, dist_b = 0.0;
            if (centroid_cnt[ca] > 0) {
                for (int f = 0; f < num_features; ++f) {
                    double d = features[inst][f] - centroid_sum[ca][f] / centroid_cnt[ca];
                    dist_a += d * d;
                }
            } else {
                dist_a = numeric_limits<double>::max(); // empty cluster: infinite distance
            }
            if (centroid_cnt[cb] > 0) {
                for (int f = 0; f < num_features; ++f) {
                    double d = features[inst][f] - centroid_sum[cb][f] / centroid_cnt[cb];
                    dist_b += d * d;
                }
            } else {
                dist_b = numeric_limits<double>::max();
            }
            return dist_a < dist_b;
        });

        // LRC = first ceil(alpha * |LC|) elements, at least 1
        int lrc_size = max(1, (int)ceil(alpha * (double)lc.size()));
        lrc_size = min(lrc_size, (int)lc.size());

        // Pick randomly from LRC
        int chosen_c = lc[Random::get<int>(0, lrc_size - 1)];

        // Assign and update running centroid
        sol[inst] = chosen_c;
        for (int f = 0; f < num_features; ++f)
            centroid_sum[chosen_c][f] += features[inst][f];
        centroid_cnt[chosen_c]++;
    }

    return sol;
}