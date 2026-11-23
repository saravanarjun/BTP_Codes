#!/usr/bin/env python3
"""
Publication-Quality Plotting Module for BTP Maximal Clique Benchmarking
Covers both Static and Dynamic experiments across all 14 graph families and workloads.
"""

import os
import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# Style configuration
plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['font.size'] = 10
plt.rcParams['axes.labelsize'] = 11
plt.rcParams['axes.titlesize'] = 12
plt.rcParams['xtick.labelsize'] = 9
plt.rcParams['ytick.labelsize'] = 9
plt.rcParams['legend.fontsize'] = 9
plt.rcParams['figure.titlesize'] = 13
plt.rcParams['lines.linewidth'] = 1.8
plt.rcParams['lines.markersize'] = 6
plt.rcParams['grid.alpha'] = 0.4
plt.rcParams['grid.linestyle'] = '--'

COLOR_A = '#1f77b4'  # Blue: Algorithm A (Theorem-based)
COLOR_B = '#d62728'  # Red: Algorithm B (Bron-Kerbosch)
COLOR_C = '#2ca02c'  # Green: Algorithm C (Standard Sweep-Line)

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)

def plot_static_benchmarks(results_csv, plots_dir):
    ensure_dir(plots_dir)
    if not os.path.exists(results_csv):
        print(f"[-] Static CSV not found: {results_csv}")
        return

    df = pd.read_csv(results_csv)

    # 1. End-to-End Runtime vs n (Medium Overlap)
    med_df = df[(df['graph_family'] == 'medium-overlap') & (df['status'] == 'SUCCESS')]
    plt.figure(figsize=(8, 5))
    for alg, color, marker in [('Algorithm A (Theorem-Based)', COLOR_A, 'o'),
                               ('Algorithm B (Bron-Kerbosch)', COLOR_B, 's'),
                               ('Algorithm C (Standard Sweep-Line)', COLOR_C, '^')]:
        sub = med_df[med_df['algorithm'] == alg]
        if not sub.empty:
            plt.plot(sub['n'], sub['direct_end_to_end_ms'], label=alg, color=color, marker=marker)

    # Plot timeout indication for Alg B
    b_timeouts = df[(df['graph_family'] == 'medium-overlap') & (df['algorithm'].str.contains('Bron-Kerbosch')) & (df['status'] == 'TIMEOUT')]
    if not b_timeouts.empty:
        plt.axvline(x=b_timeouts['n'].min(), color=COLOR_B, linestyle=':', alpha=0.7, label='Alg B Timeout Frontier (n>=5000)')

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Number of Vertices (n)')
    plt.ylabel('Direct End-to-End Time (ms)')
    plt.title('Plot 1: Static End-to-End Runtime vs. Graph Order n (Medium Overlap)')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(plots_dir, 'runtime_vs_n.png'), dpi=300)
    plt.close()

    # 2. Core Identification Runtime vs n
    plt.figure(figsize=(8, 5))
    for alg, color, marker in [('Algorithm A (Theorem-Based)', COLOR_A, 'o'),
                               ('Algorithm C (Standard Sweep-Line)', COLOR_C, '^')]:
        sub = med_df[med_df['algorithm'] == alg]
        if not sub.empty:
            plt.plot(sub['n'], sub['core_ms'], label=f'{alg} (Core)', color=color, marker=marker)

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Number of Vertices (n)')
    plt.ylabel('Core Identification Time (ms)')
    plt.title('Plot 2: Core Clique Identification Time vs. Graph Order n')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(plots_dir, 'core_runtime_vs_n.png'), dpi=300)
    plt.close()

    # 3. Preprocessing Runtime vs n
    plt.figure(figsize=(8, 5))
    for alg, color, marker in [('Algorithm A (Theorem-Based)', COLOR_A, 'o'),
                               ('Algorithm C (Standard Sweep-Line)', COLOR_C, '^')]:
        sub = med_df[med_df['algorithm'] == alg]
        if not sub.empty:
            label = 'Alg A (Distance Tree Construction)' if 'A' in alg else 'Alg C (Endpoint Event Sorting)'
            plt.plot(sub['n'], sub['preprocessing_ms'], label=label, color=color, marker=marker)

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Number of Vertices (n)')
    plt.ylabel('Preprocessing Time (ms)')
    plt.title('Plot 3: Preprocessing Time vs. Graph Order n')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(plots_dir, 'preprocessing_vs_n.png'), dpi=300)
    plt.close()

    # 4. Materialization Overhead vs n
    plt.figure(figsize=(8, 5))
    for alg, color, marker in [('Algorithm A (Theorem-Based)', COLOR_A, 'o'),
                               ('Algorithm C (Standard Sweep-Line)', COLOR_C, '^')]:
        sub = med_df[med_df['algorithm'] == alg]
        if not sub.empty:
            plt.plot(sub['n'], sub['materialization_ms'], label=f'{alg} (Materialization)', color=color, marker=marker)

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Number of Vertices (n)')
    plt.ylabel('Materialization Time (ms)')
    plt.title('Plot 4: Explicit Clique Materialization Overhead vs. Graph Order n')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(plots_dir, 'materialization_vs_n.png'), dpi=300)
    plt.close()

    # 5. Runtime vs Number of Edges (m)
    plt.figure(figsize=(8, 5))
    for alg, color, marker in [('Algorithm A (Theorem-Based)', COLOR_A, 'o'),
                               ('Algorithm B (Bron-Kerbosch)', COLOR_B, 's'),
                               ('Algorithm C (Standard Sweep-Line)', COLOR_C, '^')]:
        sub = med_df[med_df['algorithm'] == alg]
        if not sub.empty:
            plt.plot(sub['m'], sub['direct_end_to_end_ms'], label=alg, color=color, marker=marker)

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Number of Edges (m)')
    plt.ylabel('Direct End-to-End Time (ms)')
    plt.title('Plot 5: Runtime vs. Number of Edges (m)')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(plots_dir, 'runtime_vs_edges.png'), dpi=300)
    plt.close()

    # 6. Runtime vs Total Clique Memberships (S)
    plt.figure(figsize=(8, 5))
    for alg, color, marker in [('Algorithm A (Theorem-Based)', COLOR_A, 'o'),
                               ('Algorithm C (Standard Sweep-Line)', COLOR_C, '^')]:
        sub = med_df[med_df['algorithm'] == alg]
        if not sub.empty:
            plt.plot(sub['total_memberships'], sub['direct_end_to_end_ms'], label=alg, color=color, marker=marker)

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Total Clique Memberships S = sum |C_j|')
    plt.ylabel('Direct End-to-End Time (ms)')
    plt.title('Plot 6: Runtime vs. Total Output Size S')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(plots_dir, 'runtime_vs_memberships.png'), dpi=300)
    plt.close()

    # 7. Families Comparison (at n = 1000)
    fam_1000 = df[(df['n'] == 1000) & (df['status'] == 'SUCCESS')]
    if not fam_1000.empty:
        plt.figure(figsize=(10, 5))
        families = fam_1000['graph_family'].unique()
        x = np.arange(len(families))
        width = 0.35

        a_times = []
        c_times = []
        valid_fams = []
        for f in families:
            sub_a = fam_1000[(fam_1000['graph_family'] == f) & (fam_1000['algorithm'].str.contains('Theorem'))]
            sub_c = fam_1000[(fam_1000['graph_family'] == f) & (fam_1000['algorithm'].str.contains('Sweep'))]
            if not sub_a.empty and not sub_c.empty:
                a_times.append(sub_a['direct_end_to_end_ms'].values[0])
                c_times.append(sub_c['direct_end_to_end_ms'].values[0])
                valid_fams.append(f)

        if valid_fams:
            x = np.arange(len(valid_fams))
            plt.bar(x - width/2, a_times, width, label='Algorithm A (Theorem-Based)', color=COLOR_A)
            plt.bar(x + width/2, c_times, width, label='Algorithm C (Standard Sweep-Line)', color=COLOR_C)
            plt.xticks(x, valid_fams, rotation=35, ha='right')
            plt.ylabel('Direct End-to-End Time (ms)')
            plt.title('Plot 7: Algorithmic Runtime Across Graph Families (n = 1,000)')
            plt.grid(axis='y')
            plt.legend()
            plt.tight_layout()
            plt.savefig(os.path.join(plots_dir, 'families_comparison.png'), dpi=300)
            plt.close()

    # 8. Memory Storage: Compact Descriptors vs. Explicit Memberships
    plt.figure(figsize=(8, 5))
    sub = med_df[med_df['algorithm'].str.contains('Theorem')]
    if not sub.empty:
        plt.plot(sub['n'], sub['estimated_descriptor_bytes'] / 1024.0, label='Compact Descriptors O(q) (KB)', color='#2ca02c', marker='o')
        plt.plot(sub['n'], sub['estimated_explicit_bytes'] / 1024.0, label='Explicit Memberships O(S) (KB)', color='#d62728', marker='s')
        plt.xscale('log')
        plt.yscale('log')
        plt.xlabel('Number of Vertices (n)')
        plt.ylabel('Estimated Memory (KB)')
        plt.title('Plot 8: Memory Footprint: Compact Representation vs. Explicit Storage')
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        plt.savefig(os.path.join(plots_dir, 'memory_comparison.png'), dpi=300)
        plt.close()

    # 9. Scalability Frontier up to max n
    plt.figure(figsize=(8, 5))
    for alg, color, marker in [('Algorithm A (Theorem-Based)', COLOR_A, 'o'),
                               ('Algorithm C (Standard Sweep-Line)', COLOR_C, '^')]:
        sub = med_df[med_df['algorithm'] == alg]
        if not sub.empty:
            plt.plot(sub['n'], sub['direct_end_to_end_ms'], label=alg, color=color, marker=marker)

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Number of Vertices (n)')
    plt.ylabel('Direct End-to-End Runtime (ms)')
    plt.title('Plot 9: Macro-Scale Scalability Frontier (up to n = 1,000,000)')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(plots_dir, 'scalability_comparison.png'), dpi=300)
    plt.close()

    print("[+] Static benchmark plots successfully generated.")

