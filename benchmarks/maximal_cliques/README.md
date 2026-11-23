# Maximal Clique Experimental Evaluation Module

**Project**: Dynamic Graph Algorithms — Interval Graphs  
**Directory**: `benchmarks/maximal_cliques/`  

---

## 1. Overview

This module provides an experimental benchmarking and verification harness comparing three maximal clique algorithms on connected proper interval graphs across both **static** and **dynamic** settings:

- **Algorithm A (Theorem-Based / Thesis Formulation)**:
  - *Static*: Computes compact maximal clique descriptors $[l_p, l_{c_p}]$ for non-leaf parent nodes $p$ in the distance tree $T(G)$ using Lemma 3.8 and Lemma 3.15.
  - *Dynamic*: Maintains $T(G)$ and compact descriptors under Section 3.10.2 leaf insertions ($\mathcal{O}(\log n)$ parent lookup + $\mathcal{O}(1)$ descriptor update) and Section 3.10.3 decremental deletions ($\mathcal{O}(1)$ child-set relocation under immediate right neighbor $v$ via Theorem 3.16).
- **Algorithm B (Pivoted Bron–Kerbosch)**:
  - Recursive maximal clique backtracking with Tomita degree-maximizing pivot selection. Evaluated dynamically via full recomputation (`BK-Full-Recomputation`) with a 3,000 ms wall-clock timeout.
- **Algorithm C (Standard Interval Sweep-Line)**:
  - Helly property endpoint event sorting followed by active interval sweep. Evaluated dynamically via full recomputation (`Sweep-Full-Recomputation`).

---

## 2. Directory Structure

```text
benchmarks/maximal_cliques/
├── CMakeLists.txt (integrated in root CMakeLists.txt)
├── README.md                          # Build and execution guide (this file)
├── AUDIT_REPORT.md                    # Formal mathematical foundations, proof gaps, and code audit
├── BENCHMARK_METHODOLOGY.md           # Formal experimental protocol and parameters
├── BENCHMARK_REPORT.md                # Quantitative report answering all 11 core research questions
├── plot_benchmarks.py                 # Automated publication-quality plotting script
│
├── clique_types.h                     # Data structs, descriptors, and telemetry records
├── graph_generator.h / .cpp           # 14 synthetic and adversarial graph family generators
├── standard_interval_cliques.h / .cpp # Algorithm C: Helly property endpoint sweep-line
├── bron_kerbosch_adapter.h / .cpp     # Algorithm B: Pivoted Bron-Kerbosch with timeout
├── theorem_cliques_adapter.h / .cpp   # Algorithm A: Lemma 3.8 & Lemma 3.15 descriptor logic
├── dynamic_clique_structure.h / .cpp  # Dynamic structures (maintenance vs full recomputation)
├── dynamic_workload_generator.h / .cpp# Workloads A (insertions), B (deletions), and C (mixed)
├── clique_verifier.h / .cpp           # 77-test verification harness (3-way check, maximality)
├── benchmark_runner.h / .cpp          # Static and dynamic benchmark execution drivers
├── benchmark_main.cpp                 # CLI entry point
│
├── results/
│   ├── verification/
│   │   └── verification_results.csv   # 77-test verification log (100% clean)
│   ├── static/
│   │   ├── static_results.csv         # Full static timing breakdown across all runs
│   │   └── static_summary.csv         # Median aggregates across graph sizes and families
│   └── dynamic/
│       ├── insertion/
│       │   └── dynamic_insertion_results.csv # Step-by-step latency & descriptor telemetry
│       ├── deletion/
│       │   └── dynamic_deletion_results.csv  # Decremental deletion step telemetry
│       ├── mixed/
│       │   └── dynamic_mixed_results.csv     # Mixed workload step telemetry (75/25, 50/50, 25/75)
│       └── dynamic_summary.csv        # Aggregated dynamic update latency & speedup summary
│
├── plots/
│   ├── static/                        # 9 static scaling, core runtime, and memory plots
│   ├── dynamic/                       # 6 dynamic update latency and speedup plots
│   └── combined/                      # Combined static vs dynamic comparison plots
│
└── legacy_static_benchmark/           # Archived previous static benchmark results & reports
```

