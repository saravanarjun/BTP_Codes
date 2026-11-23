# Final Research Closure Report: BTP Maximal Clique Module

**Project:** Dynamic Graph Algorithms on Proper Interval Graphs  
**Module:** `benchmarks/maximal_cliques/`  
**Repository:** `C:/Users/sarav/OneDrive/Desktop/BTP_Codes`  
**Date:** November 2026  
**Status:** **PROJECT CLOSED — READY FOR THESIS INTEGRATION**  

---

## 1. Executive Summary

This document constitutes the final closure report for the maximal-clique research and benchmarking module of the Bachelor of Technology Project (BTP). Following the execution of Stage 1.6 and the targeted mathematical, algorithmic, and documentation refinements of Stage 1.7, the maximal-clique research module has achieved complete mathematical, algorithmic, implementation, and empirical closure.

All identified discrepancies in break-even arithmetic, complexity claims, mathematical proof dependencies, and test suite descriptions have been resolved. The complete regression suite across all 9 project phases passes with 100% success. The maximal-clique module is verified, reproducible, and ready for formal thesis integration under its specified operational scope.

---

## 2. Corrections Executed in Stage 1.7

The following targeted corrections were implemented across code and documentation:

1. **Break-Even Arithmetic & Input Reconciliation:**
   - Reconciled all numbers in `FINAL_BENCHMARK_REPORT.md` and related documents against the exact empirical measurements in `static_summary.csv` and `dynamic_summary.csv`.
   - Defined two explicit, mathematically sound break-even models:
     - **Model 1 (Dynamic Workload Recomputation with Algorithm C):** Evaluates $K^*$ against the cost of running Algorithm C from scratch on graphs of size $(n+1)$ or $(n-1)$ generated during dynamic benchmark runs. Yields $K^* \in [2, 3]$ operations across $n \in [50, 5000]$.
     - **Model 2 (Static Benchmark Baseline with Algorithm C):** Evaluates $K^*$ against the baseline static end-to-end execution of Algorithm C on the Medium-Overlap profile of size $n$. Consistently yields $K^* = 4$ operations across all tested graph sizes ($n = 50, 1000, 5000$).
   - Corrected all ceiling arithmetic expressions ($K^* = \lceil T_{\text{static}} / \Delta T \rceil$) and documented the mathematical condition under which a finite positive break-even threshold exists ($\Delta T = T_{\text{recompute}} - T_{\text{dynamic}} > 0$).
2. **Decomposition of Deletion Complexity:**
   - Disaggregated the asymptotic complexity of decremental deletion into its constituent algorithmic and container operations:
     - **Local Descriptor Maintenance:** $\mathcal{O}(\log q)$ time, modifying at most 3 descriptor entries in `std::map<int, int>` (where $q \le n$ is the number of maximal cliques).
     - **Child Adoption and Parent Link Update:** $\mathcal{O}(|C(u)|)$ pointer updates, where $|C(u)| \le \text{deg}(u)$ is the child count of the deleted vertex $u$.
     - **Distance Tree Node Pointers:** $\mathcal{O}(1)$ sibling and level linked-list adjustments (Theorem 3.16).
     - **Interval Vector Compaction:** $\mathcal{O}(n)$ time via `std::remove_if` on `std::vector<Interval>` in the high-level `DynamicDistanceTree::deleteVertex` helper.
   - Clarified that the public helper method has overall time complexity $\mathcal{O}(n + |C(u)| + \log q)$, and explained why microsecond-level telemetry (e.g., $3.25\ \mu\text{s}$ at $n=5000$) captures this linear container compaction.
3. **Explicit Mathematical Proof Dependencies & Qualification:**
   - Added an explicit dependency taxonomy to Section 5 of `FINAL_MATHEMATICAL_PROOFS.md`, categorizing every result into:
     - *Independently proven in this module:* Theorem 1 (Static Bijective Maximal Clique Characterization), boundary cases ($z < p$ and $z > c(p)$), injectivity, Theorem 2 (Dynamic Insertion Invariant $\le 1$), Theorem 3 (Dynamic Deletion Invariant $\le 3$), Case B 7-vertex non-monotonic counterexample, and Algorithm C 4-part correctness.
     - *Inherited from BTP Thesis Chapter 3:* Lemma 3.1–3.4 (Canonical positional ordering), Lemma 3.8 (Closed neighborhood representation), Lemma 3.15 (Compact clique descriptors), and Theorem 3.16 (Decremental distance tree maintenance).
     - *Scope & Proof Qualification:* Clarified that codebase regression tests establish software implementation consistency rather than formal mathematical proofs. Inherited lemmas rely entirely on the original proofs in Chapter 3 of the BTP thesis document (`BTP_Thesis (1).pdf`) and were not independently reproved or cross-checked from first principles if the original thesis source is unavailable.
     - *Underlying assumptions:* Connected proper interval graphs, canonical ordering, and lexicographic BFS tie-breaking.
