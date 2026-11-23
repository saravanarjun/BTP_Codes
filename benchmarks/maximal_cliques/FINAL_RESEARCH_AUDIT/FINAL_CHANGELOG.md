# Final Changelog: Maximal Clique Research Module

**Document Version:** 1.1 (Stage 1.7 Final Consistency & Project Closure)  
**Date:** November 2026  
**Module:** `benchmarks/maximal_cliques/`  
**Repository:** `BTP_Codes`

---

## 1. Summary of Changes

This changelog records the complete set of mathematical revisions, algorithmic optimizations, code corrections, test harness extensions, and reporting enhancements executed during Stage 1.6 and Stage 1.7 to achieve full closure.

---

## 2. Chronological Record of Revisions (Stage 1.6)

### 2.1 Mathematical Closure
- **Revision of Deletion Descriptor Delta Bound:**
  - *Previous Claim:* $\Delta_{\text{affected}} \le 2$.
  - *Identified Flaw:* Overlooked the case where $u$'s parent $p(u)$ has multiple children, $u$ was the rightmost child of $p(u)$, $u$ had children, and $u$'s immediate right neighbor $v$ was previously childless ($p(v) \ne p(u)$).
  - *Corrected Result:* Rigorously proved that $\Delta_{\text{affected}} \le 3$.
  - *Adversarial Counterexample:* Constructed a concrete 7-vertex proper interval graph exhibiting $\Delta_{\text{removed}} = 1, \Delta_{\text{added}} = 1, \Delta_{\text{modified}} = 1 \implies \Delta_{\text{affected}} = 3$.
- **Withdrawal of Unverified Profiling Claim:**
  - *Previous Claim:* "80% of dynamic runtime was caused by telemetry map copying."
  - *Correction:* Formally withdrew this empirical percentage claim because no isolated profiling instrumentation had been performed. Labeled as an unverified analytical hypothesis.
- **Static Maximal Clique Proof Closure:**
  - Proved all five required parts: pairwise clique property, boundary maximality (both $z < p$ and $z > c(p)$), completeness via Helly's intersection property, injectivity via minimum element uniqueness, and alignment with canonical distance tree tie-breaking.

### 2.2 Source Code Corrections
- **`benchmarks/maximal_cliques/dynamic_clique_structure.cpp`:**
  - In `insertLeaf`: Removed `auto descsBefore = parentToRightmostChild` and the $\mathcal{O}(q \log q)$ set-difference loops. Replaced with localized map lookups on parent $p$ to track $\Delta_{\text{added}}$ and $\Delta_{\text{modified}}$.
  - In `deleteVertex`: Removed `auto descsBefore = parentToRightmostChild` and the $\mathcal{O}(q \log q)$ set-difference loops. Replaced with local state tracking on $u$, $v$, and $p(u)$ to update `parentToRightmostChild`.
- **`benchmarks/maximal_cliques/clique_verifier.h` & `clique_verifier.cpp`:**
  - Added fields `bool bkgRan`, `bool bkgPassed`, `bool bkgTimedOut` to `VerificationResult`.
  - Removed fabrication of `res.numCliquesB = res.numCliquesA` when Bron–Kerbosch was skipped ($n > 50$).
  - Recorded `numCliquesB = -1` and explicit diagnostic string `"A == C verified (PASS), Maximality verified (PASS), BK skipped by policy (n > 50)"`.
  - In dynamic sequence verification, recorded `numCliquesB = trackBK ? dynB.getNumCliques() : -1`.
- **`benchmarks/maximal_cliques/benchmark_runner.cpp`:**
  - Implemented monotonic two-pointer $\mathcal{O}(n)$ degree and edge computation in `computeGraphStats()`.
  - Populated `maxDegree` for all algorithms in CSV output.
  - Replaced `0.0` timeout reporting with honest `-1.0` sentinels across all latency, clique count, and memory fields for Alg B.

### 2.3 Baseline Preservation and Documentation
- **Preservation of Historical Data:**
  - Created `benchmarks/maximal_cliques/historical_baseline/` containing full snapshots of `results/` and `plots/`.
- **Research Documentation:**
  - Created `benchmarks/maximal_cliques/FINAL_RESEARCH_AUDIT/` containing 7 core research documents.

---

## 3. Stage 1.7 Final Corrections and Consistency Reconciliations

### 3.1 Break-Even Analysis Arithmetic and Input Reconciliation
- **Defect Identified:** The reported break-even threshold formula was evaluated inconsistently at $n=50$ (reporting "3–4" while $\lceil 0.0197 / (0.005 - 0.00034) \rceil = 5$), and static Alg C was reported as 0.005 ms rather than its actual CSV value (0.0072 ms).
- **Corrections Executed:**
  - Reconciled all numbers against `static_summary.csv` (`medium-overlap`) and `dynamic_summary.csv`.
  - Formalized two distinct comparison models:
    - **Model 1 (Dynamic Workload Recomputation):** Comparing against Alg C's measured step-by-step recomputation latency ($T_{\text{recompute\_C}}$).
      - $n = 50$: $K^* = 2$ (insertion) / $K^* = 3$ (deletion).
      - $n = 1000$: $K^* = 3$ (insertion) / $K^* = 3$ (deletion).
      - $n = 5000$: $K^* = 2$ (insertion) / $K^* = 2$ (deletion).
      Overall: $K^* \in [2, 3]$ operations.
    - **Model 2 (Static Benchmark Recomputation):** Comparing against Alg C's static benchmark time from `static_summary.csv` ($T_{\text{static\_C}}$).
      - $n = 50$: $K^* = 4$ operations.
      - $n = 1000$: $K^* = 4$ operations.
      - $n = 5000$: $K^* = 4$ operations.
      Overall: Consistently $K^* = 4$ operations across all tested graph sizes.
  - Explicitly documented the non-existence of finite positive break-even if $\Delta T \le 0$.
  - Updated `FINAL_BENCHMARK_REPORT.md` and `FINAL_THESIS_READY_REPORT.md`.

