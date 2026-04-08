#include "boundary.h"

BoundaryConditions::BoundaryConditions(int type,
                                       double left, double right,
                                       double bottom, double top) : boundary_type(type), T_left(left), T_right(right),
                                                                    T_bottom(bottom), T_top(top) {}

void BoundaryConditions::apply(Grid& grid) const
{
    int nx = grid.get_nx();
    int ny = grid.get_ny();

    if (boundary_type == 0)
    { // Dirichlet (fixed temperature)
        // TODO: Apply Dirichlet boundary conditions
        // Left and right boundaries
        for (int j = 0; j < ny; j++)
        {
            grid(0, j) = T_left;
            grid(nx - 1, j) = T_right;
        }

        // Top and bottom boundaries
        for (int i = 0; i < nx; i++)
        {
            grid(i, 0) = T_bottom;
            grid(i, ny - 1) = T_top;
        }
    }
    else
    { // Neumann (insulated)
        // TODO: Apply Neumann boundary conditions
        // For insulated boundaries, the gradient is zero:
        // T(0,j) = T(1,j), T(nx-1,j) = T(nx-2,j)
        // T(i,0) = T(i,1), T(i,ny-1) = T(i,ny-2)

        // Left and right boundaries
        for (int j = 0; j < ny; j++)
        {
            grid(0, j) = grid(1, j);
            grid(nx - 1, j) = grid(nx - 2, j);
        }

        // Top and bottom boundaries
        for (int i = 0; i < nx; i++)
        {
            grid(i, 0) = grid(i, 1);
            grid(i, ny - 1) = grid(i, ny - 2);
        }
    }
}