4. **Correction of Phase 9 Test 24:**
   - Renamed test in `tests/test_main.cpp` from `testDecTree_24_SmallGraphExhaustiveOracle` to `testDecTree_24_SmallGraphDeterministicDeletions`.
   - Updated test console output to `"Small-Graph Deterministic Parameterized Deletions Suite"` and pass assertion to `"Parameterized small-graph deletions (n=5..8) 100% verified against independent oracle"`.
   - Accurately described the 20 deterministic parameterized instances generated across varying intervals and length modulations.
   - Synchronized all audit and changelog documents.

---

## 3. Reconciled Break-Even Analysis

### 3.1 Mathematical Formulation

Let:
- $T_{\text{static}}$ be the cost to build the initial distance tree and descriptors from scratch using Algorithm A.
- $T_{\text{recompute}}$ be the cost to recompute maximal cliques from scratch after each dynamic update using the fastest baseline (Algorithm C, interval sweep-line).
- $T_{\text{dynamic}}$ be the execution time of Algorithm A's dynamic update (Theorem 2 for insertion, Theorem 3 for deletion).
- $\Delta T = T_{\text{recompute}} - T_{\text{dynamic}}$ be the net time saved per operation by maintaining dynamically instead of recomputing.

The break-even operation count $K^*$ is given by:
$$K^* = \left\lceil \frac{T_{\text{static}}}{\Delta T} \right\rceil = \left\lceil \frac{T_{\text{static}}}{T_{\text{recompute}} - T_{\text{dynamic}}} \right\rceil$$

A finite positive threshold $K^* \ge 1$ exists if and only if $\Delta T > 0$ (i.e., dynamic maintenance is strictly faster than from-scratch recomputation).

### 3.2 Exact Source Measurements

The calculations are derived directly from the generated CSV benchmark files:
- Static baseline timings from `static_summary.csv` (Medium-Overlap profile):
  - $n = 50$: Alg A E2E = $0.0212\text{ ms}$, Alg C E2E = $0.0072\text{ ms}$
  - $n = 1000$: Alg A E2E = $0.4682\text{ ms}$, Alg C E2E = $0.1264\text{ ms}$
  - $n = 5000$: Alg A E2E = $2.4187\text{ ms}$, Alg C E2E = $0.6659\text{ ms}$
- Dynamic update timings from `dynamic_summary.csv`:
  - $n = 50$:
    - Insertion: Alg A = $0.0004\text{ ms}$, Alg C Recompute = $0.0182\text{ ms}$
    - Deletion: Alg A = $0.0002\text{ ms}$, Alg C Recompute = $0.0082\text{ ms}$
  - $n = 1000$:
    - Insertion: Alg A = $0.0005\text{ ms}$, Alg C Recompute = $0.2014\text{ ms}$
    - Deletion: Alg A = $0.0009\text{ ms}$, Alg C Recompute = $0.2110\text{ ms}$
  - $n = 5000$:
    - Insertion: Alg A = $0.0007\text{ ms}$, Alg C Recompute = $1.4112\text{ ms}$
    - Deletion: Alg A = $0.0033\text{ ms}$, Alg C Recompute = $1.3980\text{ ms}$

### 3.3 Reconciled Thresholds

#### Model 1: Dynamic Workload Recomputation
Comparing dynamic maintenance against the actual per-operation recomputation cost of Algorithm C on the dynamic graph:
- **At $n = 50$:**
  - Insertion: $\Delta T = 0.0182 - 0.0004 = 0.0178\text{ ms} \implies K^* = \lceil 0.0212 / 0.0178 \rceil = \lceil 1.191 \rceil = \mathbf{2}$
  - Deletion: $\Delta T = 0.0082 - 0.0002 = 0.0080\text{ ms} \implies K^* = \lceil 0.0212 / 0.0080 \rceil = \lceil 2.650 \rceil = \mathbf{3}$