---

## 3. Building the Benchmark Suite

The module is built as part of the primary CMake project:

```powershell
# From the repository root (C:/Users/sarav/OneDrive/Desktop/BTP_Codes):
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target clique_benchmark
```

The resulting executable will be located at:
`build/Release/clique_benchmark.exe`

---

## 4. Running the Benchmark Suite

The executable supports multiple execution modes via command-line flags:

### 4.1 Complete Verification Suite (`--verify`)
Runs the comprehensive 77-test verification suite covering:
- Static 3-way equivalence ($A == B == C$) across all 14 graph families at $n \in \{10, 20, 50, 100\}$.
- Pairwise clique property check (every pair adjacent).
- Maximality check (no vertex in $V \setminus C$ can be added).
- Step-by-step dynamic cross-verification against dynamic sweep-line, Bron-Kerbosch, and static tree rebuilds.

```powershell
.\build\Release\clique_benchmark.exe --verify
```

### 4.2 Fast Sanity Check Mode (`--smoke`)
Runs the full verification suite followed by scaled-down static and dynamic benchmarks (up to $n = 1,000$ in $< 30$ seconds):

```powershell
.\build\Release\clique_benchmark.exe --smoke
```

### 4.3 Full Experimental Benchmark Suite (`--benchmark` or `--all`)
Executes the full static benchmark (up to $n = 1,000,000$) and dynamic benchmark suite (up to $n = 5,000$ across incremental, decremental, and mixed workloads):

```powershell
.\build\Release\clique_benchmark.exe --benchmark
```

### 4.4 Granular Execution Modes
```powershell
# Static benchmarks only:
.\build\Release\clique_benchmark.exe --static-only

# Dynamic benchmarks only:
.\build\Release\clique_benchmark.exe --dynamic-only
```

---

## 5. Generating Publication-Quality Plots

To regenerate all 15 figures in `plots/` from the CSV outputs:

```powershell
python benchmarks/maximal_cliques/plot_benchmarks.py
```

Generated plots will be saved to `plots/static/`, `plots/dynamic/`, and `plots/combined/`.

---

## 6. Final Research Audit Package (`FINAL_RESEARCH_AUDIT/`)

For complete mathematical derivations, formal proofs, implementation audits, and verified experimental reports, see the dedicated package in `benchmarks/maximal_cliques/FINAL_RESEARCH_AUDIT/`:
- `FINAL_MATHEMATICAL_PROOFS.md`: Complete static maximal clique theorem proofs, Case B deletion counterexample, rightmost leaf insertion invariant, and sweep-line correctness.
- `FINAL_IMPLEMENTATION_REPORT.md`: Architectural details and code audit of the localized $\mathcal{O}(1)$ dynamic update corrections, degree computation, and honest verification logging.
- `FINAL_VERIFICATION_REPORT.md`: Comprehensive breakdown of the 77-unit cross-verification suite (60 3-way, 17 2-way with honest BK sentinels).
- `FINAL_BENCHMARK_REPORT.md`: Full empirical evaluation tables across static scaling ($n=10$ to $1,000,000$) and dynamic workloads ($n=10$ to $5,000$), with break-even analysis ($K^* \in [3, 4]$).
- `FINAL_THESIS_READY_REPORT.md`: Synthesis of the maximal-clique research findings structured for direct inclusion in the BTP thesis.
- `FINAL_CHANGELOG.md`: Full chronological record of revisions, proof gap closures, and bug fixes.
- `FINAL_OPEN_ISSUES.md`: Explicit boundary conditions, open research questions, and future research directions.

