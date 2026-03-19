#pragma once

#include "problem.h" // From common/
#include <vector>
#include <string>
#include <utility>

using namespace std;

class ProblemPAR : public Problem<int> {
private:
    // Data attributes
    vector<vector<double>> features;
    vector<vector<int>> constraints;
    int num_clusters;
    float lambda;

    // Helper methods
    void loadData(const string& data_path, const string& const_path);
    float calculateLambda();
    
    vector<vector<double>> calculateCentroids(const tSolution<int>& solution);
    double calculateEuclideanDistance(const vector<double>& inst1, const vector<double>& inst2);

public:
    // Constructor
    ProblemPAR(const string& data_path, const string& const_path, int k);
    
    // Destructor
    ~ProblemPAR() override = default;

    // Getters para que se puedan leer los datos
    const vector<vector<double>>& getFeatures() const { return features; }
    const vector<vector<int>>& getConstraints() const { return constraints; }
    
    float calculateDeviation(const tSolution<int>& solution);
    int calculateInfeasibility(const tSolution<int>& solution);
    
    tFitness fitness(const tSolution<int>& solution) override;
    
    tSolution<int> createSolution() override;
    
    size_t getSolutionSize() override;
    
    pair<int, int> getSolutionDomainRange() override;
    
    bool isValid(const tSolution<int>& solution) override;
    
    void fix(tSolution<int>& solution) override;
};