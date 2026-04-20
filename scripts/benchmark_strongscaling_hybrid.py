#!/usr/bin/env python3

import subprocess
import csv
import re
import os

EXEC = "./build/bin/heat_hybrid"

# (MPI ranks, threads)
CONFIGS = [
    (1, 1),
    (2, 1),
    (2, 2),
    (4, 2),
    (4, 4),
]

NX = 4096
NY = 4096
STEPS = 100

def run_case(ranks, threads):
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(threads)

    cmd = [
        "mpirun",
        "--oversubscribe",
        "-np", str(ranks),
        EXEC,
        "--nx", str(NX),
        "--ny", str(NY),
        "--steps", str(STEPS),
        "--init", "1",
        "--boundary", "0"
    ]

    result = subprocess.run(cmd, capture_output=True, text=True, env=env)
    output = result.stdout + result.stderr

    match = re.search(r"Total:([0-9]+\.[0-9]+)", output)

    if match:
        return float(match.group(1))
    else:
        print(f"Erreur parsing ranks={ranks}, threads={threads}")
        print(output)
        return None


def main():
    results = []

    print("Running HYBRID STRONG scaling...")

    for ranks, threads in CONFIGS:
        total_cores = ranks * threads
        print(f"Ranks={ranks}, Threads={threads} (Total={total_cores})")

        t = run_case(ranks, threads)
        results.append((total_cores, ranks, threads, t))

    # référence = premier cas
    T1 = results[0][3]

    rows = []
    for cores, ranks, threads, t in results:
        if t is None:
            continue

        speedup = T1 / t
        efficiency = speedup / cores

        rows.append([cores, ranks, threads, t, speedup, efficiency])

    os.makedirs("results", exist_ok=True)

    with open("results/hybrid_strongscaling.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["cores", "mpi_ranks", "omp_threads", "time", "speedup", "efficiency"])
        writer.writerows(rows)

    print("Saved to results/hybrid_strongscaling.csv")


if __name__ == "__main__":
    main()