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
        cerr << "Algorithms: random, greedy, local, agg-un, agg-sf, age-un, age-sf, am-all, am-rand, am-best, all" << endl;
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

    // Instancias de los algoritmos
    RandomSearch ralg = RandomSearch();
    GreedySearch rgreedy = GreedySearch();
    LocalSearch rlocal = LocalSearch();

    // Genéticos y Meméticos
    GeneticAlgorithm r_agg_un(rproblem, GAType::AGG, CrossoverType::UNIFORM);
    GeneticAlgorithm r_agg_sf(rproblem, GAType::AGG, CrossoverType::FIXED_SEGMENT);
    GeneticAlgorithm r_age_un(rproblem, GAType::AGE, CrossoverType::UNIFORM);
    GeneticAlgorithm r_age_sf(rproblem, GAType::AGE, CrossoverType::FIXED_SEGMENT);
    GeneticAlgorithm r_am_all(rproblem, GAType::AM_ALL, CrossoverType::UNIFORM);
    GeneticAlgorithm r_am_rand(rproblem, GAType::AM_RAND, CrossoverType::UNIFORM);
    GeneticAlgorithm r_am_best(rproblem, GAType::AM_BEST, CrossoverType::UNIFORM);


    cout << "Lambda: " << rproblem.getLambda() << endl;
    // cout << "Num instances: " << rproblem.getFeatures().size() << endl;

    // Seleccionar algoritmos
    vector<pair<string, MH<int>*>> algoritmos;
    if (alg_choice == "random" || alg_choice == "all") algoritmos.push_back({"RandomSearch", &ralg});
    if (alg_choice == "greedy" || alg_choice == "all") algoritmos.push_back({"GreedySearch", &rgreedy});
    if (alg_choice == "local" || alg_choice == "all")  algoritmos.push_back({"LocalSearch", &rlocal});
    
    if (alg_choice == "agg-un" || alg_choice == "all") algoritmos.push_back({"AGG-Uniform", &r_agg_un});
    if (alg_choice == "agg-sf" || alg_choice == "all") algoritmos.push_back({"AGG-Segment", &r_agg_sf});
    if (alg_choice == "age-un" || alg_choice == "all") algoritmos.push_back({"AGE-Uniform", &r_age_un});
    if (alg_choice == "age-sf" || alg_choice == "all") algoritmos.push_back({"AGE-Segment", &r_age_sf});
    
    if (alg_choice == "am-all" || alg_choice == "all") algoritmos.push_back({"AM-All", &r_am_all});
    if (alg_choice == "am-rand"|| alg_choice == "all") algoritmos.push_back({"AM-Rand", &r_am_rand});
    if (alg_choice == "am-best"|| alg_choice == "all") algoritmos.push_back({"AM-Best", &r_am_best});

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
            Random::seed(seed);
            
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