#!/usr/bin/env python3

import subprocess
import csv
import re
import math
import os

EXEC = "./build/bin/heat_hybrid"

CONFIGS = [
    (1, 1),
    (2, 1),
    (2, 2),
    (4, 2),
    (4, 4),
]
GRID_SIZES = {1: 1024, 2: 1536, 4: 2048, 8: 3072, 16: 4096}
BASE = 1024
STEPS = 100

# def compute_size(total_cores):
#     raw = BASE * math.sqrt(total_cores)

#     # 🔥 arrondi propre
#     grid = int(round(raw / 256) * 256)
#     return grid, grid


def run_case(ranks, threads):
    total_cores = ranks * threads
    grid = GRID_SIZES[total_cores]
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(threads)

    cmd = [
        "mpirun",
        "--oversubscribe",
        "-np",
        str(ranks),
        EXEC,
        "--nx",
        str(grid),
        "--ny",
        str(grid),
        "--steps",
        str(STEPS),
        "--init",
        "1",
        "--boundary",
        "0",
    ]

    result = subprocess.run(cmd, capture_output=True, text=True, env=env)
    output = result.stdout + result.stderr

    match = re.search(r"Total:.*?([0-9]+\.[0-9]+)", output, re.DOTALL)
    if match:
        total_time = float(match.group(1))
        return grid, total_time, total_time / STEPS
    else:
        print(f"Erreur parsing ranks={ranks}, threads={threads}")
        print(output)
        return grid, None, None


def main():
    results = []

    print("Running HYBRID WEAK scaling...")

    for ranks, threads in CONFIGS:
        total_cores = ranks * threads
        print(f"Ranks={ranks}, Threads={threads} (Total={total_cores})")

        grid, total_time, time_per_iter = run_case(ranks, threads)
        results.append((total_cores, ranks, threads, grid, total_time, time_per_iter))

    os.makedirs("results", exist_ok=True)

    with open("results/hybrid_weakscaling.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(
            [
                "cores",
                "mpi_ranks",
                "omp_threads",
                "grid_size",
                "total_time",
                "time_per_iter",
            ]
        )
        writer.writerows(results)
    
    print("Saved to results/hybrid_weakscaling.csv")


if __name__ == "__main__":
    main()
