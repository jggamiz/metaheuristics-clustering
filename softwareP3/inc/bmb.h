#pragma once

#include "mh.h"

/**
 * Búsqueda Multiarranque Básica (BMB)
 *
 * Runs LocalSearch from 5 independent random starting solutions.
 * Each LocalSearch is capped at 20000 evaluations.
 * Returns the best solution found across all restarts.
 */
class BMB : public MH<int> {
public:
    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;
};