def plot_dynamic_benchmarks(dyn_summary_csv, ins_csv, del_csv, mix_csv, plots_dir):
    ensure_dir(plots_dir)
    ins_dir = os.path.join(plots_dir, 'insertion')
    del_dir = os.path.join(plots_dir, 'deletion')
    mix_dir = os.path.join(plots_dir, 'mixed')
    ensure_dir(ins_dir)
    ensure_dir(del_dir)
    ensure_dir(mix_dir)

    if not os.path.exists(dyn_summary_csv):
        print(f"[-] Dynamic summary CSV not found: {dyn_summary_csv}")
        return

    sum_df = pd.read_csv(dyn_summary_csv)

    # 1. Insertion Latency vs initial_n
    ins_sum = sum_df[sum_df['workload'].str.contains('insertion', case=False) & (sum_df['status'] == 'SUCCESS')]
    if not ins_sum.empty:
        plt.figure(figsize=(8, 5))
        for alg, color, marker in [('Algorithm A (Thesis-Based)', COLOR_A, 'o'),
                                   ('Algorithm B (Bron-Kerbosch)', COLOR_B, 's'),
                                   ('Algorithm C (Sweep-Line)', COLOR_C, '^')]:
            sub = ins_sum[ins_sum['algorithm'] == alg]
            if not sub.empty:
                plt.plot(sub['initial_n'], sub['average_latency_ms'], label=f'{alg} (Avg Update Latency)', color=color, marker=marker)

        plt.xscale('log')
        plt.yscale('log')
        plt.xlabel('Initial Graph Size (n)')
        plt.ylabel('Average Update Latency (ms)')
        plt.title('Dynamic Plot 1: Insertion Latency vs. Graph Size')
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        plt.savefig(os.path.join(ins_dir, 'insertion_latency_vs_n.png'), dpi=300)
        plt.close()

    # 2. Deletion Latency vs initial_n
    del_sum = sum_df[sum_df['workload'].str.contains('deletion', case=False) & (sum_df['status'] == 'SUCCESS')]
    if not del_sum.empty:
        plt.figure(figsize=(8, 5))
        for alg, color, marker in [('Algorithm A (Thesis-Based)', COLOR_A, 'o'),
                                   ('Algorithm B (Bron-Kerbosch)', COLOR_B, 's'),
                                   ('Algorithm C (Sweep-Line)', COLOR_C, '^')]:
            sub = del_sum[del_sum['algorithm'] == alg]
            if not sub.empty:
                plt.plot(sub['initial_n'], sub['average_latency_ms'], label=f'{alg} (Avg Update Latency)', color=color, marker=marker)

        plt.xscale('log')
        plt.yscale('log')
        plt.xlabel('Initial Graph Size (n)')
        plt.ylabel('Average Update Latency (ms)')
        plt.title('Dynamic Plot 2: Deletion Latency vs. Graph Size')
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        plt.savefig(os.path.join(del_dir, 'deletion_latency_vs_n.png'), dpi=300)
        plt.close()

    # 3. Mixed-Update Latency across Ratios (75/25, 50/50, 25/75)
    mix_sum = sum_df[sum_df['workload'].str.contains('mixed', case=False) & (sum_df['status'] == 'SUCCESS')]
    if not mix_sum.empty:
        plt.figure(figsize=(8, 5))
        sub_1000 = mix_sum[mix_sum['initial_n'] == 1000]
        if not sub_1000.empty:
            for alg, color, marker in [('Algorithm A (Thesis-Based)', COLOR_A, 'o'),
                                       ('Algorithm C (Sweep-Line)', COLOR_C, '^')]:
                sub = sub_1000[sub_1000['algorithm'] == alg]
                if not sub.empty:
                    plt.plot(sub['workload'], sub['average_latency_ms'], label=alg, color=color, marker=marker)

            plt.ylabel('Average Update Latency (ms)')
            plt.title('Dynamic Plot 3: Update Latency Across Mixed Workload Ratios (n = 1,000)')
            plt.grid(True)
            plt.legend()
            plt.tight_layout()
            plt.savefig(os.path.join(mix_dir, 'ratios_comparison.png'), dpi=300)
            plt.close()

    # 4. Cumulative Update Time vs Steps
    if os.path.exists(ins_csv):
        step_df = pd.read_csv(ins_csv)
        sub_step = step_df[(step_df['initial_n'] == 1000) & (step_df['status'] == 'SUCCESS')]
        if not sub_step.empty:
            plt.figure(figsize=(8, 5))
            for alg, color in [('Algorithm A (Thesis-Based)', COLOR_A),
                               ('Algorithm C (Sweep-Line)', COLOR_C)]:
                sub = sub_step[sub_step['algorithm'] == alg]
                if not sub.empty:
                    cum = sub['total_latency_ms'].cumsum()
                    plt.plot(sub['update_index'], cum, label=alg, color=color)

            plt.xlabel('Update Step Index')
            plt.ylabel('Cumulative Time (ms)')
            plt.title('Dynamic Plot 4: Cumulative Update Time over 50 Sequential Insertions (n=1000)')
            plt.grid(True)
            plt.legend()
            plt.tight_layout()
            plt.savefig(os.path.join(plots_dir, 'cumulative_update_time.png'), dpi=300)
            plt.close()

    # 5. Dynamic Speedup: Alg A Maintenance vs Full Recomputation (Sweep & BK)
    if not ins_sum.empty:
        plt.figure(figsize=(8, 5))
        ns = sorted(ins_sum['initial_n'].unique())
        speedup_c = []
        valid_ns = []
        for n in ns:
            sub_a = ins_sum[(ins_sum['initial_n'] == n) & (ins_sum['algorithm'].str.contains('Thesis'))]
            sub_c = ins_sum[(ins_sum['initial_n'] == n) & (ins_sum['algorithm'].str.contains('Sweep'))]
            if not sub_a.empty and not sub_c.empty:
                t_a = sub_a['average_latency_ms'].values[0]
                t_c = sub_c['average_latency_ms'].values[0]
                if t_a > 0:
                    speedup_c.append(t_c / t_a)
                    valid_ns.append(n)

        if valid_ns:
            plt.plot(valid_ns, speedup_c, label='Speedup: Sweep-Full-Recomputation / Alg A Maintenance', color='#ff7f0e', marker='D')
            plt.xscale('log')
            plt.xlabel('Graph Size (n)')
            plt.ylabel('Relative Speedup Factor (x)')
            plt.title('Dynamic Plot 5: Speedup Factor: Alg A Dynamic Maintenance vs. Full Recomputation')
            plt.grid(True)
            plt.legend()
            plt.tight_layout()
            plt.savefig(os.path.join(plots_dir, 'dynamic_speedup_comparison.png'), dpi=300)
            plt.close()

    # 6. Descriptors Modified vs Step
    if os.path.exists(ins_csv):
        step_df = pd.read_csv(ins_csv)
        sub_a = step_df[(step_df['initial_n'] == 1000) & (step_df['algorithm'].str.contains('Thesis'))]
        if not sub_a.empty:
            plt.figure(figsize=(8, 5))
            plt.plot(sub_a['update_index'], sub_a['descriptors_modified'], label='Descriptors Modified per Step', color='#9467bd', marker='s')
            plt.xlabel('Update Step Index')
            plt.ylabel('Number of Descriptors Modified')
            plt.ylim(0, 3)
            plt.title('Dynamic Plot 6: Locality of Updates: Descriptors Modified per Insertion')
            plt.grid(True)
            plt.legend()
            plt.tight_layout()
            plt.savefig(os.path.join(plots_dir, 'descriptor_modifications.png'), dpi=300)
            plt.close()

    print("[+] Dynamic benchmark plots successfully generated.")

