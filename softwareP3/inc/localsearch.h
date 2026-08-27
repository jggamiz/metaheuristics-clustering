#pragma once

#include "mh.h"
#include "problem.h"

/**
 * Two entry points:
 *   optimize(problem, maxevals): generates a random initial solution internally
 *   optimize(problem, maxevals, initial_sol): starts from a given solution (thought for ILS in P3)
 */
class LocalSearch : public MH<int> {
public:
    // Standard entry point: random initial solution
    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;

    // Extended entry point: caller supplies the initial solution
    ResultMH<int> optimize(Problem<int>& problem, int maxevals,
                           tSolution<int> initial_sol);
};