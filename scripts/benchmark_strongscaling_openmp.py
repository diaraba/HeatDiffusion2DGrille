#!/usr/bin/env python3

import subprocess
import csv
import re

EXEC = "./build/bin/heat_omp"
THREADS = [1, 2, 4, 8, 16]


def run_case(threads):
    env = {"OMP_NUM_THREADS": str(threads)}

    cmd = [
        EXEC,
        "--nx",
        "4096",
        "--ny",
        "4096",
        "--steps",
        "100",
        "--init",
        "1",
        "--boundary",
        "0",
    ]

    result = subprocess.run(cmd, capture_output=True, text=True, env=env)

    output = result.stdout

    # Extraire temps total
    match = re.search(r"Total:.*?([0-9]+\.[0-9]+)", output)
    if match:
        return float(match.group(1))
    else:
        print("Erreur parsing")
        print(output)
        return None


def main():
    results = []

    print("Running benchmarks...")

    for t in THREADS:
        print(f"Threads = {t}")
        time = run_case(t)
        results.append((t, time))

    # Calcul speedup
    T1 = results[0][1]

    rows = []
    for cores, t in results:
        speedup = T1 / t
        efficiency = speedup / cores
        rows.append([cores, t, speedup, efficiency])

    # Écriture CSV
    os.makedirs("resultats", exist_ok=True)
    with open("resultats/openmp_strongscaling_results.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["cores", "time", "speedup", "efficiency"])
        writer.writerows(rows)

    print("Résultats sauvegardés dans openmp_results.csv")


if __name__ == "__main__":
    main()
