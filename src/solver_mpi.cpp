#ifdef USE_MPI
#include "solver.h"
#include <mpi.h>
#include <iostream>
#include <cmath>

MPISolver::MPISolver() : Solver(),
                         rank(-1), size(-1), cart_comm(MPI_COMM_NULL),
                         local_grid_old(nullptr), local_grid_new(nullptr),
                         send_buffer_left(nullptr), send_buffer_right(nullptr),
                         send_buffer_bottom(nullptr), send_buffer_top(nullptr),
                         recv_buffer_left(nullptr), recv_buffer_right(nullptr),
                         recv_buffer_bottom(nullptr), recv_buffer_top(nullptr)
{

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
}

MPISolver::~MPISolver()
{
    // TODO: Clean up allocated memory and MPI communicator
    delete local_grid_old;
    delete local_grid_new;

    delete[] send_buffer_left;
    delete[] send_buffer_right;
    delete[] send_buffer_bottom;
    delete[] send_buffer_top;
    delete[] recv_buffer_left;
    delete[] recv_buffer_right;
    delete[] recv_buffer_bottom;
    delete[] recv_buffer_top;

    if (cart_comm != MPI_COMM_NULL)
    {
        MPI_Comm_free(&cart_comm);
    }
}

void MPISolver::setup_cartesian_communicator()
{
    // TODO: Create a 2D Cartesian communicator
    // 1. Determine grid dimensions for processes (try to make it square)
    // 2. Use MPI_Dims_create to create a balanced decomposition
    // 3. Use MPI_Cart_create to create the communicator
    // 4. Use MPI_Cart_coords to get coordinates for this rank
    // 5. Use MPI_Cart_shift to find neighbors in each direction
    int dims[2] = {0, 0};

    MPI_Dims_create(size, 2, dims);

    int periods[2] = {0, 0};

    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);

    MPI_Cart_coords(cart_comm, rank, 2, coords);

    MPI_Cart_shift(cart_comm, 0, 1, &neighbors[0], &neighbors[1]);

    MPI_Cart_shift(cart_comm, 1, 1, &neighbord[2], &neighbors[3]);

    std::cout << "Rank " << rank
              << " coords (" << coords[0] << "," << coords[1] << ")"
              << " neighbors: L=" << neighbors[0]
              << " R=" << neighbors[1]
              << " B=" << neighbors[2]
              << " T=" << neighbors[3]
              << std::endl;
}

void MPISolver::initialize(const SimulationParams &params)
{
    // TODO: Initialize MPI solver
    // 1. Store global dimensions
    // 2. Set up Cartesian communicator
    // 3. Calculate local grid dimensions (including halos)
    // 4. Allocate local grids
    // 5. Initialize local grid with appropriate part of global initial condition
    // 6. Allocate send/receive buffers
    // 7. Initialize base solver parameters (dx, dy, dt, factor)

    global_nx = params.Nx;
    global_ny = params.Ny;

    setup_cartesian_communicator();

    // Calculate local dimensions including halos
    // local_nx = (global_nx / dims[0]) + 2;  // +2 for halos
    // local_ny = (global_ny / dims[1]) + 2;
    // Taille locale sans halo
    int nx_local = global_nx / dims[0];
    int ny_local = global_ny / dims[1];
    // Taille locale avec halo
    local_nx = nx_local + 2;
    local_ny = ny_local + 2;
    // Position dans la grille globale
    start_i = coords[0] * nx_local;
    start_j = coords[1] * ny_local;

    // Allocate local grids
    local_grid_old = new Grid(local_nx, local_ny);
    local_grid_new = new Grid(local_nx, local_ny);

    // Initialize local grid from global initial condition
    // This requires mapping global indices to local indices
    for (int i = 1; i <= nx_local; i++)
    {
        for (int j = 1; j <= ny_local; j++)
        {
            int global_i = start_i + (i - 1);
            int global_j = start_j + (j - 1);
            (*local_grid_old)(i, j) = params.T_initial;
        }
    }
    // Allocate communication buffers
    // send_buffer_left = new double[local_ny-2];
    // etc.
    send_buffer_left = new double[ny_local];
    send_buffer_right = new double[ny_local];
    send_buffer_bottom = new double[nx_local];
    send_buffer_top = new double[nx_local];
    recv_buffer_left = new double[ny_local];
    recv_buffer_right = new double[ny_local];
    recv_buffer_bottom = new double[nx_local];
    recv_buffer_top = new double[nx_local];

    // Initialisation des parametres du solver de base

    dx = params.Lx / (params.Nx - 1);
    dy = params.Ly / (params.Ny - 1);
    double h2 = std::min(dx * dy, dy * dy);
    dt = 0.25 * h2 / params.alpha;
    factor = prams.alpha * dt / (dx * dx);
}

