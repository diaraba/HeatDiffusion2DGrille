#!/usr/bin/env python3

import subprocess
import csv
import re

EXEC = "./build/bin/heat_mpi"
CORES = [1, 2, 4, 8, 16, 32]

NX = 8192
NY = 8192
STEPS = 100


def run_case(cores):
    cmd = [
        "mpirun",
        "-np",
        str(cores),
        EXEC,
        "--nx",
        str(NX),
        "--ny",
        str(NY),
        "--steps",
        str(STEPS),
        "--init",
        "1",
        "--boundary",
        "0",
    ]

    result = subprocess.run(cmd, capture_output=True, text=True)
    output = result.stdout

    match = re.search(r"Total:.*?([0-9]+\.[0-9]+)", output)

    if match:
        return float(match.group(1))
    else:
        print("Erreur parsing")
        print(output)
        return None


def main():
    results = []

    print("Running MPI STRONG scaling...")

    for c in CORES:
        print(f"Cores = {c}")
        t = run_case(c)
        results.append((c, t))

    T1 = results[0][1]

    rows = []
    for cores, t in results:
        speedup = T1 / t
        efficiency = speedup / cores
        rows.append([cores, t, speedup, efficiency])

    import os

    os.makedirs("results", exist_ok=True)

    with open("results/mpi_strongscaling.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["cores", "time", "speedup", "efficiency"])
        writer.writerows(rows)

    print("Saved to results/mpi_strongscaling.csv")


if __name__ == "__main__":
    main()