### 3.2 Rigorous Deletion Complexity Decomposition
- **Defect Identified:** `deleteVertex()` was casually referred to as $\mathcal{O}(\log n)$ or $\mathcal{O}(\text{deg}(u))$, overlooking the container operations (`std::map` red-black tree lookups, child traversal loop, and `std::vector` compaction).
- **Corrections Executed:**
  - Fully decomposed the asymptotic complexity of `deleteVertex()`:
    1. Descriptor dictionary maintenance: $\mathcal{O}(\log q)$ using `std::map<int, int>` (at most 4 operations).
    2. Child relocation and parent-pointer updates: $\mathcal{O}(|C(u)|)$ time ($\le \mathcal{O}(\text{deg}(u))$).
    3. Structural distance-tree updates: $\mathcal{O}(1)$ pointer updates.
    4. Interval vector compaction: $\mathcal{O}(n)$ time via `std::remove_if` on `std::vector<Interval>`.
    5. Complete public deletion method: $\mathcal{O}(n + |C(u)| + \log q)$.
  - Documented that measured telemetry includes the linear vector compaction, explaining the $3.25\ \mu\text{s}$ latency at $n=5000$.
  - Updated `FINAL_IMPLEMENTATION_REPORT.md`, `FINAL_THESIS_READY_REPORT.md`, and `FINAL_MATHEMATICAL_PROOFS.md`.

### 3.3 Proof Dependencies and Attribution Transparency
- **Defect Identified:** Theorems 1 and 3 relied on external thesis results (Lemmas 3.4, 3.8, 3.15, Theorem 3.16) without a clear separation of independently proved components versus inherited results.
- **Corrections Executed:**
  - Added Section 5 to `FINAL_MATHEMATICAL_PROOFS.md` cataloging proof dependencies into Independently Proved, Inherited from Thesis, and Underlying Assumptions.
  - Clarified that while inherited lemmas are 100% verified by codebase regression tests, their axiomatic theoretical proofs reside in Chapter 3 of the BTP thesis document.

### 3.4 Accurate Description of Phase 9 Test 24
- **Defect Identified:** Test 24 was titled `testDecTree_24_SmallGraphExhaustiveOracle` and described as "exhaustive random deletions," whereas it deterministically iterates 20 parameterized instances ($n \in [5, 8]$).
- **Corrections Executed:**
  - Renamed function to `testDecTree_24_SmallGraphDeterministicDeletions()` in `tests/test_main.cpp`.
  - Updated console output to `"Small-Graph Deterministic Parameterized Deletions Suite"`.
  - Accurately described its parameters and verification against independent static rebuild in `FINAL_VERIFICATION_REPORT.md` and `FINAL_IMPLEMENTATION_REPORT.md`.
  - Rebuilt and verified `test_runner.exe` with 100% pass across all 56 tests.

---

## 4. Stage 1.8 — Final Documentation Consistency Patch & Closeout

### 4.1 Break-Even Model 2 Harmonization
- Corrected the summary in `FINAL_BENCHMARK_REPORT.md`, `FINAL_CLOSURE_REPORT.md`, and `FINAL_THESIS_READY_REPORT.md` to state $K^* = 4$ operations across all tested graph sizes ($n = 50, 1000, 5000$).
- Fully eliminated the contradictory "4 to 5 operations" phrasing.

### 4.2 Verification Counts Reconciliation
- **14 Families vs. 17 Configurations:** Reconciled documentation across `FINAL_VERIFICATION_REPORT.md`, `FINAL_CLOSURE_REPORT.md`, and `FINAL_IMPLEMENTATION_REPORT.md`. Explicitly documented that the 14 conceptual graph families in `graph_generator.h/.cpp` expand into 17 concrete test graph configurations per size in `benchmark_runner.cpp` (due to 3 overlap variants for Family 1 and 2 seeds for Family 3), generating $17 \times 4 = 68$ static test units.
- **100 vs. 165 Dynamic Updates:** Clarified that the formal verification harness (`clique_verifier.cpp` / `verification_results.csv`) executes exactly **165 step-by-step dynamic operations** across 9 test sequences (60 insertions, 45 deletions, 60 mixed operations across 3 seeds), while the performance benchmark runner uses sequences of 100 dynamic operations to measure execution latency.

### 4.3 Proof Dependencies and Implementation Consistency Qualification
- Updated Section 5.3 in `FINAL_MATHEMATICAL_PROOFS.md`, Section 5 in `FINAL_VERIFICATION_REPORT.md`, and Section 6 in `FINAL_THESIS_READY_REPORT.md`.
- Explicitly stated that codebase regression tests establish software implementation consistency, not formal mathematical proofs.
- Documented that inherited lemmas (Lemmas 3.1–3.4, 3.8, 3.15, Theorem 3.16) rely entirely on the original proofs in Chapter 3 of `BTP_Thesis (1).pdf` and were not independently cross-checked or re-derived if the thesis source is unavailable.

### 4.4 Qualified Conclusion and Elimination of Absolute Claims
- Replaced absolute statements such as "fully mathematically proven" or "unqualified statement" with precise, rigorous claims regarding software implementation, empirical validation, and thesis readiness under documented scope boundaries.
- Synchronized all audit deliverables in `benchmarks/maximal_cliques/FINAL_RESEARCH_AUDIT/`.
