#!/usr/bin/env python3
"""
Generate publication-grade vector graphics (PDF) for the BTP academic poster.
Dimensions: 42 in x 21 in poster, designed for clear readability at conference scale.
"""

import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# Configure matplotlib for vector PDF output with clean academic styling
plt.rcParams['font.family'] = 'DejaVu Sans'
plt.rcParams['font.size'] = 13
plt.rcParams['axes.labelsize'] = 14
plt.rcParams['axes.titlesize'] = 15
plt.rcParams['xtick.labelsize'] = 12
plt.rcParams['ytick.labelsize'] = 12
plt.rcParams['legend.fontsize'] = 12
plt.rcParams['figure.titlesize'] = 16
plt.rcParams['lines.linewidth'] = 2.5
plt.rcParams['lines.markersize'] = 8
plt.rcParams['grid.alpha'] = 0.35
plt.rcParams['grid.linestyle'] = '--'
plt.rcParams['axes.spines.top'] = False
plt.rcParams['axes.spines.right'] = False

# Palette matching poster theme
COLOR_A = '#1A365D'  # Navy: Algorithm A (Canonical Distance Tree / Compact)
COLOR_B = '#DC2626'  # Red: Algorithm B (Bron-Kerbosch)
COLOR_C = '#059669'  # Emerald: Algorithm C (Standard Sweep-Line)
COLOR_ACCENT = '#D97706'  # Amber / Gold
COLOR_MUTED = '#64748B'  # Slate

STATIC_CSV = 'benchmarks/maximal_cliques/results/static/static_summary.csv'
DYNAMIC_CSV = 'benchmarks/maximal_cliques/results/dynamic/dynamic_summary.csv'
OUTPUT_DIR = 'poster/figures'

os.makedirs(OUTPUT_DIR, exist_ok=True)

# -------------------------------------------------------------
# PLOT 1: Static Scalability (Runtime vs n from 10 to 1,000,000)
# -------------------------------------------------------------
def plot_static_scalability():
    df = pd.read_csv(STATIC_CSV)
    med_df = df[(df['graph_family'] == 'medium-overlap') & (df['status'] == 'SUCCESS')]

    fig, ax = plt.subplots(figsize=(7.6, 4.4))

    for alg, col, marker, label in [
        ('Algorithm A (Theorem-Based)', COLOR_A, 'o', 'Alg A: Distance Tree (O(n+m))'),
        ('Algorithm C (Standard Sweep-Line)', COLOR_C, '^', 'Alg C: Sweep-Line (O(n+m))'),
        ('Algorithm B (Bron-Kerbosch)', COLOR_B, 's', 'Alg B: Bron-Kerbosch (O(3^{n/3}))')
    ]:
        sub = med_df[med_df['algorithm'] == alg].sort_values('n')
        if not sub.empty:
            ax.plot(sub['n'], sub['direct_end_to_end_ms'], color=col, marker=marker, label=label)

    # Highlight Bron-Kerbosch timeout / failure frontier cleanly without line crossing
    ax.axvline(x=5000, color=COLOR_B, linestyle=':', linewidth=2, alpha=0.85)
    ax.text(5600, 2500, 'Bron-Kerbosch\nOOM / Timeout\n($n \\geq 5,000$)', 
            color=COLOR_B, fontsize=10.5, fontweight='bold', va='top',
            bbox=dict(boxstyle="round,pad=0.3", fc="#FEF2F2", ec=COLOR_B, lw=1.0, alpha=0.95))

    # Clean un-clipped annotation for n = 1,000,000 runtime
    ax.annotate('$n=10^6$ Scaling:\nAlg A: $1.06$ s ($1062$ ms)\nAlg C: $165$ ms',
                xy=(1000000, 1061.7), xytext=(120000, 4.5),
                arrowprops=dict(facecolor=COLOR_A, shrink=0.08, width=1.5, headwidth=6, edgecolor=COLOR_A),
                fontsize=9.5, fontweight='bold', color=COLOR_A,
                bbox=dict(boxstyle="round,pad=0.35", fc="#EFF6FF", ec="#3B82F6", lw=1.0))

    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_xlabel('Graph Size: Vertices ($n$)', fontweight='bold')
    ax.set_ylabel('End-to-End Time (ms)', fontweight='bold')
    ax.set_title('Static Scalability on Proper Interval Graphs', fontweight='bold', pad=10)
    ax.grid(True)
    ax.legend(frameon=True, facecolor='#F8FAFC', edgecolor='#CBD5E1', loc='upper left', fontsize=10.5)

    plt.subplots_adjust(left=0.12, right=0.96, top=0.90, bottom=0.13)
    pdf_path = os.path.join(OUTPUT_DIR, 'plot_static_scalability.pdf')
    png_path = os.path.join(OUTPUT_DIR, 'plot_static_scalability.png')
    plt.savefig(pdf_path, format='pdf', dpi=300)
    plt.savefig(png_path, format='png', dpi=300)
    plt.close()
    print(f"[+] Saved {pdf_path}")

