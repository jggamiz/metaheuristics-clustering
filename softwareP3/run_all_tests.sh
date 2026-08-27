#!/bin/bash

# Limpiar resultados previos (opcional)
# rm -f det_results.csv avg_results.csv

echo "Starting the complete test battery..."

# BUPA
./bin/par_solver data/bupa_set 15 all
./bin/par_solver data/bupa_set 30 all

# GLASS
./bin/par_solver data/glass_set 15 all
./bin/par_solver data/glass_set 30 all

# ZOO
./bin/par_solver data/zoo_set 15 all
./bin/par_solver data/zoo_set 30 all

echo "All tests finished! CSV files generated."