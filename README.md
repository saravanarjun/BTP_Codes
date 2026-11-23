# Dynamic Graph Algorithms – Connected Proper Interval Graphs

Implementation of the structural analysis, distance trees, maximal cliques, independent sets, and dynamic updates presented in the B.Tech Thesis:
**"Dynamic Graph Algorithms - Interval Graphs"** by **K. Saravanarjun** (IIT Bhubaneswar, August 2026), under the guidance of **Dr. Manas Jyothi Kashyop**.

---

## 1. Project Overview

Proper interval graphs (unit interval graphs / indifference graphs) admit an interval representation where no interval properly contains another. This project provides a research-grade C++17 implementation faithful to the mathematical definitions, lemmas, theorems, and algorithms in Chapters 2 and 3 of the thesis (excluding Section 3.11 on Graph Burning).

### Core Capabilities
1. **Implicit Interval Representation & Validation**:
   - $O(n \log n)$ sorting with tie-breaking.
   - $O(n)$ sweep-line validation for properness and connectedness.
   - $O(1)$ interval intersection adjacency testing.
2. **Distance Tree Construction ($T(G)$)**:
   - Rooted distance tree based on parent selection: $\text{parent}(v) = \arg\min_{u: r_u \ge \ell_v} \ell_u$.
3. **Level Structure & Positional Parameters**:
   - Depth, level sets, level-order, preorder, and postorder traversals.
4. **Distance Encoding Formula & BFS Verification**:
   - Evaluation of $\text{dist}_G(u, v) = \text{depth}(u) - \text{depth}(v) + [\text{pos}_{\text{post}}(u) > \text{pos}_{\text{post}}(v)]$.
5. **Structural Lemmas Verification**:
   - Executable verification of Lemmas 3.1–3.6 and Theorem 3.7.
6. **Maximal Cliques**:
   - Explicit clique construction (Lemma 3.8) and compact representation $[\ell_p, \ell_c]$ (Lemma 3.15).
7. **Independent Set Optimization**:
   - Bounds ($\lfloor h/2 \rfloor \le \alpha(G) \le h-1$), 2-approximation algorithm, and $O(n)$ exact dynamic programming.
8. **Dominating Set Bound & Construction**:
   - $\gamma(G) \le \lceil (h-1)/2 \rceil$ (Theorem 3.17).
9. **Dynamic Updates**:
   - Incremental leaf insertion ($O(\log n)$ MIS & clique updates).
   - Decremental vertex deletion ($O(\log n)$ child relocation).

---

## 2. Directory Structure

```text
BTP_Interval_Graphs/
│
├── CMakeLists.txt              # CMake build configuration (C++17, MSVC / Clang / GCC)
├── README.md                   # Project overview, architecture, and manual
├── VALIDATION_REPORT.md        # Mathematical validation & test verification ledger
│
├── input/
│   ├── intervals.txt           # Input interval dataset (n, id, left, right)
│   └── updates.txt             # Incremental/decremental test queries
│
├── output/
│   ├── sorted_intervals.txt    # Sorted intervals
│   ├── distance_tree.txt       # Parent-child tree structure
│   ├── tree_parameters.txt     # Depths, levels, pos_level, pos_pre, pos_post
│   ├── structural_checks.txt   # PASS/FAIL audit of Lemmas 3.1-3.6, Theorem 3.7
│   ├── maximal_cliques.txt     # Compact & explicit maximal cliques
│   ├── independent_set.txt     # MIS bounds, 2-approx, exact DP set
│   ├── dominating_set.txt      # Minimum dominating set construction & check
│   ├── dynamic_updates.txt     # Incremental/decremental execution trace
│   └── summary.txt             # Global summary report
│
├── src/
│   ├── main.cpp                # Pipeline driver
│   ├── IntervalGraph.h/.cpp    # Module 1: Interval graph representation & validation
│   ├── DistanceTree.h/.cpp     # Module 2 & 3: Distance tree, levels, traversals
│   ├── StructuralAnalysis.h/.cpp # Module 4 & 5: Distance check & structural lemmas
│   ├── MaximalCliques.h/.cpp   # Module 6: Maximal cliques (explicit & compact)
│   ├── IndependentSet.h/.cpp   # Module 7: MIS bounds, approximation & exact DP
│   ├── DominatingSet.h/.cpp    # Module 8: Dominating set upper bound construction
│   ├── IncrementalIndependentSet.h/.cpp # Module 9: Incremental MIS maintenance (Theorem 3.14)
│   └── DecrementalDistanceTree.h/.cpp   # Module 10: Decremental child relocation (Theorem 3.16)
│
└── tests/
    └── test_main.cpp           # Complete unit and regression test suite
```