void MPISolver::exchange_halos()
{
    // TODO: Implement blocking halo exchange
    // Use MPI_Sendrecv for each neighbor direction
    // This is simpler but may have more waiting
    int nx = local_nx;
    int ny = local_ny;

    // Buffers temporaires
    std::vector<double> send_left(ny - 2), recv_left(ny - 2);
    std::vector<double> send_right(ny - 2), recv_right(ny - 2);

    std::vector<double> send_bottom(nx - 2), recv_bottom(nx - 2);
    std::vector<double> send_top(nx - 2), recv_top(nx - 2);

    // Left  / RIGHT COLONNES

    for (int j = 1; j < ny - 1; j++)
    {
        send_left[j - 1] = (*local_grid_old)(1, j);
        send_right[j - 1] = (*local_grid_old)(nx - 2, j)
    }

    MPI_Sendrecv(send_left.data(), ny - 2, MPI_DOUBLE, neighbors[0], 0,
                 recv_right.data(), ny - 2, MPI_DOUBLE, neighbors[1], 0,
                 cart_comm, MPI_STATUS_IGNORE);
    MPI_Sendrecv(send_right.data(), ny - 2, MPI_DOUBLE, neighbors[1], 1,
                 recv_left.data(), ny - 2, MPI_DOUBLE, neighbors[0], 1,
                 cart_comm, MPI_STATUS_IGNORE);
    // Copie dans les halos gauches et droites
    for (int j = 1; j < ny - 1; j++)
    {
        (*local_grid_old)(0, j) = recv_left[j - 1];
        (*local_grid_old)(nx - 1, j) = recv_right[j - 1];
    }

    // Bottom / Top colonnes
    for (int i = 1; i < nx - 1; i++)
    {
        send_bottom[i - 1] = (*local_grid_old)(i, 1);
        send_top[i - 1] = (*local_grid_old)(ny - 2, i);
    }

    MPI_Sendrecv(send_bottom.data(), nx - 2, MPI_DOUBLE, neighbors[2], 2,
                 recv_top.data(), nx - 2, MPI_DOUBLE, neighbors[3], 2,
                 cart_comm, MPI_STATUS_IGNORE);
    MPI_Sendrecv(send_top.data(), nx - 2, MPI_DOUBLE, neighbors[3], 3,
                 recv_bottom.data(), nx - 2, MPI_DOUBLE, neighbors[2], 3,
                 cart_comm, MPI_STATUS_IGNORE);

    // Copie dans les halos haut et bas

    for (int i = 1; i < nx - 1; i++)
    {
        (*local_grid_old)(i, 0) = recv_bottom[i - 1];
        (*local_grid_old)(i, ny - 2) = recv_top[i - 1];
    }
}

