# HPC Capstone Project: Parallel Heat Diffusion Solver

This project implements a parallel solver for the 2D heat diffusion equation using stencil computations. Multiple parallel versions are provided: sequential, OpenMP, MPI, and hybrid MPI+OpenMP.

## Project Structure

- `include/`: Header files with class declarations
- `src/`: Source files with implementation skeletons
- `scripts/`: Run scripts and analysis tools
- `tests/`: Unit tests and validation scripts
- `results/`: Output directory for performance data

## Building the Code

```bash
mkdir build
cd build
cmake ..
make