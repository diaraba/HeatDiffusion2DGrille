#include <iostream>
#include <cmath>
#include <cstdlib>
#include "../include/grid.h"
#include "../include/boundary.h"
#include "../include/solver.h"

bool test_grid_allocation() {
    std::cout << "Testing grid allocation..." << std::endl;
    
    // Test 1: Create a small grid
    Grid grid(10, 10);
    if (grid.get_nx() != 10 || grid.get_ny() != 10) {
        std::cerr << "  FAIL: Grid dimensions incorrect" << std::endl;
        return false;
    }
    
    // Test 2: Fill with value and check
    grid.fill(5.0);
    if (std::abs(grid(5, 5) - 5.0) > 1e-10) {
        std::cerr << "  FAIL: Grid fill incorrect" << std::endl;
        return false;
    }
    
    std::cout << "  PASS" << std::endl;
    return true;
}

bool test_boundary_conditions() {
    std::cout << "Testing boundary conditions..." << std::endl;
    
    Grid grid(10, 10);
    grid.fill(10.0);  // Interior starts at 10
    
    // Test Dirichlet (fixed temperature)
    BoundaryConditions dirichlet(0, 0.0, 0.0, 0.0, 0.0);  // All boundaries 0
    dirichlet.apply(grid);
    
    // Check corners
    if (std::abs(grid(0, 0) - 0.0) > 1e-10 ||
        std::abs(grid(9, 9) - 0.0) > 1e-10) {
        std::cerr << "  FAIL: Dirichlet corners incorrect" << std::endl;
        return false;
    }
    
    // Check interior (should still be 10)
    if (std::abs(grid(5, 5) - 10.0) > 1e-10) {
        std::cerr << "  FAIL: Dirichlet affected interior" << std::endl;
        return false;
    }
    
    std::cout << "  PASS" << std::endl;
    return true;
}

bool test_sequential_solver() {
    std::cout << "Testing sequential solver..." << std::endl;
    
    SimulationParams params;
    params.Nx = 32;
    params.Ny = 32;
    params.num_steps = 10;
    params.init_type = 1;  // Gaussian
    
    SequentialSolver solver;
    solver.initialize(params);
    
    // Run solver
    solver.run(params.num_steps);
    
    // Get result
    Grid* result = solver.get_result();
    
    // Basic sanity checks
    if (!result) {
        std::cerr << "  FAIL: No result returned" << std::endl;
        return false;
    }
    
    // Check that temperature is finite
    for (int i = 0; i < result->get_nx(); i++) {
        for (int j = 0; j < result->get_ny(); j++) {
            if (std::isnan((*result)(i, j)) || std::isinf((*result)(i, j))) {
                std::cerr << "  FAIL: Invalid temperature at (" << i << "," << j << ")" << std::endl;
                return false;
            }
        }
    }
    
    std::cout << "  PASS" << std::endl;
    return true;
}

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "Running Sequential Solver Tests" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    bool all_passed = true;
    
    all_passed &= test_grid_allocation();
    all_passed &= test_boundary_conditions();
    all_passed &= test_sequential_solver();
    
    std::cout << "=========================================" << std::endl;
    if (all_passed) {
        std::cout << "ALL TESTS PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "SOME TESTS FAILED!" << std::endl;
        return 1;
    }
}