void MPISolver::exchange_halos_nonblocking()
{
    // TODO: Implement non-blocking halo exchange
    // Use MPI_Isend and MPI_Irecv with MPI_Waitall
    // This allows overlap of communication and computation

    int nx = local_nx;
    int ny = local_ny;

    MPI_Request requests[8];
    int req_count = 0;

    // Preparation des buffers
    for (int j = 1; j < ny - 1; j++)
    {
        send_buffer_left[j - 1] = (*local_grid_old)(1, j);
        send_buffer_right[j - 1] = (*local_grid_old)(nx - 2, j);
    }

    for (int i = 1; i < nx - 1; i++)
    {
        send_buffer_bottom[i - 1] = (*local_grid_old)(i, 1);
        send_buffer_top[i - 1] = (*local_grid_old)(i, ny - 2);
    }

    //  Exchange with left and right neighbors
    //  Exchange with bottom and top neighbors

    // Recv
    MPI_Irecv(recv_buffer_left, ny - 2, MPI_DOUBLE, neighbors[0], 0, cart_comm, &requests[req_count++]);
    MPI_Irecv(recv_buffer_right, ny - 2, MPI_DOUBLE, neighbors[1], 1, cart_comm, &requests[req_count++]);
    MPI_Irecv(recv_buffer_bottom, nx - 2, MPI_DOUBLE, neighbors[2], 2, cart_comm, &requests[req_count++]);
    MPI_Irecv(recv_buffer_top, nx - 2, MPI_DOUBLE, neighbors[3], 3, cart_comm, &requests[req_count++]);
    // Send
    MPI_Isend(send_buffer_left, ny - 2, MPI_DOUBLE, neighbors[0], 1, cart_comm, &requests[req_count++]);
    MPI_Isend(send_buffer_right, ny - 2, MPI_DOUBLE, neighbors[1], 0, cart_comm, &requests[req_count++]);
    MPI_Isend(send_buffer_bottom, nx - 2, MPI_DOUBLE, neighbors[2], 3, cart_comm, &requests[req_count++]);
    MPI_Isend(send_buffer_top, nx - 2, MPI_DOUBLE, neighbors[3], 2, cart_comm, &requests[req_count++]);
    //    Wait for all communications to complete
    MPI_Waitall(req, requests, MPI_STATUSES_IGNORE);

    // Copie dans les halos droit, gauche, haut et bas

    for (int j = 1; j < ny - 1, j++)
    {
        (*local_grid_old)(0, j) = recv_buffer_left[j - 1];
        (*local_grid_old)(nx - 1, j) = recv_buffer_right[j - 1]
    }
    for (int i = 1; i < nx - 1; i++)
    {
        (*local_grid_old)(i, 0) = recv_buffer_bottom[i - 1];
        (*local_grid_old)(i, ny - 1) = recv_buffer_top[i - 1];
    }
}

void MPISolver::time_step()
{
    // TODO: Implement one MPI time step
    // 1. Exchange halo cells with neighbors
    // 2. Update interior points (excluding halos)
    // 3. Apply boundary conditions (only on physical boundaries)
    // 4. Swap local grids
    int nx = local_nx;
    int ny = local_ny;

    // Start communication timer
    communication_timer.start();

    // Exchange halos
    exchange_halos_nonblocking();

    communication_timer.stop();

    // Start computation timer
    computation_timer.start();

    // TODO: Update interior points (similar to sequential but on local grid)
    // Be careful with indices: halos are at i=0, i=local_nx-1, j=0, j=local_ny-1

    for (int i = 1; i < nx - 1; i++)
    {
        for (int j = 1; j < ny - 1; j++)
        {
            double center = (*local_grid_old)(i, j);
            double left = (*local_grid_old)(i - 1, j);
            double right = (*local_grid_old)(i + j, j);
            double down = (*local_grid_old)(i, j - 1);
            double up = (*local_grid_old)(i, j + 1);
            (*local_grid_old)(i, j) = center + factor * (left + right + down + up + 4.0 * center);
        }
    }

    computation_timer.stop();

    // Apply boundary conditions to physical boundaries only
    // This requires knowing if this rank is on a global boundary
    // if (coords[0] == 0) // left boundary
    // if (coords[0] == dims[0]-1) // right boundary
    // etc.

    // Gauche global
    if (coords[0] == 0)
    {
        for (int j = 1; j < ny - 1; j++)
        {
            (*local_grid_new)(1, j) = bc->get_type() == 0 ? bc->T_left : (*local_grid_old)(2, j)
        }
    }
    // Droite global
    if (coords[0])
    {
        for (int i = 1; j < ny - 1; j++)
        {
            (*local_grid_new)(nx - 2, j) = bc->get_type() == 0 ? bc->T_right : (*local_grid_new)(nx - 3, j);
        }
    }

    // Bas global
    if (coords[1] == 0)
    {
        for (int i; i < nx - 1; i++)
        {
            (*local_grid_new)(i, ny - 2) = bc->get_type() == 0 ? bc->T_bottom : (*local_grid_new)(i, 2);
        }
    }
    // Haut global
    if (coords[1] == dims[1] - 1)
    {
        for (int i = 1; i < nx - 1; i++)
        {
            (*local_grid_new)(i, ny - 2) = bc->get_type() == 0 ? bc->T_top : (*local_grid_new)(i, ny - 3);
        }
    }
}