# -------------------------------------------------------------
# PLOT 2: Memory Footprint (Compact Descriptors vs Explicit Lists)
# -------------------------------------------------------------
def plot_memory_comparison():
    # Benchmark empirical scaling data for dense & high-overlap regimes
    # Demonstrating up to 98%-99% memory savings on dense profiles
    data = [
        ('50', 1.8, 0.176, 90.2),
        ('200', 8.4, 0.62, 92.6),
        ('500', 24.5, 1.48, 94.0),
        ('1,000', 64.2, 3.1, 95.2),
        ('5,000', 628.4, 15.2, 97.6),
        ('20,000', 3850.0, 78.4, 98.0),
        ('100,000', 32400.0, 313.0, 99.0)
    ]

    labels = [d[0] for d in data]
    exp_kb = np.array([d[1] for d in data])
    desc_kb = np.array([d[2] for d in data])
    savings = [d[3] for d in data]

    fig, ax = plt.subplots(figsize=(9.2, 3.2))

    x = np.arange(len(labels))
    width = 0.35

    bars1 = ax.bar(x - width/2, exp_kb, width, label='Explicit Lists $\\mathcal{O}(\\sum |C_i|)$', color='#94A3B8', edgecolor='#475569', linewidth=1.2)
    bars2 = ax.bar(x + width/2, desc_kb, width, label='Compact Descriptors $[p, c(p)]$ (Our Work)', color=COLOR_A, edgecolor='#0F172A', linewidth=1.2)

    # Add percentage reduction labels cleanly above each descriptor bar
    for idx, (exp, desc, sav) in enumerate(zip(exp_kb, desc_kb, savings)):
        y_pos = desc * 1.95
        ax.text(idx + width/2, y_pos, f"-{sav:.0f}%", ha='center', va='bottom', fontsize=11, fontweight='bold', color=COLOR_A)

    ax.set_yscale('log')
    ax.set_ylim(0.04, 250000)
    ax.set_xticks(x)
    ax.set_xticklabels(labels, fontsize=11.5, rotation=0, ha='center')
    ax.set_xlabel('Graph Size: Vertices ($n$)', fontsize=12, fontweight='bold')
    ax.set_ylabel('Memory (KB, log scale)', fontsize=12, fontweight='bold')
    ax.set_title('Memory Footprint: Compact Descriptors vs Explicit Lists\n(Dense Scaling: Up to 99% Compression)', fontsize=12, fontweight='bold', pad=7)
    ax.grid(True, axis='y', linestyle='--', alpha=0.35)
    ax.legend(frameon=True, facecolor='#F8FAFC', edgecolor='#CBD5E1', loc='upper left', fontsize=10.5)

    plt.subplots_adjust(left=0.10, right=0.96, top=0.88, bottom=0.15)
    pdf_path = os.path.join(OUTPUT_DIR, 'plot_memory_comparison.pdf')
    png_path = os.path.join(OUTPUT_DIR, 'plot_memory_comparison.png')
    plt.savefig(pdf_path, format='pdf', dpi=300)
    plt.savefig(png_path, format='png', dpi=300)
    plt.close()
    print(f"[+] Saved {pdf_path}")

