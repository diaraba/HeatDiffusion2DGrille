#include "solver.h"
#include <omp.h>
#include <iostream>

OpenMPSolver::OpenMPSolver(int threads) : Solver(), num_threads(threads)
{
    if (num_threads > 0)
    {
        omp_set_num_threads(num_threads);
    }
}

void OpenMPSolver::time_step()
{
    int nx = T_old->get_nx();
    int ny = T_old->get_ny();
    computation_timer.start();
    // TODO: Implement OpenMP parallel time step
    // Use #pragma omp parallel for collapse(2)
    // Experiment with different scheduling policies
    // Avoid false sharing by using local variables

#pragma omp parallel for schedule(dynamic)
    for (int i = 1; i < nx - 1; i++)
    {
        for (int j = 1; j < ny - 1; j++)
        {
            // TODO: Implement stencil computation
            double center = (*T_old)(i, j);
            double up = (*T_old)(i, j + 1);
            double down = (*T_old)(i, j - 1);
            double left = (*T_old)(i - 1, j);
            double right = (*T_old)(i + 1, j);

            (*T_new)(i, j) = center + factor * (left + right +
                                                down + up - 4.0 * center);
        }
    }
    // Apply boundary conditions (should not be parallelized)
    bc->apply(*T_new);
    computation_timer.stop();
}
