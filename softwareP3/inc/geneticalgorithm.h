#pragma once

#include "problemPAR.h"
#include "mh.h"
#include <vector>
#include <random>

using namespace std;

// Enumerados para configurar el algoritmo
enum class GAType {
    AGG, // Generacional
    AGE,       // Estacionario
    AM_ALL,    // Memético: BLS a todos
    AM_RAND,   // Memético: BLS aleatorio (10%)
    AM_BEST    // Memético: BLS a los mejores (10%)
};

enum class CrossoverType {
    UNIFORM,
    FIXED_SEGMENT
};

class GeneticAlgorithm : public MH<int> {
private:
    ProblemPAR& problem; 
    GAType ga_type;
    CrossoverType cross_type;

    int pop_size;
    int max_evals;
    int current_evals;
    int generations;
    double p_cross;
    double p_mut; 

    // Poblaciones
    vector<tSolution<int>> population;
    vector<tSolution<int>> offspring; 

    // Vectores paralelos para guardar el fitness sin recalcularlo
    vector<float> pop_fitness;
    vector<float> off_fitness;

    // Generador de números aleatorios
    mt19937 rng;

    long int current_seed = 0;

    // Métodos internos del flujo del algoritmo
    void initializePopulation();
    void evaluatePopulation(vector<tSolution<int>>& pop, vector<float>& fitnesses);
    
    // Operadores
    void selection(); 
    void crossover();
    void mutate(vector<tSolution<int>>& current_pop);
    void replacement();

    // Métodos específicos
    void uniformCrossover(tSolution<int>& parent1, tSolution<int>& parent2);
    void fixedSegmentCrossover(tSolution<int>& parent1, tSolution<int>& parent2);
    int tournamentSelection();
    void repairSolution(tSolution<int>& sol);
    void applyMemetic();
    void softLocalSearch(int pop_index);

    // Helpers
    int getBestIndividual(const vector<float>& fitnesses);
    int getWorstIndividual(const vector<float>& fitnesses);

public:
    // Constructor
    GeneticAlgorithm(ProblemPAR& p, GAType gType, CrossoverType cType, 
                     int pSize = 50, int mEvals = 100000, 
                     double pC = 0.8, double pM = 0.001);

    tSolution<int> run();

    ResultMH<int> optimize(Problem<int>& problem, int maxevals) override;

    void setSeed(long int s);
};