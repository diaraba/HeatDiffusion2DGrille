#!/bin/bash
#cd ~/hpc_project

echo "Checking for required files..."

# Check header files
for header in include/grid.h include/boundary.h include/timer.h include/config.h include/solver.h; do
    if [ -f "$header" ]; then
        echo "✓ $header"
    else
        echo "✗ $header - MISSING"
    fi
done

# Check source files
for src in src/grid.cpp src/boundary.cpp src/timer.cpp src/config.cpp src/solver.cpp \
           src/main.cpp src/solver_sequential.cpp src/solver_openmp.cpp \
           src/solver_mpi.cpp src/solver_hybrid.cpp; do
    if [ -f "$src" ]; then
        echo "✓ $src"
    else
        echo "✗ $src - MISSING"
    fi
done

# Check test files
for test in tests/test_sequential.cpp tests/test_correctness.py; do
    if [ -f "$test" ]; then
        echo "✓ $test"
    else
        echo "✗ $test - MISSING"
    fi
done

echo ""
echo "CMakeLists.txt: $(if [ -f CMakeLists.txt ]; then echo "✓"; else echo "✗"; fi)"
