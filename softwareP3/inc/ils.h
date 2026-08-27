#pragma once

#include "mh.h"
#include "problem.h"

/**
 * Iterated Local Search (Búsqueda Local Reiterada - ILS)
 *
 * Schema  : simplified version — always mutates the current best solution.
 * Iters   : 5 total  (1 initial BL + 4 over mutated solutions)
 * BL cap  : maxevals/5 evaluations per call
 * Mutation: 20 % of genes reassigned to a different cluster (uniformly random)
 * Accept  : best-so-far criterion (mutation is always applied to best_sol)
 */
class ILS : public MH<int> {
public:
    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;

    /** Mutate 20 % of the solution's genes to a different (random) cluster.
     *  Made static/public so ILS-ES can reuse it without duplication. */
    static tSolution<int> mutate(const tSolution<int>& sol, int k);
};