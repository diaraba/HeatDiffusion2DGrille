#!/bin/bash
# Simple script to run sequential version

# Compile if needed
cd ..
mkdir -p build
cd build
cmake ..
make heat_seq -j4

# Run with different grid sizes
echo "Running sequential solver..."
./bin/heat_seq --nx 512 --ny 512 --steps 100 --init 1 --boundary 0
