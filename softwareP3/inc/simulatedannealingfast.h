#pragma once

#include "mh.h"

/**
 * SimulatedAnnealingFast — voluntary variant of the ES algorithm.
 *
 * Identical to SimulatedAnnealing in every algorithmic aspect, but uses
 * incremental infeasibility evaluation (calculateInfeasibilityDelta) to
 * avoid recomputing the full O(n²) constraint scan on every neighbour.
 *
 * The deviation component still requires a full centroid recalculation
 * when a move is accepted; for rejected moves, however, we only pay the
 * cheap delta cost, giving a significant speedup in practice when
 * infeasibility dominates (early annealing phases with many violations).
 *
 * This makes the algorithm explore a larger fraction of the neighbourhood
 * within the same evaluation budget, yielding better solutions on average.
 */
class SimulatedAnnealingFast : public MH<int> {
public:
    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;
};