- **At $n = 1000$:**
  - Insertion: $\Delta T = 0.2014 - 0.0005 = 0.2009\text{ ms} \implies K^* = \lceil 0.4682 / 0.2009 \rceil = \lceil 2.331 \rceil = \mathbf{3}$
  - Deletion: $\Delta T = 0.2110 - 0.0009 = 0.2101\text{ ms} \implies K^* = \lceil 0.4682 / 0.2101 \rceil = \lceil 2.228 \rceil = \mathbf{3}$
- **At $n = 5000$:**
  - Insertion: $\Delta T = 1.4112 - 0.0007 = 1.4105\text{ ms} \implies K^* = \lceil 2.4187 / 1.4105 \rceil = \lceil 1.715 \rceil = \mathbf{2}$
  - Deletion: $\Delta T = 1.3980 - 0.0033 = 1.3947\text{ ms} \implies K^* = \lceil 2.4187 / 1.3947 \rceil = \lceil 1.734 \rceil = \mathbf{2}$

**Conclusion for Model 1:** Dynamic maintenance pays for its initial distance tree construction within **2 to 3 dynamic operations**.

#### Model 2: Static Benchmark Profile Comparison
Comparing dynamic maintenance against the baseline static recomputation of Algorithm C:
- **At $n = 50$:** $\Delta T = 0.0072 - 0.0004 = 0.0068\text{ ms} \implies K^* = \lceil 0.0212 / 0.0068 \rceil = \lceil 3.118 \rceil = \mathbf{4}$
- **At $n = 1000$:** $\Delta T = 0.1264 - 0.0005 = 0.1259\text{ ms} \implies K^* = \lceil 0.4682 / 0.1259 \rceil = \lceil 3.719 \rceil = \mathbf{4}$
- **At $n = 5000$:** $\Delta T = 0.6659 - 0.0007 = 0.6652\text{ ms} \implies K^* = \lceil 2.4187 / 0.6652 \rceil = \lceil 3.636 \rceil = \mathbf{4}$

**Conclusion for Model 2:** Under static benchmark recomputation, dynamic maintenance consistently breaks even at **$K^* = 4$ operations** across all tested graph sizes ($n = 50, 1000, 5000$).

---

## 4. Algorithmic and Structural Complexity

The complexity bounds across the maximal clique algorithms are summarized below:

| Algorithm / Operation | Theoretical Descriptor / Tree Complexity | Container Overhead | Public Method Complexity |
| :--- | :--- | :--- | :--- |
| **Algorithm A (Static Construction)** | $\mathcal{O}(n + m)$ | None (vector indexable) | $\mathcal{O}(n + m)$ time, $\mathcal{O}(n)$ space |
| **Algorithm C (Static Sweep-Line)** | $\mathcal{O}(n \log n + m)$ (sorting + sweep) | Output vector | $\mathcal{O}(n \log n + m)$ time, $\mathcal{O}(n)$ space |
| **Algorithm B (Bron–Kerbosch)** | $\mathcal{O}(3^{n/3})$ worst case | Recursion stack | Exponential time |
| **Algorithm A: Rightmost Insertion** | $\mathcal{O}(1)$ (at most 1 descriptor updated) | $\mathcal{O}(\log q)$ (`std::map`) | $\mathcal{O}(\log q)$ time |
| **Algorithm A: Decremental Deletion** | $\mathcal{O}(|C(u)|)$ child links, $\le 3$ descriptors | $\mathcal{O}(\log q)$ map, $\mathcal{O}(n)$ vector compaction | $\mathcal{O}(n + |C(u)| + \log q)$ time |

### Key Complexity Insights:
1. **Descriptor Maintenance:** Under Theorem 2, rightmost insertion updates at most **1 descriptor**. Under Theorem 3, decremental deletion updates at most **3 descriptors**.
2. **Container Impact:** Using `std::map<int, int>` yields an $\mathcal{O}(\log q)$ overhead per descriptor access. The auxiliary `std::remove_if` vector scan in `DynamicDistanceTree::deleteVertex` adds an $\mathcal{O}(n)$ term. In low-level telemetry, this linear compaction accounts for the rise in deletion time from $0.20\ \mu\text{s}$ at $n=50$ to $3.25\ \mu\text{s}$ at $n=5000$.
3. **Space Advantage:** Compact descriptor representation requires strictly $2q \times \text{sizeof}(\text{int})$ bytes ($\le 2n$ integers), achieving up to **$83\times$ memory reduction** over explicit clique lists on dense graphs.

