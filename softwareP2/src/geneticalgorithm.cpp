#include "../inc/geneticalgorithm.h"
#include <algorithm>
#include <iostream>
#include <numeric> // Para iota

using namespace std;

// Constructor
GeneticAlgorithm::GeneticAlgorithm(ProblemPAR& p, GAType gType, CrossoverType cType, 
                                   int pSize, int mEvals, double pC, double pM)
    : problem(p), ga_type(gType), cross_type(cType), 
      pop_size(pSize), max_evals(mEvals), current_evals(0), generations(0), 
      p_cross(pC), p_mut(pM) {
}

// Bucle principal
tSolution<int> GeneticAlgorithm::run() {
    current_evals = 0;
    generations = 0;
    
    initializePopulation();
    evaluatePopulation(population, pop_fitness);

    while (current_evals < max_evals) {
        selection();
        crossover();
        mutate(offspring);

        // Evaluar hijos
        evaluatePopulation(offspring, off_fitness);
        replacement();
        
        generations++;

        // Hibridación Memética cada 10 generaciones
        if (generations % 10 == 0) {
            applyMemetic();
        }
    }

    int best_idx = getBestIndividual(pop_fitness);
    return population[best_idx];
}

ResultMH<int> GeneticAlgorithm::optimize(Problem<int>& p, int maxevals) {
    ProblemPAR* par_prob = dynamic_cast<ProblemPAR*>(&p);
    max_evals = maxevals;
    rng.seed(current_seed);
    tSolution<int> best_sol = run();
    float best_fit = par_prob->fitness(best_sol);
    return ResultMH<int>(best_sol, best_fit, current_evals);
}

void GeneticAlgorithm::setSeed(long int s) {
    rng.seed(s);
    current_seed = s;
}

// Inicialización aleatoria
void GeneticAlgorithm::initializePopulation() {
    population.clear();
    population.reserve(pop_size);
    for (int i=0; i<pop_size; ++i) {
        tSolution<int> initial_sol = problem.createSolution();
        repairSolution(initial_sol);    // Para asegurarnos que no tenemos clusters vacíos
        population.push_back(initial_sol);
    }
}

// Evaluación
void GeneticAlgorithm::evaluatePopulation(vector<tSolution<int>>& pop, vector<float>& fitnesses) {
    fitnesses.resize(pop.size());
    for (size_t i=0; i<pop.size(); ++i) {
        if (current_evals < max_evals) {
            fitnesses[i] = problem.fitness(pop[i]);
            current_evals++;
        }
    }
}

// Torneo con k = 3
int GeneticAlgorithm::tournamentSelection() {
    uniform_int_distribution<int> dist(0, pop_size - 1);
    int best_idx = dist(rng);
    
    // Competimos contra otros dos
    for (int i=1; i<3; ++i) {
        int contestant = dist(rng);
        if (pop_fitness[contestant] < pop_fitness[best_idx]) best_idx = contestant;
    }
    return best_idx;
}

// Selección
void GeneticAlgorithm::selection() {
    offspring.clear();
    
    // Tanto en AGG como en AMs el esquema base es generacional
    if (ga_type == GAType::AGG || ga_type == GAType::AM_ALL || 
        ga_type == GAType::AM_RAND || ga_type == GAType::AM_BEST) {
        offspring.reserve(pop_size);
        for (int i = 0; i < pop_size; ++i) {
            offspring.push_back(population[tournamentSelection()]);
        }
    } else { // AGE
        offspring.reserve(2);
        offspring.push_back(population[tournamentSelection()]);
        offspring.push_back(population[tournamentSelection()]);
    }
}

// Cruce
void GeneticAlgorithm::crossover() {
    uniform_real_distribution<double> prob_dist(0.0, 1.0);
    
    // Emparejamiento fijo: (0 con 1), (2 con 3)...
    for (size_t i=0; i+1<offspring.size(); i += 2) {
        if (prob_dist(rng) <= p_cross) {
            if (cross_type == CrossoverType::UNIFORM) uniformCrossover(offspring[i], offspring[i+1]);
            else fixedSegmentCrossover(offspring[i], offspring[i+1]);   
        }
    }
}

