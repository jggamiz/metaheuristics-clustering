#pragma once

#include "mh.h"
#include "problem.h"

class RandomSearch : public MH<int> {
public:
    RandomSearch() = default;
    ~RandomSearch() override = default;

    // Implement the MH interface methods
    /**
    * Create random solutions until maxevals has been achieved, and returns the best one.
    *
    * @param problem The problem to be optimized
    * @param maxevals Maximum number of evaluations allowed
    * @return A pair containing the best solution found and its fitness
    */
    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;
};