void MPISolver::run(int num_steps)
{
    total_timer.start();

    for (int step = 0; step < num_steps; step++)
    {
        time_step();

        // Swap grids
        std::swap(local_grid_old, local_grid_new);
    }

    total_timer.stop();

    // Gather results to rank 0 for output/validation
    gather_results();
}

void MPISolver::gather_results()
{
    // TODO: Gather all local grids to rank 0
    // This is for final validation and output only
    // Not needed for performance measurements

    int nx_local = local_nx - 2;
    int ny_local = local_ny - 2;
    int local_size = nx_local * ny_local;
    std::vector<double> send_buffer(local_size);
    int idx = 0;
    for (int i = 1; i < nx_local; i++)
    {
        for (int j = 1; j <= ny_local; j++)
        {
            send_buffer[idx++] = (*local_grid_old)(i, j);
        }
    }
    if (rank == 0)
    {
        // buffer global
        std::vector<double> global_data(global_nx * global_ny);

        // copier ses propres donnees
        int start_i = coords[0] * nx_local;
        int start_j = coords[1] * ny_local;
        for (int i = 1; i < nx_local; i++)
        {
            for (int j = 1; j < ny_local; j++)
            {
                int gi = start_i + i;
                int gj = start_j + j;
                global_data[gi * global_ny + gj] = send_buffer[idx++];
            }
        }

        // Allocate global grid
        // Receive from all ranks and assemble
        for (int p = 1; p < size; p++)
        {
            std::vector<double> recv_buffer(local_size);
            MPI_Recv(recv_buffer.data(), local_size, MPI_DOUBLE, p, 0, cart_comm, MPI_STATUS_IGNORE);
            // recuperer coords du processus p
            int coords_p[2];
            MPI_Cart_coords(cart_comm, p, 2, coords_p);
            int si = coords_p[0] * nx_local;
            int sj = coords_p[1] * ny_local;

            idx = 0;
            for (int i = 0; i < nx_local; i++)
            {
                for (int j = 0; j < ny_local; j++)
                {
                    int gi = si + 1;
                    int gj = sj + 1;
                    global_data[gi * global_ny + gj] = recv_buffer[idx++];
                }
            }
        }
        std::out << "Gather complete on rank 0" << std::endl;
    }
    else
    {
        // Send local grid to rank 0
        MPI_SEND(send_buffer.data(), local_size, MPI_DOUBLE, 0, 0, cart_comm);
    }
}

void MPISolver::report_timing() const
{
    if (rank == 0)
    {
        std::cout << "MPI Solver Timing:" << std::endl;
        std::cout << "  Total: ";
        total_timer.report();
        std::cout << "  Computation: ";
        computation_timer.report();
        std::cout << "  Communication: ";
        communication_timer.report();
        std::cout << "  Communication ratio: "
                  << (communication_timer.elapsed() / total_timer.elapsed() * 100.0)
                  << "%" << std::endl;
    }
}

#endif // USE_MPI
