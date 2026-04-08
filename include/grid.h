#ifndef GRID_H
#define GRID_H

class Grid {
private:
    double* data;        // Contiguous 1D data array
    double** rows;       // 2D view of the data
    int nx, ny;          // Dimensions
    
public:
    // Constructor/Destructor
    Grid(int nx, int ny);
    ~Grid();
    
    // Disable copy constructor and assignment
    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;
    
    // Access operators
    inline double& operator()(int i, int j) { return rows[i][j]; }
    inline const double& operator()(int i, int j) const { return rows[i][j]; }
    
    // Get dimensions
    inline int get_nx() const { return nx; }
    inline int get_ny() const { return ny; }
    
    // Fill with constant value
    void fill(double value);
    
    // Apply initial condition based on type
    void initialize(int init_type, double T_initial, 
                    double Lx, double Ly);
    
    // Swap with another grid (for double buffering)
    void swap(Grid& other);
    
    // Get raw pointer to data (for MPI operations)
    inline double* get_data() { return data; }
    
    // Check if two grids are approximately equal
    bool is_approx_equal(const Grid& other, double tolerance = 1e-10) const;
};

#endif // GRID_H
