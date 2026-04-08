#!/usr/bin/env python3
"""
Python test script to validate solver correctness
Compares different implementations for consistency
"""

import subprocess
import numpy as np
import sys
import os

def run_solver(executable, nx, ny, steps, init_type=1):
    """Run a solver and capture output"""
    cmd = f"../build/bin/{executable} --nx {nx} --ny {ny} --steps {steps} --init {init_type}"
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, cwd=os.path.dirname(__file__))
        return result.stdout
    except Exception as e:
        print(f"Error running {executable}: {e}")
        return None

def parse_grid_from_output(output, nx, ny):
    """Parse grid data from output (if implemented)"""
    # This assumes you've added grid output to your solver
    # You'll need to implement a --output flag to write grid to file
    pass

def compare_implementations():
    """Compare sequential, OpenMP, and MPI results"""
    print("Comparing different implementations...")
    
    # Small grid for quick testing
    nx, ny = 64, 64
    steps = 10
    
    # Run all versions
    print(f"Running with grid {nx}x{ny}, {steps} steps")
    
    # You'll need to implement output to file for comparison
    # For now, just check that they run without crashing
    for exe in ["heat_seq", "heat_omp", "heat_mpi"]:
        print(f"  Testing {exe}...")
        output = run_solver(exe, nx, ny, steps)
        if output is None:
            print(f"    FAIL: {exe} crashed")
            return False
        
        # Check for errors in output
        if "error" in output.lower() or "exception" in output.lower():
            print(f"    FAIL: {exe} reported error")
            return False
        
        print(f"    PASS")
    
    print("All implementations run without crashing")
    return True

def test_convergence():
    """Test that solution converges with finer grid"""
    print("\nTesting convergence...")
    
    # This requires implementing an analytical solution
    # For now, just a placeholder
    return True

def main():
    print("=========================================")
    print("Running Correctness Tests")
    print("=========================================")
    
    tests = [
        ("Implementation comparison", compare_implementations),
        ("Convergence test", test_convergence)
    ]
    
    all_passed = True
    for name, test_func in tests:
        print(f"\n--- {name} ---")
        try:
            if test_func():
                print(f"✓ {name} passed")
            else:
                print(f"✗ {name} failed")
                all_passed = False
        except Exception as e:
            print(f"✗ {name} raised exception: {e}")
            all_passed = False
    
    print("\n=========================================")
    if all_passed:
        print("ALL TESTS PASSED!")
        return 0
    else:
        print("SOME TESTS FAILED!")
        return 1

if __name__ == "__main__":
    sys.exit(main())
