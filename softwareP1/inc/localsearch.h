#pragma once

#include "mh.h"
#include "problem.h"

class LocalSearch : public MH<int> {
public:
    LocalSearch() = default;
    ~LocalSearch() override = default;

    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;
};