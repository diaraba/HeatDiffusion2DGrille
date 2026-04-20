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
    (8, 2),
    (8, 4),
]

BASE = 1024
STEPS = 100

def compute_size(total_cores):
    size = int(BASE * math.sqrt(total_cores))

    # ajustement simple pour divisibilité
    while size % 2 != 0:
        size += 1

    return size, size


def run_case(ranks, threads):
    total_cores = ranks * threads
    nx, ny = compute_size(total_cores)

    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(threads)

    cmd = [
        "mpirun",
        "--oversubscribe",
        "-np", str(ranks),
        EXEC,
        "--nx", str(nx),
        "--ny", str(ny),
        "--steps", str(STEPS),
        "--init", "1",
        "--boundary", "0"
    ]

    result = subprocess.run(cmd, capture_output=True, text=True, env=env)
    output = result.stdout + result.stderr

    match = re.search(r"TOTAL_TIME=([0-9]+\.[0-9]+)", output)

    if match:
        total_time = float(match.group(1))
        return nx, total_time, total_time / STEPS
    else:
        print(f"Erreur parsing ranks={ranks}, threads={threads}")
        print(output)
        return nx, None, None


def main():
    results = []

    print("Running HYBRID WEAK scaling...")

    for ranks, threads in CONFIGS:
        total_cores = ranks * threads
        print(f"Ranks={ranks}, Threads={threads} (Total={total_cores})")

        nx, total_time, time_per_iter = run_case(ranks, threads)
        results.append((total_cores, ranks, threads, nx, total_time, time_per_iter))

    os.makedirs("results", exist_ok=True)

    with open("results/hybrid_weakscaling.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["cores", "mpi_ranks", "omp_threads", "grid_size", "total_time", "time_per_iter"])
        writer.writerows(results)

    print("Saved to results/hybrid_weakscaling.csv")


if __name__ == "__main__":
    main()