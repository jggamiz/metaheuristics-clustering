#include "ils.h"
#include "localsearch.h"
#include "problemPAR.h"
#include "random.hpp"
#include <vector>
#include <numeric>
#include <algorithm>

using namespace std;

// Public entry point
ResultMH<int> ILS::optimize(Problem<int>& problem, int maxevals) {

    ProblemPAR* par_problem = dynamic_cast<ProblemPAR*>(&problem);
    int n = (int)par_problem->getSolutionSize();
    auto bounds = par_problem->getSolutionDomainRange();
    int k = bounds.second + 1;

    const int num_iters = 5;
    const int evals_per_restart = maxevals / num_iters; // 20000

    unsigned int total_evals = 0;
    LocalSearch ls;

    // Random initial solution (no empty clusters)
    tSolution<int> initial_sol(n, -1);
    {
        vector<int> available(n);
        iota(available.begin(), available.end(), 0);
        Random::shuffle(available.begin(), available.end());
        for (int c = 0; c < k; ++c) initial_sol[available[c]] = c;
        for (int i = k; i < n; ++i)
            initial_sol[available[i]] = Random::get<int>(0, k - 1);
    }

    // First BL
    ResultMH<int> result = ls.optimize(problem, evals_per_restart, initial_sol);
    total_evals += result.evaluations;

    tSolution<int> best_sol = result.solution;
    tFitness best_fitness = result.fitness;

    // Mutate best -> BL -> update best  (4 more times)
    for (int iter = 1; iter < num_iters; ++iter) {
        tSolution<int> mutated = mutate(best_sol, k);
        result = ls.optimize(problem, evals_per_restart, mutated);
        total_evals += result.evaluations;

        if (result.fitness < best_fitness) {
            best_fitness = result.fitness;
            best_sol = result.solution;
        }
    }

    return ResultMH<int>(best_sol, best_fitness, total_evals);
}


// Static mutation operator
// Change 20 % of genes to a different cluster (uniform random, no repetition of selected indices)
tSolution<int> ILS::mutate(const tSolution<int>& sol, int k) {
    int n = (int)sol.size();
    int num_to_change = max(1, (int)(0.2 * n));

    tSolution<int> mutated = sol;

    vector<int> indices(n);
    iota(indices.begin(), indices.end(), 0);
    Random::shuffle(indices.begin(), indices.end());

    for (int i = 0; i < num_to_change; ++i) {
        int idx = indices[i];
        int old_c = mutated[idx];
        int new_c;
        do { new_c = Random::get<int>(0, k - 1); } while (new_c == old_c);
        mutated[idx] = new_c;
    }

    return mutated;
}