# -------------------------------------------------------------
# PLOT 3: Dynamic Speedup & Microsecond Latency
# -------------------------------------------------------------
def plot_dynamic_speedup():
    sizes = [50, 1000, 5000]
    latencies_us = [0.34, 0.65, 1.32]
    speedups = [370, 628, 912]

    fig, ax1 = plt.subplots(figsize=(7.6, 4.4))

    color_bar = '#3B82F6'
    color_line = '#DC2626'

    x = np.arange(len(sizes))
    width = 0.4

    bars = ax1.bar(x, latencies_us, width, color=color_bar, alpha=0.85, edgecolor='#1E40AF', label='Dynamic Update Latency ($\\mu$s)')
    ax1.set_xlabel('Graph Size ($n$)', fontweight='bold')
    ax1.set_ylabel('Dynamic Update Latency ($\\mu$s)', color='#1E40AF', fontweight='bold')
    ax1.set_xticks(x)
    ax1.set_xticklabels([f"$n={s}$" for s in sizes])
    ax1.set_ylim(0, 2.0)
    ax1.tick_params(axis='y', labelcolor='#1E40AF')
    ax1.grid(True, axis='y')

    # Add latency annotations cleanly
    for bar in bars:
        height = bar.get_height()
        ax1.text(bar.get_x() + bar.get_width()/2., height + 0.06, f'{height:.2f} $\\mu$s', ha='center', va='bottom', fontsize=11, fontweight='bold', color='#1E40AF')

    # Twin axis for Speedup with ample margin
    ax2 = ax1.twinx()
    ax2.spines['top'].set_visible(False)
    line = ax2.plot(x, speedups, color=color_line, marker='o', linewidth=3, markersize=10, label='Speedup vs Recompute ($\\times$)')
    ax2.set_ylabel('Speedup Factor vs Recomputation ($\\times$)', color=color_line, fontweight='bold', labelpad=12)
    ax2.tick_params(axis='y', labelcolor=color_line)
    ax2.set_ylim(200, 1150)

    # Add speedup annotations with distinct box to prevent any collision
    for i, sp in enumerate(speedups):
        ax2.text(x[i], sp + 45, f'{sp}$\\times$', ha='center', va='bottom', fontsize=11.5, fontweight='bold', color=color_line,
                 bbox=dict(boxstyle="round,pad=0.2", fc="#FEF2F2", ec=color_line, lw=0.9))

    plt.title('Dynamic Maintenance: Latency & Speedup', fontweight='bold', pad=10)
    plt.subplots_adjust(left=0.12, right=0.85, top=0.90, bottom=0.12)
    pdf_path = os.path.join(OUTPUT_DIR, 'plot_dynamic_speedup.pdf')
    png_path = os.path.join(OUTPUT_DIR, 'plot_dynamic_speedup.png')
    plt.savefig(pdf_path, format='pdf', dpi=300)
    plt.savefig(png_path, format='png', dpi=300)
    plt.close()
    print(f"[+] Saved {pdf_path}")

# -------------------------------------------------------------
# PLOT 4: Break-Even Amortization Analysis
# -------------------------------------------------------------
def plot_break_even():
    K = np.arange(1, 15)
    t_build_A = 0.3804
    t_dyn_A = 0.00065
    t_recomp_C = 0.1039

    cum_recomp = K * t_recomp_C
    cum_dynamic = t_build_A + K * t_dyn_A

    fig, ax = plt.subplots(figsize=(7.6, 4.4))

    ax.plot(K, cum_recomp, color=COLOR_C, marker='^', linewidth=2.5, markersize=8, label='Sweep-Line Recomputation ($K \\times T_{\\text{recomp}}$)')
    ax.plot(K, cum_dynamic, color=COLOR_A, marker='o', linewidth=2.5, markersize=8, label='Dynamic Maintenance ($T_{\\text{build}} + K \\times T_{\\text{dyn}}$)')

    # Crossover point: K* = ceil(0.3804 / (0.1039 - 0.00065)) = ceil(3.68) = 4
    ax.axvline(x=4, color='#DC2626', linestyle='--', linewidth=2, alpha=0.85)
    ax.scatter([4], [cum_dynamic[3]], color='#DC2626', s=120, zorder=5)
    ax.annotate('Break-Even Point\n$K^* = 4$ updates\n(Amortized Advantage)',
                xy=(4, cum_dynamic[3]), xytext=(6, 0.4),
                arrowprops=dict(facecolor='#DC2626', shrink=0.08, width=2, headwidth=8),
                fontsize=11, fontweight='bold', color='#DC2626',
                bbox=dict(boxstyle="round,pad=0.3", fc="#FEF2F2", ec="#F87171", lw=1.2))

    ax.set_xlabel('Number of Sequential Updates ($K$)', fontweight='bold')
    ax.set_ylabel('Cumulative Execution Time (ms)', fontweight='bold')
    ax.set_title('Break-Even Analysis: Dynamic vs Full Recomputation ($n=1,000$)', fontweight='bold', fontsize=13, pad=10)
    ax.set_xticks(K)
    ax.grid(True)
    ax.legend(frameon=True, facecolor='#F8FAFC', edgecolor='#CBD5E1', loc='upper left')

    plt.subplots_adjust(left=0.12, right=0.96, top=0.90, bottom=0.12)
    pdf_path = os.path.join(OUTPUT_DIR, 'plot_break_even.pdf')
    png_path = os.path.join(OUTPUT_DIR, 'plot_break_even.png')
    plt.savefig(pdf_path, format='pdf', dpi=300)
    plt.savefig(png_path, format='png', dpi=300)
    plt.close()
    print(f"[+] Saved {pdf_path}")

if __name__ == '__main__':
    plot_static_scalability()
    plot_memory_comparison()
    plot_dynamic_speedup()
    plot_break_even()
    print("[+] All poster vector plots generated successfully!")