def plot_combined_overview(static_csv, dynamic_csv, plots_dir):
    ensure_dir(plots_dir)
    comb_dir = os.path.join(plots_dir, 'combined')
    ensure_dir(comb_dir)

    if not os.path.exists(static_csv) or not os.path.exists(dynamic_csv):
        return

    st_df = pd.read_csv(static_csv)
    dyn_df = pd.read_csv(dynamic_csv)

    # Static vs Dynamic Latency comparison
    med_st = st_df[(st_df['graph_family'] == 'medium-overlap') & (st_df['status'] == 'SUCCESS')]
    ins_dyn = dyn_df[dyn_df['workload'].str.contains('insertion', case=False) & (dyn_df['status'] == 'SUCCESS')]

    plt.figure(figsize=(8, 5))
    sub_st_a = med_st[med_st['algorithm'].str.contains('Theorem')]
    if not sub_st_a.empty:
        plt.plot(sub_st_a['n'], sub_st_a['direct_end_to_end_ms'], label='Static Alg A (Recompute from scratch)', color=COLOR_A, linestyle='--', marker='o')

    sub_dyn_a = ins_dyn[ins_dyn['algorithm'].str.contains('Thesis')]
    if not sub_dyn_a.empty:
        plt.plot(sub_dyn_a['initial_n'], sub_dyn_a['average_latency_ms'], label='Dynamic Alg A (Incremental Update)', color='#2ca02c', marker='^')

    sub_dyn_c = ins_dyn[ins_dyn['algorithm'].str.contains('Sweep')]
    if not sub_dyn_c.empty:
        plt.plot(sub_dyn_c['initial_n'], sub_dyn_c['average_latency_ms'], label='Sweep-Line (Full Recomputation per update)', color='#ff7f0e', linestyle=':', marker='s')

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Graph Size (n)')
    plt.ylabel('Time (ms)')
    plt.title('Combined Plot 1: Static Recomputation vs. Dynamic Maintenance')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(comb_dir, 'static_vs_dynamic.png'), dpi=300)
    plt.close()
    print("[+] Combined plots successfully generated.")

