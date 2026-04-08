#!/bin/bash
#SBATCH --job-name=heat_mpi
#SBATCH --output=heat_mpi_%j.out
#SBATCH --error=heat_mpi_%j.err
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=2
#SBATCH --cpus-per-task=1
#SBATCH --time=01:00:00
#SBATCH --partition=training

# Load modules (adjust based on your cluster)
module load gcc
module load openmpi

# Change to submission directory
cd $SLURM_SUBMIT_DIR

# Create build directory if needed
cd ..
mkdir -p build
cd build

# Configure and build
cmake ..
make heat_mpi -j4

# Strong scaling test
for nodes in 1 2 4; do
    for tasks_per_node in 1 2 4; do
        total_ranks=$((nodes * tasks_per_node))
        echo "========================================"
        echo "Running with $nodes nodes, $tasks_per_node ranks per node (total: $total_ranks)"
        echo "========================================"
        
        srun --nodes=$nodes --ntasks-per-node=$tasks_per_node \
             ./bin/heat_mpi --nx 8192 --ny 8192 --steps 100 --init 1 --boundary 0
    done
done