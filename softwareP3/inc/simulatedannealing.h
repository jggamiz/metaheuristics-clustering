#pragma once

#include "mh.h"

/**
 * Simulated Annealing (Enfriamiento Simulado) for the PAR problem.
 *
 * Cooling scheme : Modified Cauchy  T_{k+1} = T_k / (1 + beta * T_k)
 * Neighbour op   : random (instance, new_cluster) without within-step repetition
 * Inner loop ends: max_vecinos generated  OR  max_exitos accepted
 * Outer loop ends: maxevals reached        OR  n_exitos == 0 in current step
 */
class SimulatedAnnealing : public MH<int> {
public:
    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;
};