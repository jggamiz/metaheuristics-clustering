#pragma once

#include "mh.h"

/**
 * GRASP (Greedy Randomized Adaptive Search Procedure) for the PAR problem.
 *
 * Each iteration consists of two phases:
 *   1. Greedy Randomized Construction: builds a complete solution element by
 *      element. For each unassigned element the algorithm:
 *        a) Computes the infeasibility increment for every candidate cluster.
 *        b) Keeps only clusters that achieve the minimum increment (LC).
 *        c) Within LC, sorts by distance to the current cluster centroid.
 *        d) Picks a cluster uniformly at random from the top-alpha fraction
 *           of that sorted list (LRC).
 *   2. Local Search (BL from Práctica 1) applied to the constructed solution.
 *
 * Parameters
 * ----------
 *   num_iters        : 5   (matching BMB / ILS budget split)
 *   evals_per_restart: maxevals / num_iters  (= 20 000 with maxevals = 100 000)
 *   alpha            : 0.3  — fraction of LC that forms the LRC
 *                      0 → pure random from LC ; 1 → greedy (always best in LC)
 */
class GRASP : public MH<int> {
public:
    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;

private:
    /** Build one greedy-randomised solution. */
    tSolution<int> constructSolution(Problem<int>& problem, int k, double alpha);
};