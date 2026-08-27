#include "localsearch.h"
#include "problemPAR.h"
#include "random.hpp"
#include <vector>
#include <algorithm>
#include <numeric>

using namespace std;

//  Original entry point: generates random initial solution, then delegates to the overload
ResultMH<int> LocalSearch::optimize(Problem<int>& problem, int maxevals) {

    ProblemPAR* par = dynamic_cast<ProblemPAR*>(&problem);
    int n = (int)par->getSolutionSize();
    auto bounds = par->getSolutionDomainRange();
    int k = bounds.second + 1;

    tSolution<int> initial_sol = par->createSolution();

    if (!par->isValid(initial_sol))
        par->fix(initial_sol);

    return optimize(problem, maxevals, initial_sol);
}

//  Extended entry point: accepts an external initial solution
ResultMH<int> LocalSearch::optimize(Problem<int>& problem, int maxevals, tSolution<int> current_sol) {

    ProblemPAR* par = dynamic_cast<ProblemPAR*>(&problem);
    int n = (int)par->getSolutionSize();
    auto bounds = par->getSolutionDomainRange();
    int k = bounds.second + 1;

    // Build cluster counts from the provided solution
    vector<int> cluster_counts(k, 0);
    for (int i = 0; i < n; ++i)
        if (current_sol[i] >= 0 && current_sol[i] < k)
            cluster_counts[current_sol[i]]++;

    float current_fitness = problem.fitness(current_sol);
    unsigned int evaluations = 1;
    bool improvement = true;

    while (improvement && evaluations < (unsigned int)maxevals) {
        improvement = false;

        // Build and shuffle the full neighbourhood
        vector<pair<int,int>> neighborhood;
        neighborhood.reserve(n * (k - 1));
        for (int i = 0; i < n; ++i)
            for (int c = 0; c < k; ++c)
                if (current_sol[i] != c)
                    neighborhood.push_back({i, c});

        Random::shuffle(neighborhood.begin(), neighborhood.end());

        // First-improvement exploration
        for (const auto& [inst, new_cluster] : neighborhood) {
            if (evaluations >= (unsigned int)maxevals) break;

            int old_cluster = current_sol[inst];

            // Validating we don't leave any cluster empty
            if (cluster_counts[old_cluster] <= 1) continue;

            tSolution<int> neighbor = current_sol;
            neighbor[inst] = new_cluster;

            // Explicit validity check (bounds + no empty clusters)
            if (!par->isValid(neighbor)) continue;

            float neighbor_fitness = problem.fitness(neighbor);
            ++evaluations;

            if (neighbor_fitness < current_fitness) {
                current_sol = neighbor;
                current_fitness = neighbor_fitness;
                cluster_counts[old_cluster]--;
                cluster_counts[new_cluster]++;
                improvement = true;
                break;
            }
        }
    }

    return ResultMH<int>(current_sol, current_fitness, evaluations);
}