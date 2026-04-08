#!/bin/bash
# Script to run OpenMP version with different thread counts

cd ..
mkdir -p build
cd build
cmake ..
make heat_omp -j4

# Test with different numbers of threads
for threads in 1 2 4 8 16; do
    echo "========================================"
    echo "Running with $threads threads"
    echo "========================================"
    export OMP_NUM_THREADS=$threads
    ./bin/heat_omp --nx 4096 --ny 4096 --steps 100 --init 1 --boundary 0
done
