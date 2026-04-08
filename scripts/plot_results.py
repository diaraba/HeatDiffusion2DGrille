#!/usr/bin/env python3
"""
Script to plot performance results from CSV files
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def plot_strong_scaling(csv_file):
    """Plot strong scaling results"""
    data = pd.read_csv(csv_file)
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
    
    # Speedup plot
    ax1.plot(data['cores'], data['speedup'], 'bo-', label='Measured')
    ax1.plot(data['cores'], data['cores'], 'k--', label='Ideal')
    ax1.set_xlabel('Number of Cores')
    ax1.set_ylabel('Speedup')
    ax1.set_title('Strong Scaling: Speedup')
    ax1.legend()
    ax1.grid(True)
    
    # Efficiency plot
    ax2.plot(data['cores'], data['efficiency'], 'ro-')
    ax2.set_xlabel('Number of Cores')
    ax2.set_ylabel('Efficiency')
    ax2.set_title('Strong Scaling: Parallel Efficiency')
    ax2.grid(True)
    
    plt.tight_layout()
    plt.savefig('strong_scaling.png', dpi=150)
    plt.show()

def plot_weak_scaling(csv_file):
    """Plot weak scaling results"""
    data = pd.read_csv(csv_file)
    
    fig, ax = plt.subplots(figsize=(8, 5))
    
    ax.plot(data['cores'], data['time_per_iter'], 'go-')
    ax.set_xlabel('Number of Cores')
    ax.set_ylabel('Time per Iteration (seconds)')
    ax.set_title('Weak Scaling: Time per Iteration')
    ax.grid(True)
    
    plt.tight_layout()
    plt.savefig('weak_scaling.png', dpi=150)
    plt.show()

def main():
    if len(sys.argv) < 2:
        print("Usage: plot_results.py <csv_file> [strong|weak]")
        return
    
    csv_file = sys.argv[1]
    if not os.path.exists(csv_file):
        print(f"File {csv_file} not found")
        return
    
    if len(sys.argv) >= 3:
        scaling_type = sys.argv[2]
        if scaling_type == 'strong':
            plot_strong_scaling(csv_file)
        elif scaling_type == 'weak':
            plot_weak_scaling(csv_file)
    else:
        # Auto-detect based on filename
        if 'strong' in csv_file.lower():
            plot_strong_scaling(csv_file)
        elif 'weak' in csv_file.lower():
            plot_weak_scaling(csv_file)
        else:
            print("Please specify scaling type: strong or weak")

if __name__ == "__main__":
    main()
