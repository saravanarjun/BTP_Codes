# Comprehensive Experimental Benchmarking Methodology for Maximal Cliques on Proper Interval Graphs

**Project**: Dynamic Graph Algorithms — Interval Graphs  
**Date**: November 2026  
**Module**: `benchmarks/maximal_cliques/`  

---

## 1. Executive Summary & Purpose

This document outlines the rigorous, reproducible benchmarking methodology designed to evaluate the computational performance, memory efficiency, and structural scalability of maximal clique algorithms on connected proper interval graphs.

The study investigates three fundamentally distinct algorithmic paradigms across both **static** (from scratch) and **dynamic** (incremental and decremental updates) operating regimes:

1. **Algorithm A — Theorem-Based Algorithm (Thesis Source of Truth)**:
   - *Static*: Lemma 3.8 / Lemma 3.15 compact maximal clique descriptor formulation. Evaluates candidate parents $p \in V$ and their rightmost children $c_p$ to define maximal cliques as interval ranges $[l_p, l_{c_p}]$.
   - *Dynamic*: Maintains distance tree and compact descriptors under Section 3.10.2 leaf insertions and Section 3.10.3 decremental vertex deletions. Mutates at most $\mathcal{O}(1)$ compact descriptors per update.
2. **Algorithm B — Pivoted Bron–Kerbosch (General Graph Baseline)**:
   - *Static*: Recursive maximal clique backtracking with Tomita degree-maximizing pivot selection.
   - *Dynamic* (`BK-Full-Recomputation`): Full recomputation of all maximal cliques from scratch after each vertex update, bounded by a 3,000 ms wall-clock timeout.
3. **Algorithm C — Standard Interval-Graph Sweep-Line (Domain-Specific Baseline)**:
   - *Static*: Helly property endpoint event sorting and active interval sweeping.
   - *Dynamic* (`Sweep-Full-Recomputation`): Incremental graph update followed by full sweep-line recomputation from scratch.

---

## 2. Experimental Environment & Hardware Specifications

All experiments were executed natively on the host workstation under standardized operating conditions to eliminate external throttling and background interference:

- **Operating System**: Microsoft Windows 11 Enterprise (x64)
- **Compiler**: Microsoft Visual C++ Compiler (MSVC 2022 v19.44 / C++17)
- **Compilation Flags**: `/O2 /EHsc /utf-8 /STACK:67108864` (Full release optimization, 64 MB execution stack)
- **Timing Instrumentation**: High-resolution monotonic clock (`std::chrono::steady_clock`, sub-microsecond precision)
- **Process Priority**: Dedicated thread, standard high-priority scheduling

---

## 3. Graph Families & Structural Diversity

To prevent synthetic bias and stress-test the algorithms across extreme graph morphologies, the benchmarking suite generates **14 distinct graph families**:

| Family # | Name | Generation Mechanism & Theoretical Stress Focus |
|:---:|:---|:---|
| **1** | **Equal-Length Intervals** | Regularly spaced intervals of identical width. Tested across three overlap densities: **Sparse** (width 1.5, clique size $\approx 2$), **Medium** (width 8.0, clique size $\approx 8$), and **Dense** (width 25.0, clique size $\approx 25$). Tests baseline scaling and bandwidth sensitivity. |
| **2** | **Unequal-Length Intervals** | Proper interval graphs with strictly monotonically increasing left and right endpoints ($l_i < l_{i+1}$ and $r_i < r_{i+1}$) where widths vary sinusoidally. Guarantees no interval contains another while varying degrees. |
| **3** | **Random Proper Intervals** | Uniformly distributed endpoint advances ($0.5 \mu$ to $1.5 \mu$) with random widths, seeded via Mersenne Twister (`std::mt19937`). Generates diverse, realistic topology. |
| **4** | **Identical Interval Clusters** | Groups of identical intervals ($l_i = l_j$, $r_i = r_j$). Tests non-strict containment where intervals coincide, exercising canonical tie-breaking rules. |
| **5** | **Endpoint-Touching Intervals** | Closed-interval boundary contacts where $r_i = l_{i+1}$. Produces path-like graphs ($P_n$) where adjacent vertices intersect only at boundary points. |
| **6** | **Long Chain Graphs** | Path-like proper interval chains with minimal overlap bandwidth ($k=1$), forming trees with depth $h = n$. |
| **7** | **High-Overlap Graphs** | Broad bandwidth ($k=15$), producing dense graphs with large individual maximal cliques ($|C| \ge 16$) and massive edge densities. |
| **8** | **Alternating Lengths** | Alternates between short ($w=1.5$) and long ($w=5.0$) interval widths while enforcing strict properness via progressive left advances. |
| **9** | **Irregularly Spaced Endpoints** | Large variance in gaps between consecutive left endpoints ($0.2$ to $1.8$), creating non-uniform level populations in the distance tree. |
| **10** | **Many Small Cliques** | Sparse topology yielding a large number of distinct maximal cliques ($q \approx n$), each having small membership ($|C| \approx 2$). |
| **11** | **Few Large Cliques** | Highly clustered intervals forming very few maximal cliques ($q \ll n$), each containing many vertices ($|C| \gg 1$). |
| **12** | **Varying Overlap Profile** | Overlap bandwidth oscillates across the graph, transitioning dynamically between sparse and dense regions. |
| **13** | **Adversarial Tie-Breaking** | Multiple distinct vertices sharing identical left and right endpoints in clusters of size 4, testing lexicographic tie-breaking in distance-tree parent selection. |
| **14** | **Shuffled Vertex IDs** | Proper interval geometry where vertex IDs are randomly permuted, ensuring algorithms rely strictly on geometric coordinates and index mappings rather than ID order. |

