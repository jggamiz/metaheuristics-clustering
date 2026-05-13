#include "problemPAR.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <limits>

using namespace std;

// --- Constructor ---
ProblemPAR::ProblemPAR(const string& data_path, const string& const_path, int k) {
    this->num_clusters = k;
    loadData(data_path, const_path);
    this->lambda = calculateLambda();
}

// --- Data Loading ---
void ProblemPAR::loadData(const string& data_path, const string& const_path) {
    // Read features
    ifstream file_dat(data_path);
    if (!file_dat.is_open()) {
        cerr << "Critical Error: Could not open dataset -> " << data_path << endl;
        exit(EXIT_FAILURE);
    }
    
    string line;
    while (getline(file_dat, line)) {
        if (line.empty()) continue; // ignore empty lines 
        
        vector<double> row;
        stringstream ss(line);
        string value;
        
        while (getline(ss, value, ',')) {
            row.push_back(stod(value));
        }
        this->features.push_back(row);
    }
    file_dat.close();

    // Read constraints
    ifstream file_const(const_path);
    if (!file_const.is_open()) {
        cerr << "Critical Error: Could not open constraints -> " << const_path << endl;
        exit(EXIT_FAILURE);
    }
    
    while (getline(file_const, line)) {
        if (line.empty()) continue;
        
        vector<int> row;
        stringstream ss(line);
        string value;
        
        while (getline(ss, value, ',')) {
            row.push_back(stoi(value));
        }
        this->constraints.push_back(row);
    }
    file_const.close();

    int n = features.size();
    ml_constraints.resize(n);
    cl_constraints.resize(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                if (constraints[i][j] == 1)  ml_constraints[i].push_back(j);
                if (constraints[i][j] == -1) cl_constraints[i].push_back(j);
            }
        }
    }
}

double ProblemPAR::calculateEuclideanDistance(const vector<double>& inst1, const vector<double>& inst2) {
    double distance = 0.0;
    for (size_t i=0; i<inst1.size(); i++) {
        double diff = inst1[i]-inst2[i];
        distance += diff*diff;
    }

    return sqrt(distance);
}

float ProblemPAR::calculateLambda() {
    double max_dist = 0.0;
    int num_instances = features.size();
    
    for (int i=0; i<num_instances; i++) {
        for (int j=i+1; j<num_instances; j++) {
            double dist = calculateEuclideanDistance(features[i], features[j]);
            if (dist>max_dist) max_dist = dist;
        }
    }
    
    int R=0;
    for (int i=0; i<num_instances; i++) {
        for (int j=i+1; j<num_instances; j++) {
            if (constraints[i][j]!=0) R++;
        }
    }
    
    if (R==0) return 0.0f;
    return (float)(max_dist/(double)R);
}

vector<vector<double>> ProblemPAR::calculateCentroids(const tSolution<int>& solution) {
    int num_features = features[0].size();
    int num_instances = features.size();
    
    vector<vector<double>> centroids(num_clusters, vector<double>(num_features, 0.0));
    vector<int> count_elements(num_clusters, 0); // for each cluster

    for (int i=0; i<num_instances; i++) {
        int cluster_id = solution[i];
        if (cluster_id >= 0 && cluster_id < num_clusters) {
            for (int j=0; j<num_features; j++) {
                centroids[cluster_id][j] += features[i][j];
            }
            count_elements[cluster_id]++; 
        }
    }

    for (int i=0; i<num_clusters; i++) {
        if (count_elements[i] > 0) {
            for (int j=0; j<num_features; j++) {
                centroids[i][j] /= count_elements[i];
            }
        }
    }

    return centroids;
}

float ProblemPAR::calculateDeviation(const tSolution<int>& solution) {
    vector<vector<double>> centroids = calculateCentroids(solution);
    int num_instances = features.size();
    vector<int> count_elements(num_clusters, 0);
    vector<double> cluster_dev(num_clusters, 0.0);

    for (int i=0; i<num_instances; i++) {
        int cluster_id = solution[i];
        if (cluster_id == -1) continue;
        cluster_dev[cluster_id] += calculateEuclideanDistance(centroids[cluster_id], features[i]);
        count_elements[cluster_id]++;
    }

    double total_dev = 0.0;
    for (int i=0; i<num_clusters; i++) {
        if (count_elements[i] > 0) {
            cluster_dev[i] /= (double)count_elements[i];
        } else {
            cluster_dev[i] = 999999.0; // penalty for empty clusters
        }
        total_dev += cluster_dev[i];
    }

    return (float)(total_dev / num_clusters);
}

int ProblemPAR::calculateInfeasibility(const tSolution<int>& solution) {
    int infeasibility = 0;
    int num_instances = features.size();

    for (int i=0; i<num_instances; i++) {
        if (solution[i] == -1) continue;
        for (int j=i+1; j<num_instances; j++) {
            if (solution[j] == -1) continue;
            int res = constraints[i][j];
            if (res!=0) {
                bool same_cluster = (solution[i] == solution[j]);
                if ((res == 1 && !same_cluster) || (res == -1 && same_cluster)) {
                    infeasibility++;
                }
            }
        }
    }
    return infeasibility;
}

int ProblemPAR::calculateInfeasibilityDelta(const tSolution<int>& sol, int gene, int old_cluster, int new_cluster) const {
    int delta = 0;
    
    for (int j : ml_constraints[gene]) {
        if (old_cluster != sol[j]) delta--; // violación que se elimina
        if (new_cluster != sol[j]) delta++; // violación que se añade
    }

    for (int j : cl_constraints[gene]) {
        if (old_cluster == sol[j]) delta--; // violación que se elimina
        if (new_cluster == sol[j]) delta++; // violación que se añade
    }

    return delta;
}


// --- Overridden Methods from Problem<int> ---

tFitness ProblemPAR::fitness(const tSolution<int>& solution) {
    float deviation = calculateDeviation(solution);
    float infeas = (float)calculateInfeasibility(solution);
    return deviation + (lambda * infeas);
}

tSolution<int> ProblemPAR::createSolution() {
    int size = features.size();
    tSolution<int> random_sol(size);
    
    // Using the random library
    for (int i=0; i<size; i++) {
        random_sol[i] = Random::get(0, num_clusters-1); 
    }
    
    return random_sol;
}

size_t ProblemPAR::getSolutionSize() {
    return features.size();
}

pair<int, int> ProblemPAR::getSolutionDomainRange() {
    return make_pair(0, num_clusters-1);
}

bool ProblemPAR::isValid(const tSolution<int>& solution) {
    // Any integer array within [0, k-1] is mathematically valid
    for (int val : solution) {
        if (val < 0 || val >= num_clusters) return false;
    }

    return true;
}

void ProblemPAR::fix(tSolution<int>& solution) {
    // If a value somehow goes out of bounds, clamp it back
    for (size_t i=0; i<solution.size(); i++) {
        if (solution[i] < 0) solution[i] = 0;
        if (solution[i] >= num_clusters) solution[i] = num_clusters-1;
    }
}