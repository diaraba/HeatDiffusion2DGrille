#include "solver.h"
#include <iostream>
#include <cmath>

Solver::Solver() : T_old(nullptr), T_new(nullptr), bc(nullptr) {}

Solver::~Solver()
{
    delete T_old;
    delete T_new;
    delete bc;
}

void Solver::initialize(const SimulationParams &params)
{
    // Create grids
    T_old = new Grid(params.Nx, params.Ny);
    T_new = new Grid(params.Nx, params.Ny);

    // Initialize grid with initial condition
    T_old->initialize(params.init_type, params.T_initial, params.Lx, params.Ly);

    // Create boundary conditions
    bc = new BoundaryConditions(params.boundary_type,
                                params.T_left, params.T_right,
                                params.T_bottom, params.T_top);

    // Apply initial boundary conditions
    bc->apply(*T_old);

    // Calculate grid spacing and time step
    dx = params.Lx / (params.Nx - 1);
    dy = params.Ly / (params.Ny - 1);

    // For stability, dt <= min(dx^2, dy^2) / (4 * alpha)
    double h2 = std::min(dx * dx, dy * dy);
    dt = 0.25 * h2 / params.alpha; // Conservative time step

    factor = params.alpha * dt / (dx * dx); // assuming dx=dy for simplicity

    std::cout << "Grid spacing: dx=" << dx << ", dy=" << dy << std::endl;
    std::cout << "Time step: dt=" << dt << ", factor=" << factor << std::endl;
}

void Solver::run(int num_steps)
{
    total_timer.start();

    for (int step = 0; step < num_steps; step++)
    {
        time_step();

        // Swap grids
        T_old->swap(*T_new);
    }

    total_timer.stop();
}

void Solver::report_timing() const
{
    std::cout << "Timing:" << std::endl;
    std::cout << "  Total: ";
    total_timer.report();
    std::cout << "  Computation: ";
    computation_timer.report();
    std::cout << "  Communication: ";
    communication_timer.report();
    std::cout << "COMM_RATIO  " << (communication_timer.elapsed() / total_timer.elapsed()) * 100 << std::endl;
}

bool Solver::validate(Grid *reference, double tolerance) const
{
    if (!reference || !T_old)
        return false;
    return T_old->is_approx_equal(*reference, tolerance);
}