void GeneticAlgorithm::uniformCrossover(tSolution<int>& parent1, tSolution<int>& parent2) {
    int n = problem.getSolutionSize();
    vector<int> indices(n);
    iota(indices.begin(), indices.end(), 0);
    shuffle(indices.begin(), indices.end(), rng);

    tSolution<int> o1 = parent1;
    tSolution<int> o2 = parent2;

    int half = n / 2;

    // La primera mitad de los índices (o1 se queda con P1, o2 con P2)    
    // En la segunda mitad, intercambiamos (o1 recibe de P2, o2 recibe de P1)
    for (int i=half; i<n; ++i) {
        int idx = indices[i];
        o1[idx] = parent2[idx];
        o2[idx] = parent1[idx];
    }

    parent1 = o1;
    parent2 = o2;
    
    // Aseguramos que no se nos quedan clusters vacíos
    repairSolution(parent1);
    repairSolution(parent2);
}

void GeneticAlgorithm::fixedSegmentCrossover(tSolution<int>& parent1, tSolution<int>& parent2) {
    int n = problem.getSolutionSize();
    uniform_int_distribution<int> pos_dist(0, n - 1);
    uniform_int_distribution<int> fn_dist(1, max(1, n / 3)); 
    uniform_real_distribution<double> prob(0.0, 1.0);

    int pos = pos_dist(rng);
    int fn = fn_dist(rng);
    int final_pos = pos + fn;

    tSolution<int> o1 = parent1;
    tSolution<int> o2 = parent2;

    for (int i=0; i<n; ++i) {
        bool in_segment = false;
        if (final_pos < n) {
            if (i >= pos && i <= final_pos) in_segment = true;
        } else {
            int final_rem = final_pos % n;
            if (i >= pos || i <= final_rem) in_segment = true;
        }

        if (in_segment) {
            // El segmento siempre se intercambia
            o1[i] = parent2[i];
            o2[i] = parent1[i];
        } else {
            // Fuera del segmento: intercambio aleatorio 50%
            if (prob(rng) > 0.5) {
                o1[i] = parent2[i];
                o2[i] = parent1[i];
            }
        }
    }

    parent1 = o1;
    parent2 = o2;

    // Aseguramos que no se nos quedan clusters vacíos
    repairSolution(parent1);
    repairSolution(parent2);
}

// Mutación
void GeneticAlgorithm::mutate(vector<tSolution<int>>& current_pop) {
    int n = problem.getSolutionSize();
    int k = problem.getSolutionDomainRange().second + 1;
    uniform_real_distribution<double> dist_prob(0.0, 1.0);

    for (auto& sol : current_pop) {
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            if (dist_prob(rng) < p_mut) {
                int original_cluster = sol[i];

                // Buscamos el candidato con menor delta (que no empeore infeasibility)
                int best_delta = 999999;
                int best_cluster = -1;
                for (int c = 0; c < k; ++c) {
                    if (c == original_cluster) continue;
                    int delta = problem.calculateInfeasibilityDelta(sol, i, original_cluster, c);
                    if (delta < best_delta) {
                        best_delta = delta;
                        best_cluster = c;
                    }
                }

                if (best_cluster != -1 && best_delta <= 0) {
                    sol[i] = best_cluster;
                    changed = true;
                }
            }
        }

        // Aseguramos que no se nos quedan clusters vacíos
        if (changed) repairSolution(sol);
    }
}

// Reemplazo
void GeneticAlgorithm::replacement() {
    // Para todos los que derivan de Generacional (AGG y Meméticos)
    if (ga_type != GAType::AGE) {
        int best_parent_idx = getBestIndividual(pop_fitness);
        int best_off_idx = getBestIndividual(off_fitness);
        
        if (off_fitness[best_off_idx] > pop_fitness[best_parent_idx]) {
            int worst_off_idx = getWorstIndividual(off_fitness);
            offspring[worst_off_idx] = population[best_parent_idx];
            off_fitness[worst_off_idx] = pop_fitness[best_parent_idx];
        }
        
        // Sustituir toda la población
        population = move(offspring);
        pop_fitness = move(off_fitness);
        
    } else { // AGE
        int worst1 = getWorstIndividual(pop_fitness);
        if (off_fitness[0] < pop_fitness[worst1]) {
            population[worst1] = offspring[0];
            pop_fitness[worst1] = off_fitness[0];
        }
        
        int worst2 = getWorstIndividual(pop_fitness);
        if (off_fitness[1] < pop_fitness[worst2]) {
            population[worst2] = offspring[1];
            pop_fitness[worst2] = off_fitness[1];
        }
    }
}


// --- LOGICA DE MEMÉTICOS ---

