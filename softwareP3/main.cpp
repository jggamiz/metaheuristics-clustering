#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>

#include "problem.h"
#include "random.hpp"
#include "mh.h"
#include "problemPAR.h"
#include "randomsearch.h"
#include "greedy.h"
#include "localsearch.h"
#include "geneticalgorithm.h"

// --- Práctica 3: algoritmos de trayectoria ---
#include "simulatedannealing.h"
#include "simulatedannealingfast.h"   // variante voluntaria
#include "bmb.h"
#include "ils.h"
#include "ils_es.h"
#include "grasp.h"

using namespace std;

// Función auxiliar para comprobar si un archivo ya existe
bool fileExists(const string& filename) {
    ifstream f(filename.c_str());
    return f.good();
}

void print_result(ResultMH<int>& result, long int seed, float deviation, int infeasibility, double time_sec) {
    cout << "----------------------------------------" << endl;
    cout << "Using seed: " << seed << endl;
    cout << "Best fitness:   " << result.fitness << endl;
    cout << "Deviation:      " << deviation << endl;
    cout << "Infeasibility:  " << infeasibility << endl;
    cout << "Evaluations:    " << result.evaluations << endl; 
    cout << "Time (s): " << time_sec << endl;
}

int main(int argc, char *argv[]) {
    string dataset_base;
    int k_clusters;
    string alg_choice;
    long int seed;
    vector<long int> seeds;
    const int NUM_ITS = 10;
    
    if (argc<4 || argc>5) {
        cerr << "Usage: " << argv[0] << " <dataset_base_path> <k_clusters> <algorithm> [seed]" << endl;
        cerr << "Algorithms: random, greedy, local, am-best, es, es-fast, bmb, ils, ils-es, grasp, all" << endl;
        cerr << "Example: " << argv[0] << " data/glass_set 15 all 42" << endl;
        return 1;
    } 
    
    dataset_base = argv[1];
    k_clusters = atoi(argv[2]);
    alg_choice = argv[3];

    string file_dat = dataset_base + ".dat";
    string file_const = dataset_base + "_const_" + to_string(k_clusters) + ".dat";

    if (argc == 4) {
        cout << "Running 10 iterations with different seeds..." << endl;
        for (int i=0; i<NUM_ITS; i++) {
            seeds.push_back(1000 + i);
        }
    } else {
        seeds = {atol(argv[4])};
        cout << "Using custom seed: " << seeds[0] << endl;
    }

    string csv_detailed = "det_results.csv";
    string csv_avg = "avg_results.csv";
    
    bool det_exists = fileExists(csv_detailed);
    bool avg_exists = fileExists(csv_avg);

    ofstream file_det(csv_detailed, ios::app);
    ofstream file_avg(csv_avg, ios::app);

    if (!det_exists) {
        file_det << "Dataset,K,Algorithm,Seed,Fitness,Distance,Infeasibility,Evaluations,Time\n";
    }
    if (!avg_exists) {
        file_avg << "Dataset,K,Algorithm,AvgFitness,AvgDistance,AvgInfeasibility,AvgEvaluations,AvgTime\n";
    }
    
    // Instanciar el problema
    cout << "Loading dataset: " << dataset_base << " with k=" << k_clusters << endl;
    ProblemPAR rproblem(file_dat, file_const, k_clusters);
    Problem<int> *problem = &rproblem;

    // Instancias de los algoritmos clásicos
    RandomSearch ralg = RandomSearch();
    GreedySearch rgreedy = GreedySearch();
    LocalSearch rlocal = LocalSearch();
    GeneticAlgorithm r_am_best(rproblem, GAType::AM_BEST, CrossoverType::UNIFORM);

    // Algoritmos de Trayectoria (Práctica 3)
    SimulatedAnnealing r_es = SimulatedAnnealing();
    SimulatedAnnealingFast r_es_fast = SimulatedAnnealingFast();
    BMB r_bmb = BMB();
    ILS r_ils = ILS();
    ILS_ES r_ils_es = ILS_ES();
    GRASP r_grasp = GRASP();

    cout << "Lambda: " << rproblem.getLambda() << endl;

    // Seleccionar algoritmos
    vector<pair<string, MH<int>*>> algoritmos;
    if (alg_choice == "random" || alg_choice == "all") algoritmos.push_back({"RandomSearch", &ralg});
    if (alg_choice == "greedy" || alg_choice == "all") algoritmos.push_back({"GreedySearch", &rgreedy});
    if (alg_choice == "local"  || alg_choice == "all") algoritmos.push_back({"LocalSearch", &rlocal});
    if (alg_choice == "am-best"|| alg_choice == "all") algoritmos.push_back({"AM-Best", &r_am_best});

    // Algoritmos de trayectoria
    if (alg_choice == "es" || alg_choice == "all") algoritmos.push_back({"SimulatedAnnealing", &r_es});
    if (alg_choice == "es-fast" || alg_choice == "all") algoritmos.push_back({"SimulatedAnnealingFast", &r_es_fast});
    if (alg_choice == "bmb"    || alg_choice == "all") algoritmos.push_back({"BMB", &r_bmb});
    if (alg_choice == "ils"    || alg_choice == "all") algoritmos.push_back({"ILS", &r_ils});
    if (alg_choice == "ils-es" || alg_choice == "all") algoritmos.push_back({"ILS-ES", &r_ils_es});
    if (alg_choice == "grasp"  || alg_choice == "all") algoritmos.push_back({"GRASP", &r_grasp});

    if (algoritmos.empty()) {
        cerr << "Error: Unknown algorithm '" << alg_choice << "'." << endl;
        return 1;
    }

    // Bucle principal
    for (size_t i=0; i<algoritmos.size(); i++) {
        string name = algoritmos[i].first;
        MH<int> *mh = algoritmos[i].second;
        
        int maxevals = (name != "GreedySearch") ? 100000 : 1;

        double sum_fitness = 0.0, sum_deviation = 0.0, sum_infeasibility = 0.0, sum_evals = 0.0, sum_time = 0.0;

        cout << "\n========================================" << endl;
        cout << " ALGORITHM: " << name << endl;
        cout << "========================================" << endl;

        for (size_t j=0; j<seeds.size(); j++) {
            seed = seeds[j];
            Random::seed(seed); // Semilla global para métodos que usen Random::get
            
            // Si el algoritmo es el Genético, usamos su propio inicializador de semilla
            GeneticAlgorithm* ga_ptr = dynamic_cast<GeneticAlgorithm*>(mh);
            if (ga_ptr != nullptr) {
                ga_ptr->setSeed(seed);
            }

            auto start = chrono::high_resolution_clock::now();
            ResultMH<int> result = mh->optimize(*problem, maxevals);
            auto end = chrono::high_resolution_clock::now();
            
            chrono::duration<double> duration = end - start;
            double time_sec = duration.count();

            float dev = rproblem.calculateDeviation(result.solution);
            int inf = rproblem.calculateInfeasibility(result.solution);

            if (seeds.size() == 1) {
                print_result(result, seed, dev, inf, time_sec);
            }

            file_det << dataset_base << "," << k_clusters << "," << name << "," << seed << ","
                     << result.fitness << "," << dev << "," << inf << "," 
                     << result.evaluations << "," << time_sec << "\n";

            sum_fitness += result.fitness;
            sum_deviation += dev;
            sum_infeasibility += inf;
            sum_evals += result.evaluations;
            sum_time += time_sec;
        }

        // Mostrar media por consola
        cout << "\n----------------------------------------" << endl;
        cout << " AVERAGE RESULTS (over " << seeds.size() << " runs)" << endl;
        cout << "----------------------------------------" << endl;
        cout << "Algorithm:     " << name << endl;
        cout << "Fitness:       " << sum_fitness / seeds.size() << endl;
        cout << "Distance:      " << sum_deviation / seeds.size() << endl;
        cout << "Infeasibility: " << sum_infeasibility / seeds.size() << endl;
        cout << "Evaluations:   " << sum_evals / seeds.size() << endl;
        cout << "Time (sec):    " << sum_time / seeds.size() << endl << endl;

        // Escribir fila media
        if (seeds.size() > 1) {
            file_avg << dataset_base << "," << k_clusters << "," << name << ","
                     << sum_fitness / seeds.size() << ","
                     << sum_deviation / seeds.size() << ","
                     << sum_infeasibility / seeds.size() << ","
                     << sum_evals / seeds.size() << ","
                     << sum_time / seeds.size() << "\n";
        }
    }

    file_det.close();
    file_avg.close();

    return 0;
}