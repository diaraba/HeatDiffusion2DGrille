#ifndef BOUNDARY_H
#define BOUNDARY_H

#include "grid.h"

class BoundaryConditions
{
private:
    int boundary_type; // 0: Dirichlet, 1: Neumann
    double T_left, T_right, T_bottom, T_top;

public:
    // Constructor
    BoundaryConditions(int type = 0,
                       double left = 0.0, double right = 0.0,
                       double bottom = 0.0, double top = 0.0);

    // Apply boundary conditions to a grid
    void apply(Grid &grid) const;
 
    // Getters
    inline int get_type() const { return boundary_type; }
    inline double get_T_right() const { return T_right; }
    inline double get_T_left() const { return T_left; }
    inline double get_T_bottom() const { return T_bottom; }
    inline double get_T_top() const { return T_top; }
};

#endif // BOUNDARY_H
