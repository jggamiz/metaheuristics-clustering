#pragma once

#include "problem.h" // From common/
#include <vector>
#include <string>
#include <utility>

class ProblemPAR : public Problem<int> {
private:
    // Data attributes
    std::vector<std::vector<double>> features;
    std::vector<std::vector<int>> constraints;
    int num_clusters;
    float lambda;

    // Helper methods
    void loadData(const std::string& data_path, const std::string& const_path);
    float calculateLambda();
    
    std::vector<std::vector<double>> calculateCentroids(const tSolution<int>& solution);
    double calculateEuclideanDistance(const std::vector<double>& inst1, const std::vector<double>& inst2);

public:
    // Constructor
    ProblemPAR(const std::string& data_path, const std::string& const_path, int k);
    
    // Destructor
    ~ProblemPAR() override = default;

    // Getters para que algoritmos específicos puedan leer los datos
    const std::vector<std::vector<double>>& getFeatures() const { return features; }
    const std::vector<std::vector<int>>& getConstraints() const { return constraints; }
    
    float calculateDeviation(const tSolution<int>& solution);
    int calculateInfeasibility(const tSolution<int>& solution);
    
    tFitness fitness(const tSolution<int>& solution) override;
    
    tSolution<int> createSolution() override;
    
    size_t getSolutionSize() override;
    
    std::pair<int, int> getSolutionDomainRange() override;
    
    bool isValid(const tSolution<int>& solution) override;
    
    void fix(tSolution<int>& solution) override;
};