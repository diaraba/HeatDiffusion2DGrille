#ifdef USE_MPI
#include "solver.h"
#include <mpi.h>
#include <iostream>
#include <cmath>
#include <vector>

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
    dims[0] = 0;
    dims[1] = 0;
    MPI_Dims_create(size, 2, dims);

    int periods[2] = {0, 0};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);
    MPI_Cart_coords(cart_comm, rank, 2, coords);

    // 0: axe X (gauche/droite), 1: axe Y (bas/haut)
    MPI_Cart_shift(cart_comm, 0, 1, &neighbors[0], &neighbors[1]);
    MPI_Cart_shift(cart_comm, 1, 1, &neighbors[2], &neighbors[3]);
}

void MPISolver::initialize(const SimulationParams &params)
{
    Solver::initialize(params);
    global_nx = params.Nx;
    global_ny = params.Ny;

    setup_cartesian_communicator();

    int nx_inner = global_nx / dims[0];
    int ny_inner = global_ny / dims[1];

    local_nx = nx_inner + 2;
    local_ny = ny_inner + 2;

    start_i = coords[0] * nx_inner;
    start_j = coords[1] * ny_inner;

    local_grid_old = new Grid(local_nx, local_ny);
    local_grid_new = new Grid(local_nx, local_ny);

    local_grid_old->fill(params.T_initial);
    local_grid_new->fill(params.T_initial);

    // Buffers de communication (taille correspondant à la partie utile sans halo)
    send_buffer_left = new double[ny_inner];
    send_buffer_right = new double[ny_inner];
    send_buffer_bottom = new double[nx_inner];
    send_buffer_top = new double[nx_inner];
    recv_buffer_left = new double[ny_inner];
    recv_buffer_right = new double[ny_inner];
    recv_buffer_bottom = new double[nx_inner];
    recv_buffer_top = new double[nx_inner];

    dx = params.Lx / (params.Nx - 1);
    dy = params.Ly / (params.Ny - 1);
    double h2 = std::min(dx * dx, dy * dy);
    dt = 0.25 * h2 / params.alpha;
    factor = params.alpha * dt / (dx * dx);
}

void MPISolver::exchange_halos_nonblocking()
{
    int nx_inner = local_nx - 2;
    int ny_inner = local_ny - 2;
    MPI_Request requests[8];
    int req_count = 0;

    // Remplissage des buffers d'envoi
    for (int j = 0; j < ny_inner; j++)
    {
        send_buffer_left[j] = (*local_grid_old)(1, j + 1);
        send_buffer_right[j] = (*local_grid_old)(local_nx - 2, j + 1);
    }
    for (int i = 0; i < nx_inner; i++)
    {
        send_buffer_bottom[i] = (*local_grid_old)(i + 1, 1);
        send_buffer_top[i] = (*local_grid_old)(i + 1, local_ny - 2);
    }

    // Réceptions (IRECV)
    if (neighbors[0] != MPI_PROC_NULL)
        MPI_Irecv(recv_buffer_left, ny_inner, MPI_DOUBLE, neighbors[0], 1, cart_comm, &requests[req_count++]);
    if (neighbors[1] != MPI_PROC_NULL)
        MPI_Irecv(recv_buffer_right, ny_inner, MPI_DOUBLE, neighbors[1], 0, cart_comm, &requests[req_count++]);
    if (neighbors[2] != MPI_PROC_NULL)
        MPI_Irecv(recv_buffer_bottom, nx_inner, MPI_DOUBLE, neighbors[2], 3, cart_comm, &requests[req_count++]);
    if (neighbors[3] != MPI_PROC_NULL)
        MPI_Irecv(recv_buffer_top, nx_inner, MPI_DOUBLE, neighbors[3], 2, cart_comm, &requests[req_count++]);

    // Envois (ISEND)
    if (neighbors[0] != MPI_PROC_NULL)
        MPI_Isend(send_buffer_left, ny_inner, MPI_DOUBLE, neighbors[0], 0, cart_comm, &requests[req_count++]);
    if (neighbors[1] != MPI_PROC_NULL)
        MPI_Isend(send_buffer_right, ny_inner, MPI_DOUBLE, neighbors[1], 1, cart_comm, &requests[req_count++]);
    if (neighbors[2] != MPI_PROC_NULL)
        MPI_Isend(send_buffer_bottom, nx_inner, MPI_DOUBLE, neighbors[2], 2, cart_comm, &requests[req_count++]);
    if (neighbors[3] != MPI_PROC_NULL)
        MPI_Isend(send_buffer_top, nx_inner, MPI_DOUBLE, neighbors[3], 3, cart_comm, &requests[req_count++]);

    if (req_count > 0)
        MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);

    // Mise à jour des halos
    for (int j = 0; j < ny_inner; j++)
    {
        if (neighbors[0] != MPI_PROC_NULL)
            (*local_grid_old)(0, j + 1) = recv_buffer_left[j];
        if (neighbors[1] != MPI_PROC_NULL)
            (*local_grid_old)(local_nx - 1, j + 1) = recv_buffer_right[j];
    }
    for (int i = 0; i < nx_inner; i++)
    {
        if (neighbors[2] != MPI_PROC_NULL)
            (*local_grid_old)(i + 1, 0) = recv_buffer_bottom[i];
        if (neighbors[3] != MPI_PROC_NULL)
            (*local_grid_old)(i + 1, local_ny - 1) = recv_buffer_top[i];
    }
}

