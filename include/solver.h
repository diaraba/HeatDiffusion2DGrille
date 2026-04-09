#ifndef SOLVER_H
#define SOLVER_H

#include "grid.h"
#include "boundary.h"
#include "timer.h"
#include "config.h"
#ifdef USE_MPI
#include <mpi.h>
#endif
// Base solver class (interface)
class Solver
{
protected:
    Grid *T_old;
    Grid *T_new;
    BoundaryConditions *bc;
    double dx, dy, dt;
    double alpha;
    double factor; // alpha * dt / (dx*dx) assuming dx=dy

    // Timing
    Timer total_timer;
    Timer computation_timer;
    Timer communication_timer;

public:
    Solver();
    virtual ~Solver();

    // Initialize solver with parameters
    virtual void initialize(const SimulationParams &params);

    // Perform one time step (to be implemented by derived classes)
    virtual void time_step() = 0;

    // Run complete simulation
    virtual void run(int num_steps);

    // Get results
    Grid *get_result() { return T_old; }

    // Get timing information
    virtual void report_timing() const;

    // Validate against reference solution
    virtual bool validate(Grid *reference, double tolerance = 1e-8) const;
};

// Sequential solver
class SequentialSolver : public Solver
{
public:
    SequentialSolver();
    virtual void time_step() override;
};

// OpenMP solver
class OpenMPSolver : public Solver
{
private:
    int num_threads;

public:
    OpenMPSolver(int threads = 0); // 0 = use default
    virtual void time_step() override;
};

// MPI solver (forward declaration - implementation in separate file)
#ifdef USE_MPI
class MPISolver : public Solver
{
private:
    int rank, size;
    int coords[2]; // Cartesian coordinates
    int dims[2];
    int neighbors[4];       // Neighbor ranks (left, right, bottom, top)
    int local_nx, local_ny; // Local grid dimensions (including halos)
    int global_nx, global_ny;
    int start_i, start_j; // Starting indices in global grid
    Grid *local_grid_old;
    Grid *local_grid_new;
#ifdef USE_MPI
    MPI_Comm cart_comm;
#endif
    // Halo exchange buffers
    double *send_buffer_left;
    double *send_buffer_right;
    double *send_buffer_bottom;
    double *send_buffer_top;
    double *recv_buffer_left;
    double *recv_buffer_right;
    double *recv_buffer_bottom;
    double *recv_buffer_top;

public:
    MPISolver();
    virtual ~MPISolver();

    virtual void initialize(const SimulationParams &params) override;
    virtual void time_step() override;
    virtual void run(int num_steps) override;
    virtual void report_timing() const override;

private:
    void setup_cartesian_communicator();
    void exchange_halos();
    void exchange_halos_nonblocking();
    void gather_results();
};

// Hybrid MPI+OpenMP solver
class HybridSolver : public MPISolver
{
private:
    int num_threads_per_process;

public:
    HybridSolver(int threads_per_process = 0);
    virtual void time_step() override;
};
#endif // USE_MPI

#endif // SOLVER_H
