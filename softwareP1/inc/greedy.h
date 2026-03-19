#pragma once

#include "mh.h"
#include "problem.h"

class GreedySearch : public MH<int> {
public:
    GreedySearch() = default;
    ~GreedySearch() override = default;

    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;
};