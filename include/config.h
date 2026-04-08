#ifndef CONFIG_H
#define CONFIG_H

// Simulation parameters structure
struct SimulationParams {
    int Nx;              // Grid size in x-direction
    int Ny;              // Grid size in y-direction
    int num_steps;       // Number of time steps
    double Lx;           // Physical domain length in x
    double Ly;           // Physical domain length in y
    double alpha;        // Thermal diffusivity
    int boundary_type;    // 0: Dirichlet, 1: Neumann
    int init_type;       // 0: uniform, 1: Gaussian, 2: random
    double T_left;       // Left boundary temperature (Dirichlet)
    double T_right;      // Right boundary temperature (Dirichlet)
    double T_bottom;     // Bottom boundary temperature (Dirichlet)
    double T_top;        // Top boundary temperature (Dirichlet)
    double T_initial;    // Initial temperature (uniform case)
    
    // Default constructor
    SimulationParams() : 
        Nx(256), Ny(256), num_steps(100), 
        Lx(1.0), Ly(1.0), alpha(1.0),
        boundary_type(0), init_type(0),
        T_left(0.0), T_right(0.0), T_bottom(0.0), T_top(0.0),
        T_initial(0.0) {}
};

// Function to parse command line arguments
bool parse_arguments(int argc, char** argv, SimulationParams& params);

#endif // CONFIG_H