---

## 3. Building and Running

### Prerequisites
- C++17 compatible compiler (MSVC 2022, Clang 12+, or GCC 9+)
- CMake 3.15+

### Build Instructions (Windows / MSVC)
```powershell
cmake -B build -S .
cmake --build build --config Release
```

### Running the Pipeline
```powershell
.\build\Release\interval_graphs.exe
```

### Running Tests
```powershell
.\build\Release\test_runner.exe
```

---

## 4. Current Status
- **Phase 1 Completed**: Interval input parsing, $O(n \log n)$ sorting with tie-breaking, $O(n)$ properness and connectedness validation, adjacency tests, closed neighbourhood queries, unit tests passing.
- **Phase 2 Completed**: Distance tree construction $T(G)$ via Equation (2.1)/(3.1) in $O(n)$ monotonic sweep, verified against independent $O(n^2)$ brute-force implementation; BFS level structure with both 0-based depth and 1-based level conventions; level sets $L_d$ ordered left-to-right; positional parameters ($pos_{level}, pos_{pre}, pos_{post}$) computed; export of `distance_tree.txt` and `tree_parameters.txt`; 7 comprehensive test suites passing.
- **Phase 3 Completed**: Shortest-path distance formula (Equation 2.2) implemented and verified against independent BFS ground-truth on interval intersection; directional and symmetric query handling; positional ordering property ($pos_{level}(u) < pos_{level}(v) \iff u < v$) verified across all instances; exhaustive testing on 50 deterministic random proper interval graphs (7,800 pairs) with 0 mismatches.
- **Phase 4 Completed**: Structural analysis and executable verification module for Lemmas 3.1–3.6, Theorem 3.7, and Depth Locality; independent ground-truth evaluation via interval intersection; verified across handcrafted topologies and 50 random graphs (3,642 structural checks, 292 level-clique pairs checked in Theorem 3.7) with 0 violations; export of `output/structural_checks.txt`.
- **Phase 5 Completed & Clarified**: Maximal clique construction using distance tree (Lemma 3.8) and compact interval representation $[\ell_p, \ell_c]$ (Lemma 3.15); independent verification via Bron–Kerbosch maximal clique enumeration (used as correctness oracle); leaf non-maximal sub-clique qualification documented (Lemma 3.8 yields maximal cliques for non-leaf parent nodes; leaf nodes produce sub-cliques extendable by ancestors); cross-verified across handcrafted topologies and 50 random graphs (369 maximal cliques verified with 100% exact set matches); export of `output/maximal_cliques.txt`, `output/compact_cliques.txt`, and `output/clique_verification.txt`.
- **Phase 6 Completed**: Maximum Independent Set algorithms from Section 3.10; Lemma 3.11 bounds ($\lfloor h/2 \rfloor \le \alpha(G) \le h - 1$ for $h \ge 2$, with explicit handling of the $h=1$ degenerate boundary case $\alpha=1$); Theorem 3.12 2-approximation algorithm in $O(h)$ time selecting alternate level rightmost vertices; Section 3.10.1 & Theorem 3.13 exact linear-time $O(n)$ dynamic programming with prefix-maximum optimization and $O(n)$ independent set reconstruction; verified against independent exponential brute-force solver on 50 random graphs (100% agreement); export of `output/mis_bounds.txt`, `output/mis_approximation.txt`, `output/mis_exact_dp.txt`, and `output/mis_verification.txt`.
- **Phase 7 Completed**: Dominating Set upper bound and explicit construction (Theorem 3.17, Section 3.10, page 47 of thesis); select rightmost vertex of Level 2 (dominates Levels 1, 2, 3), followed by the rightmost vertex of the first level of each successive pair $(4,5), (6,7), \dots$ (dominating Levels $i$ and $i+1$); bound $\gamma(G) \le \lceil (h-1)/2 \rceil$ (and $\gamma(G) \le (h-1)/2$ for odd $h$) verified; boundary cases analyzed ($h=1$ single-vertex graph where formula gives 0 and Level 2 is absent, handled explicitly with $|D|=1$; $h=2$ two-level graph where $|D|=1 \le 1$ holds without qualification); $O(h)$ construction time and $O(h)$ space; verified against independent interval intersection dominating set validator and brute-force Minimum Dominating Set (MDS) oracle across handcrafted topologies and 50 random proper interval graphs (100% domination coverage, 0 uncovered vertices, 100% bound compliance); export of `output/dominating_set.txt` and `output/dominating_set_verification.txt`.
- **Phase 8 Completed**: Incremental maintenance of Maximum Independent Set under the Rightmost Append-Only Leaf Insertion Model (Theorem 3.14, Section 3.10.2, pages 44–45 of thesis); recurrence $DP[i][u] = 1 + \max(BEST[i-2], \text{prefixMax}_{L_{i-1}}(p))$ and global $\alpha(G') \leftarrow \max(\alpha(G), DP[i][u])$; dynamic prefix maxima complexity gap resolved via Augmented AVL Trees (`LevelAVLTree`) storing subtree `maxDP`, enabling genuine $O(\log n)$ worst-case prefix queries and insertions; parent search in $O(\log n)$ implemented via Augmented AVL Tree (`ParentAVLTree`) storing subtree minimum candidate $(\ell, r, \text{id})$; full Rule-of-Five compliance (deep-copy tree cloning and move semantics) for memory safety; validation enforces connected proper interval graph invariants, leaf non-adoption invariants, and rightmost append-only ordering within the assigned level (middle-of-level and leftmost insertions are rejected with clear diagnostics to prevent cascading DP invalidation on descendants); internal state update runs in worst-case $O(\log n)$ time, while end-to-end insertion with full pairwise validation is $O(n)$; comprehensive test suite with 19 test suites and 180 dynamic sequential leaf insertions across 30 random sequences verified against static Phase 6 DP and independent brute-force MIS oracle (100% exact match).
- **Phase 9 Completed (Final Implementation Phase)**: Decremental maintenance of distance tree under vertex deletions (Observation 3.10.3 and Theorem 3.16, Section 3.10.3, pages 44–46 of thesis); linked child-set representation maintains children in canonical left-to-right order with only boundary links (`childHead`, `childTail`, `prevSibling`, `nextSibling`, `prevLevelSibling`, `nextLevelSibling`); vertex deletion relocates former children to the immediate right same-level vertex in $O(1)$ boundary link modifications without relinking individual children; parent search in $O(\log n)$ via `ParentAVLTree`; boundary conditions handled (reject root deletion, reject deletions disconnecting graph, reject non-root vertices with children but no right sibling, handle leaf deletions and tree height reductions); atomic execution with independent static rebuild verification; 17 unit test suites and 240 sequential random deletions across 30 sequences verified against fresh static distance trees and brute-force MIS oracles (100% exact match); Section 3.11 (Graph Burning) is strictly excluded.
- **Maximal Clique Experimental Benchmarking Completed**: Comprehensive empirical evaluation module (`benchmarks/maximal_cliques/`) comparing three algorithms (Algorithm A: Thesis Theorem/Compact Descriptor formulation, Algorithm B: Pivoted Bron-Kerbosch, Algorithm C: Standard Interval Sweep-Line) across both static (from scratch up to $n = 1,000,000$) and dynamic settings (incremental leaf insertions, decremental deletions, and mixed workloads up to $n = 5,000$). 77-test automated verification harness (100% pass); 15 publication-quality plot figures; comprehensive methodology and empirical report.

---

## 5. Maximal Clique Experimental Evaluation

For full documentation, benchmarking protocols, and empirical reports comparing maximal clique algorithms on proper interval graphs, see [`benchmarks/maximal_cliques/README.md`](benchmarks/maximal_cliques/README.md):

- **Methodology Specification**: [`benchmarks/maximal_cliques/BENCHMARK_METHODOLOGY.md`](benchmarks/maximal_cliques/BENCHMARK_METHODOLOGY.md)
- **Comprehensive Empirical Report**: [`benchmarks/maximal_cliques/BENCHMARK_REPORT.md`](benchmarks/maximal_cliques/BENCHMARK_REPORT.md)
- **Plot Generation**: `python benchmarks/maximal_cliques/plot_benchmarks.py`