---

## 4. Benchmark Workloads & Execution Profiles

### 4.1 Static Benchmark Protocol
- **Target Sizes ($n$)**: $10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000$.
- **Graph Coverage**: All 14 families evaluated up to $n=50,000$; sparse, medium, and dense profiles evaluated up to $n=1,000,000$.
- **Adaptive Repetitions**:
  - $n \le 100$: 20 repetitions
  - $100 < n \le 1000$: 10 repetitions
  - $1000 < n \le 10000$: 5 repetitions
  - $10000 < n \le 50000$: 3 repetitions
  - $n > 50000$: 2 repetitions
- **Separation of Timing Components**:
  - `preprocessing_ms`: Graph validation, sorting, distance-tree construction (Alg A) or event queue generation (Alg C).
  - `core_ms`: Identification of non-leaf parent descriptors (Alg A), Tomita pivot exploration (Alg B), or active set endpoint sweep (Alg C).
  - `materialization_ms`: Explicit conversion of compact interval descriptors $[l_p, l_c]$ into explicit vertex ID sets $\mathcal{C}$.
  - `postprocessing_total_ms`: `core_ms` + `materialization_ms`.
  - `end_to_end_sum_ms`: `preprocessing_ms` + `core_ms` + `materialization_ms`.
  - `direct_end_to_end_ms`: Independent single-pass measurement executing the entire pipeline continuously from unparsed intervals to materialized cliques.

### 4.2 Dynamic Benchmark Protocol
Evaluated across initial graph sizes $n \in \{50, 100, 500, 1000, 5000\}$:
- **Workload A (Incremental Leaf Insertion, 100% Ins)**: 50 sequential rightmost leaf insertions per run.
- **Workload B (Decremental Deletion, 100% Del)**: 30 sequential valid deletions under Theorem 3.16 per run.
- **Workload C (Mixed Workloads)**: 40 operations per run evaluated across three update distributions:
  - **75% Insertions / 25% Deletions** (Growth-heavy)
  - **50% Insertions / 50% Deletions** (Equilibrium)
  - **25% Insertions / 75% Deletions** (Shrinkage-heavy)

### 4.3 Policy Limits & Timeout Protection
- **Bron–Kerbosch Timeout**: A strict 3,000 ms wall-clock limit is enforced via periodic recursive checks (`callCount % 500 == 0`). If exceeded, execution halts immediately and status is recorded as `TIMEOUT`.
- **Bron–Kerbosch Scaling Boundary**: In static benchmarks, Alg B is executed only up to $n=2,000$; for $n > 2,000$, it is skipped by policy and marked `TIMEOUT`. In dynamic benchmarks, Alg B is executed only up to initial $n=500$ and $\le 50$ operations.

---

## 5. Formal Correctness & Verification Protocol

Before running benchmarks, the suite executes an automated **77-test verification harness** (`clique_benchmark --verify`):

1. **Static 3-Way Equivalence ($A == B == C$)**: For all 14 graph families at $n \in \{10, 20, 50, 100\}$, the cliques generated by Algorithm A, Algorithm B, and Algorithm C are canonicalized (sorted internally and lexicographically) and asserted to be 100% identical.
2. **Pairwise Adjacency (Clique Property)**: Every materialized clique $C \in \mathcal{C}$ is checked to guarantee every pair $u, v \in C$ satisfies $I_u \cap I_v \neq \emptyset$.
3. **Maximality Verification**: Every clique $C$ is tested against all $w \in V \setminus C$. If any vertex $w$ is adjacent to all members of $C$, the test fails immediately.
4. **Dynamic Step-by-Step Cross-Verification**: After *every single* dynamic operation (insertion or deletion), Algorithm A is verified against:
   - Dynamic Sweep-Line (`DynamicSweepLine`).
   - Dynamic Bron-Kerbosch (`DynamicBronKerbosch`, when $n \le 50$).
   - A complete static rebuild of the distance tree and descriptors from the exported graph state.
