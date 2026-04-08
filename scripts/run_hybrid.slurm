#!/bin/bash
#SBATCH --job-name=heat_hybrid
#SBATCH --output=heat_hybrid_%j.out
#SBATCH --error=heat_hybrid_%j.err
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=2
#SBATCH --cpus-per-task=4
#SBATCH --time=01:00:00
#SBATCH --partition=training

# Load modules
module load gcc
module load openmpi

cd $SLURM_SUBMIT_DIR
cd ..
mkdir -p build
cd build

cmake ..
make heat_hybrid -j4

# Test different MPI/OpenMP ratios
for nodes in 1 2 4; do
    for tasks_per_node in 1 2 4; do
        for threads_per_task in 1 2 4; do
            total_threads=$((nodes * tasks_per_node * threads_per_task))
            echo "========================================"
            echo "Nodes: $nodes, MPI ranks per node: $tasks_per_node"
            echo "Threads per rank: $threads_per_task, Total threads: $total_threads"
            echo "========================================"
            
            export OMP_NUM_THREADS=$threads_per_task
            srun --nodes=$nodes --ntasks-per-node=$tasks_per_node \
                 --cpus-per-task=$threads_per_task \
                 ./bin/heat_hybrid --nx 8192 --ny 8192 --steps 100 --init 1 --boundary 0
        done
    done
done