#include "bmb.h"
#include "localsearch.h"

using namespace std;

ResultMH<int> BMB::optimize(Problem<int>& problem, int maxevals) {
    // Parameters
    const int num_restarts = 5;
    const int evals_per_restart = maxevals / num_restarts; // 100000/5 = 20000

    tSolution<int> best_sol;
    tFitness best_fitness = numeric_limits<tFitness>::max();
    unsigned int total_evals  = 0;

    LocalSearch ls;

    for (int i = 0; i < num_restarts; ++i) {
        // LocalSearch generates its own random initial solution internally
        ResultMH<int> result = ls.optimize(problem, evals_per_restart);
        total_evals += result.evaluations;

        if (result.fitness < best_fitness) {
            best_fitness = result.fitness;
            best_sol = result.solution;
        }
    }

    return ResultMH<int>(best_sol, best_fitness, total_evals);
}