#!/usr/bin/env python3

import subprocess
import csv
import re
import math

EXEC = "./build/bin/heat_omp"
THREADS = [1, 2, 4, 8, 16]

BASE_SIZE = 1024  # taille de base
STEPS = 100


def run_case(threads):
    env = {"OMP_NUM_THREADS": str(threads)}

    # Taille qui augmente avec sqrt(threads)
    size = int(BASE_SIZE * math.sqrt(threads))

    cmd = [
        EXEC,
        "--nx",
        str(size),
        "--ny",
        str(size),
        "--steps",
        str(STEPS),
        "--init",
        "1",
        "--boundary",
        "0",
    ]

    result = subprocess.run(cmd, capture_output=True, text=True, env=env)
    output = result.stdout

    match = re.search(r"Total:.*?([0-9]+\.[0-9]+)", output)
    if match:
        total_time = float(match.group(1))
        time_per_iter = total_time / STEPS
        return total_time, time_per_iter, size
    else:
        print("Erreur parsing")
        print(output)
        return None, None, size


def main():
    results = []

    print("Running WEAK scaling benchmarks...")

    for t in THREADS:
        print(f"Threads = {t}")
        total_time, time_per_iter, size = run_case(t)
        results.append((t, size, total_time, time_per_iter))

    # CSV (format attendu par ton plot)
    os.makedirs("resultats", exist_ok=True)

    with open("openmp_weakscaling_results.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["cores", "grid_size", "total_time", "time_per_iter"])
        writer.writerows(results)

    print("Résultats sauvegardés dans openmp_weakscaling_results.csv")


if __name__ == "__main__":
    main()