void GeneticAlgorithm::applyMemetic() {
    if (ga_type == GAType::AGG || ga_type == GAType::AGE) return;

    if (ga_type == GAType::AM_ALL) {
        for (size_t i = 0; i < population.size(); ++i) {
            if (current_evals >= max_evals) break;
            softLocalSearch(i);
        }
    } 
    else if (ga_type == GAType::AM_RAND) {
        uniform_real_distribution<double> prob(0.0, 1.0);
        for (size_t i=0; i<population.size(); ++i) {
            if (current_evals >= max_evals) break;
            if (prob(rng) <= 0.1) softLocalSearch(i);
        }
    } 
    else if (ga_type == GAType::AM_BEST) {
        int num_best = max(1, (int)(0.1 * pop_size));
        vector<int> indices(pop_size);
        iota(indices.begin(), indices.end(), 0);
        
        // Ordenar índices para que los primeros apunten a los mejores fitness (menor es mejor)
        sort(indices.begin(), indices.end(), [&](int a, int b) {
            return pop_fitness[a] < pop_fitness[b];
        });

        for (int j=0; j<num_best; ++j) {
            if (current_evals >= max_evals) break;
            softLocalSearch(indices[j]);
        }
    }
}

void GeneticAlgorithm::softLocalSearch(int pop_index) {
    int n = problem.getSolutionSize();
    int k = problem.getSolutionDomainRange().second + 1; 
    int max_fallos = max(1, (int)(0.1 * pop_size));
    int max_evals_bl = 100;
    int fallos = 0;
    int evals_bl = 0;
    int i = 0;

    tSolution<int> S = population[pop_index];
    float bestfit = pop_fitness[pop_index];

    vector<int> RSI(n);
    iota(RSI.begin(), RSI.end(), 0);
    shuffle(RSI.begin(), RSI.end(), rng);

    tSolution<int> newsol = S;

    while (fallos < max_fallos && i < n && evals_bl < max_evals_bl && current_evals < max_evals) {
        int p = RSI[i];
        int oldvalue = S[p];

        for (int val = 0; val < k; ++val) { 
            if (val == oldvalue) continue;
            if (evals_bl >= max_evals_bl || current_evals >= max_evals) break;

            newsol[p] = val;
            float fit = problem.fitness(newsol);
            evals_bl++;
            current_evals++;

            if (fit < bestfit) {
                S = newsol;
                bestfit = fit;
            } else {
                newsol[p] = S[p];
            }
        }

        if (oldvalue == S[p]) fallos++;
        i++;
    }

    population[pop_index] = S;
    pop_fitness[pop_index] = bestfit;
}

// Helpers
int GeneticAlgorithm::getBestIndividual(const vector<float>& fitnesses) {
    int best_idx = 0;
    for (size_t i=1; i<fitnesses.size(); ++i) {
        if (fitnesses[i] < fitnesses[best_idx]) best_idx = i;
    }
    return best_idx;
}

int GeneticAlgorithm::getWorstIndividual(const vector<float>& fitnesses) {
    int worst_idx = 0;
    for (size_t i=1; i<fitnesses.size(); ++i) {
        if (fitnesses[i] > fitnesses[worst_idx]) worst_idx = i;
    }
    return worst_idx;
}

// Reoaración de soluciones
void GeneticAlgorithm::repairSolution(tSolution<int>& sol) {
    int k = problem.getSolutionDomainRange().second + 1;
    int n = problem.getSolutionSize();
    vector<int> counts(k, 0);
    for (int i = 0; i < n; ++i) counts[sol[i]]++;

    for (int c = 0; c < k; ++c) {
        if (counts[c] == 0) {
            vector<int> robable;
            for (int i = 0; i < n; ++i)
                if (counts[sol[i]] > 1) robable.push_back(i);
            shuffle(robable.begin(), robable.end(), rng);

            bool repaired = false;
            for (int inst : robable) {
                int old_cluster = sol[inst];
                int delta = problem.calculateInfeasibilityDelta(sol, inst, old_cluster, c);
                if (delta <= 0) {
                    counts[old_cluster]--;
                    sol[inst] = c;
                    counts[c]++;
                    repaired = true;
                    break;
                }
            }

            // Fallback
            if (!repaired && !robable.empty()) {
                int inst = robable[0];
                counts[sol[inst]]--;
                sol[inst] = c;
                counts[c]++;
            }
        }
    }
}