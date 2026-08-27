#pragma once

#include "mh.h"

/**
 * ILS-ES: Hybridisation of Iterated Local Search and Simulated Annealing.
 *
 * Same outer structure as ILS:
 *   - 5 iterations total  (1 initial ES + 4 over mutated best solution)
 *   - Each ES capped at maxevals/5 evaluations
 *   - Same 20 % mutation operator as ILS (reused via ILS::mutate)
 *   - Best-so-far acceptance criterion
 *
 * The inner optimiser is Simulated Annealing (ES) instead of BL.
 * ES parameters are identical to the standalone ES, but the initial
 * solution is supplied externally rather than generated internally.
 */
class ILS_ES : public MH<int> {
public:
    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;

private:
    /** Run one ES instance starting from a given solution. */
    ResultMH<int> runES(Problem<int>& problem,
                        tSolution<int> initial_sol,
                        int maxevals);
};