void MPISolver::time_step()
{
    communication_timer.start();
    exchange_halos_nonblocking();
    communication_timer.stop();

    computation_timer.start();
    for (int i = 1; i < local_nx - 1; i++)
    {
        for (int j = 1; j < local_ny - 1; j++)
        {
            double center = (*local_grid_old)(i, j);
            (*local_grid_new)(i, j) = center + factor * ((*local_grid_old)(i - 1, j) + (*local_grid_old)(i + 1, j) +
                                                         (*local_grid_old)(i, j - 1) + (*local_grid_old)(i, j + 1) - 4.0 * center);
        }
    }
    computation_timer.stop();

    // Conditions aux limites physiques (Dirichlet ou Neumann)
    if (coords[0] == 0)
    { // Gauche
        for (int j = 1; j < local_ny - 1; j++)
            (*local_grid_new)(1, j) = (bc->get_type() == 0) ? bc->get_T_left() : (*local_grid_new)(2, j);
    }
    if (coords[0] == dims[0] - 1)
    { // Droite
        for (int j = 1; j < local_ny - 1; j++)
            (*local_grid_new)(local_nx - 2, j) = (bc->get_type() == 0) ? bc->get_T_right() : (*local_grid_new)(local_nx - 3, j);
    }
    if (coords[1] == 0)
    { // Bas
        for (int i = 1; i < local_nx - 1; i++)
            (*local_grid_new)(i, 1) = (bc->get_type() == 0) ? bc->get_T_bottom() : (*local_grid_new)(i, 2);
    }
    if (coords[1] == dims[1] - 1)
    { // Haut
        for (int i = 1; i < local_nx - 1; i++)
            (*local_grid_new)(i, local_ny - 2) = (bc->get_type() == 0) ? bc->get_T_top() : (*local_grid_new)(i, local_ny - 3);
    }
}

void MPISolver::run(int num_steps)
{
    total_timer.start();
    for (int step = 0; step < num_steps; step++)
    {
        time_step();
        local_grid_old->swap(*local_grid_new);
    }
    total_timer.stop();
    gather_results();
}

void MPISolver::gather_results()
{
    int nx_inner = local_nx - 2;
    int ny_inner = local_ny - 2;
    int local_size = nx_inner * ny_inner;
    std::vector<double> send_buffer(local_size);

    int idx = 0;
    for (int i = 1; i <= nx_inner; i++)
    {
        for (int j = 1; j <= ny_inner; j++)
        {
            send_buffer[idx++] = (*local_grid_old)(i, j);
        }
    }

    if (rank == 0)
    {
        // Remplir la partie locale du Rank 0 dans T_old (grille globale)
        idx = 0;
        for (int i = 0; i < nx_inner; i++)
        {
            for (int j = 0; j < ny_inner; j++)
            {
                (*T_old)(start_i + i, start_j + j) = send_buffer[idx++];
            }
        }

        for (int p = 1; p < size; p++)
        {
            std::vector<double> recv_buffer(local_size);
            MPI_Recv(recv_buffer.data(), local_size, MPI_DOUBLE, p, 0, cart_comm, MPI_STATUS_IGNORE);

            int p_coords[2];
            MPI_Cart_coords(cart_comm, p, 2, p_coords);
            int p_start_i = p_coords[0] * nx_inner;
            int p_start_j = p_coords[1] * ny_inner;

            idx = 0;
            for (int i = 0; i < nx_inner; i++)
            {
                for (int j = 0; j < ny_inner; j++)
                {
                    // FIX : Utilisation correcte de p_start_j + j
                    (*T_old)(p_start_i + i, p_start_j + j) = recv_buffer[idx++];
                }
            }
        }
    }
    else
    {
        MPI_Send(send_buffer.data(), local_size, MPI_DOUBLE, 0, 0, cart_comm);
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