if __name__ == '__main__':
    base_dir = os.path.dirname(os.path.abspath(__file__))
    static_csv = os.path.join(base_dir, 'results', 'static', 'static_results.csv')
    dyn_sum_csv = os.path.join(base_dir, 'results', 'dynamic', 'dynamic_summary.csv')
    ins_csv = os.path.join(base_dir, 'results', 'dynamic', 'insertion', 'dynamic_insertion_results.csv')
    del_csv = os.path.join(base_dir, 'results', 'dynamic', 'deletion', 'dynamic_deletion_results.csv')
    mix_csv = os.path.join(base_dir, 'results', 'dynamic', 'mixed', 'dynamic_mixed_results.csv')

    plots_static = os.path.join(base_dir, 'plots', 'static')
    plots_dynamic = os.path.join(base_dir, 'plots', 'dynamic')
    plots_combined = os.path.join(base_dir, 'plots', 'combined')

    print("[*] Generating static plots...")
    plot_static_benchmarks(static_csv, plots_static)

    print("[*] Generating dynamic plots...")
    plot_dynamic_benchmarks(dyn_sum_csv, ins_csv, del_csv, mix_csv, plots_dynamic)

    print("[*] Generating combined plots...")
    plot_combined_overview(static_csv, dyn_sum_csv, os.path.join(base_dir, 'plots'))
