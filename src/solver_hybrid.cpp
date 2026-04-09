#if defined(USE_MPI) && defined(USE_OPENMP)
#include "solver.h"
#include <omp.h>
#include <iostream>

HybridSolver::HybridSolver(int threads_per_process) : MPISolver(), num_threads_per_process(threads_per_process)
{

    if (num_threads_per_process > 0)
    {
        omp_set_num_threads(num_threads_per_process);
    }
}

void HybridSolver::time_step()
{
    // TODO: Implement hybrid MPI+OpenMP time step
    // 1. Exchange halos using MPI (same as MPI version)
    // 2. Parallelize interior update with OpenMP
    // 3. Apply boundary conditions
    int nx = local_grid_old->get_nx();
    int ny = local_grid_old->get_ny();

    // Start communication timer
    communication_timer.start();

    // Exchange halos
    exchange_halos_nonblocking();

    communication_timer.stop();

    // Start computation timer
    computation_timer.start();

// TODO: Add OpenMP parallelization here
// Be careful with collapse(2) and thread safety
#pragma omp parallel for collapse(2) schedule(static)
    for (int i = 1; i < nx - 1; i++)
    {
        for (int j = 1; j < ny - 1; j++)
        {
            // Update interior points
            double center = (*local_grid_old)(i, j);
            double up = (*local_grid_old)(i, j + 1);
            double down = (*local_grid_old)(i, j - 1);
            double left = (*local_grid_old)(i - 1, j);
            double right = (*local_grid_old)(i + 1, j);

            (*local_grid_new)(i, j) = center + factor * (left + right +
                                                         down + up - 4.0 * center);
        }
    }

    computation_timer.stop();

    // Apply boundary conditions (serial)
    // Left
    if (coords[0] == 0)
    {
        for (int j = 1; j < ny - 1; j++)
        {
            (*local_grid_new)(1, j) = (bc->get_type() == 0) ? bc->get_T_left() : (*local_grid_old)(2, j);
        }
    }

    // Right
    if (coords[0] == dims[0] - 1)
    {
        for (int j = 1; j < ny - 1; j++)
        {
            (*local_grid_new)(nx - 2, j) = (bc->get_type() == 0) ? bc->get_T_right() : (*local_grid_old)(nx - 3, j);
        }
    }

    // Bottom
    if (coords[1] == 0)
    {
        for (int i = 1; i < nx - 1; i++)
        {
            (*local_grid_new)(i, 1) = (bc->get_type() == 0) ? bc->get_T_bottom() : (*local_grid_old)(i, 2);
        }
    }
    //  Top
    if (coords[1] == dims[1] - 1)
    {
        for (int i = 1; i < nx - 1; i++)
        {
            (*local_grid_new)(i, ny - 2) = (bc->get_type() == 0) ? bc->get_T_top() : (*local_grid_old)(i, ny - 3);
        }
    }
    // Need to apply only on physical boundaries
}

#endif // USE_MPI && USE_OPENMP