---

## 5. Verification and Test Results

### 5.1 Project-Wide Regression Test Suite
Execution of `test_runner.exe` in `build/Release/` confirms:
- **Total Test Cases:** 56
- **Passed:** 56
- **Failed:** 0
- **Phases Covered:**
  - Phases 1–4: Interval graphs, canonical distance tree construction, BFS tie-breaking.
  - Phase 5: Static maximal clique descriptors, Bron–Kerbosch verification.
  - Phases 6–8: Independent sets, incremental maintenance, structural interval properties.
  - Phase 9: Decremental distance tree maintenance (Theorem 3.16), child adoption, sibling relinking, and 20 deterministic parameterized small-graph instances (Test 24).

### 5.2 Maximal Clique Verification Harness
Execution of `clique_benchmark.exe --verify` confirms:
- **Total Verification Units:** 77
- **Passed:** 77
- **Failed:** 0
- **Families & Workloads Verified:**
  - 14 distinct graph topologies evaluated via 17 concrete test graph configurations across sizes $n \in \{10, 20, 50, 100\}$ (68 static verification units).
  - Cross-validation: Algorithm A (descriptors materialized) $\equiv$ Algorithm B (Bron–Kerbosch) $\equiv$ Algorithm C (Sweep-line).
  - Dynamic verification: 165 step-by-step dynamic operations across 9 test sequences (60 incremental insertions, 45 decremental deletions, 60 mixed operations across 3 seeds), matched against both Algorithm C and static distance tree rebuilds from scratch at every single step with zero discrepancies. (Note: this correctness harness is distinct from the latency benchmark suite, which evaluates sequences of 100 dynamic operations to measure execution latency).

---

## 6. Disclosed Limitations for Thesis Integration

To maintain the highest standard of academic rigor, the following operational boundaries must be explicitly stated in the thesis:

1. **Dynamic Operation Scope:**
   - *Insertion:* Restricted to rightmost append-only leaf insertions where the new vertex does not alter previous level assignments.
   - *Deletion:* Restricted to admissible non-root deletions satisfying Theorem 3.16 (where vertex $u$ has an immediate right neighbor in the same level $L_k$ whenever $u$ has children). Arbitrary deletions that disconnect the graph or alter shortest-path level depths require full $\mathcal{O}(n + m)$ static tree recomputation.
2. **Static Sweep-Line Superiority for Single Runs:**
   - In a purely static setting where maximal cliques are computed only once, Algorithm C (Interval Sweep-Line) is faster than Algorithm A by a factor of $3\times$ to $4\times$ because it avoids constructing the full distance tree.
   - Algorithm A's computational advantage is realized in dynamic environments, where incremental maintenance takes $< 3.5\ \mu\text{s}$, yielding an amortized speedup of up to $2000\times$ over static recomputation.
3. **Container Implementation Overheads:**
   - The current implementation employs `std::map<int, int>` for descriptors ($\mathcal{O}(\log q)$ lookups) and `std::vector<Interval>` compaction ($\mathcal{O}(n)$ scan). While asymptotically sub-optimal compared to theoretical $\mathcal{O}(1)$ random-access arrays or indexed node pools, practical performance remains in the low microseconds across all tested graph sizes.

---

## 7. Final Research Closure Statement

The maximal-clique research and benchmarking module in `benchmarks/maximal_cliques/` is:
- **Mathematically Grounded:** The module's novel dynamic invariants (Theorems 1, 2, and 3, boundary conditions, and tight delta bounds) are rigorously derived, while foundational structural properties rely on the established proofs in the BTP thesis.
- **Fully Implemented:** Static descriptor enumeration, dynamic leaf insertion, and decremental distance tree maintenance are fully realized in standard C++17.
- **Empirically Verified:** 100% test pass rate across 56 project-wide unit tests and 77 maximal-clique verification units (covering 14 graph families via 17 configurations and 165 dynamic update operations).
- **Reproducible:** All static and dynamic benchmarks, raw CSV datasets, summary tables, and high-resolution visualization plots are fully preserved and reproducible.

**Under the documented operational scope (rightmost append-only leaf insertions and Theorem 3.16 admissible deletions), the maximal-clique research and benchmarking module is fully implemented, verified for software consistency, and ready for thesis integration.**
