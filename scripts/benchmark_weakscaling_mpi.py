#!/usr/bin/env python3

import subprocess
import csv
import re
import math
import os

EXEC = "./build/bin/heat_mpi"
CORES = [1, 2, 4, 8, 16]

BASE_SIZE = 1024
STEPS = 100

def run_case(cores):
    size = int(BASE_SIZE * math.sqrt(cores))

    cmd = [
        "mpirun", "-np", str(cores),
        EXEC,
        "--nx", str(size),
        "--ny", str(size),
        "--steps", str(STEPS),
        "--init", "1",
        "--boundary", "0"
    ]

    result = subprocess.run(cmd, capture_output=True, text=True)
    output = result.stdout

    match = re.search(r"Total:.*?([0-9]+\.[0-9]+)", output)

    if match:
        total_time = float(match.group(1))
        time_per_iter = total_time / STEPS
        return size, total_time, time_per_iter
    else:
        print("Erreur parsing")
        print(output)
        return size, None, None


def main():
    results = []

    print("Running MPI WEAK scaling...")

    for c in CORES:
        print(f"Cores = {c}")
        size, total_time, time_per_iter = run_case(c)
        results.append((c, size, total_time, time_per_iter))

    os.makedirs("results", exist_ok=True)

    with open("results/mpi_weakscaling.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["cores", "grid_size", "total_time", "time_per_iter"])
        writer.writerows(results)

    print("Saved to results/mpi_weakscaling.csv")


if __name__ == "__main__":
    main()