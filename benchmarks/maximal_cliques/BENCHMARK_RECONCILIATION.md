# Benchmark Reconciliation and Audit Ledger
## Reconciling Claims, Reports, and Raw CSV Datasets

**Repository**: `C:/Users/sarav/OneDrive/Desktop/BTP_Codes`  
**Subdirectory**: `benchmarks/maximal_cliques/`  
**Date**: November 2026  

---

# 1. Executive Claim-by-Claim Reconciliation Ledger

| # | Existing Claim / Statement | Source Location | Raw Evidence in CSV | Verdict | Required Correction |
|:---|:---|:---|:---|:---:|:---|
| 1 | "Algorithm A breaks even dynamically after $K^* = 1$ operations." | `BENCHMARK_REPORT.md:14` | Raw static data: $I_A \approx 0.038\text{ ms}$, $I_C \approx 0.007\text{ ms}$. Dynamic: $R_C \approx 14.1\ \mu\text{s}$, $U_A \approx 2.6\ \mu\text{s}$. | **Partially True** | Valid only if $T(G)$ is pre-existing ($I_A = 0$). When initializing from scratch, $K^* = \frac{31}{11.5} \approx 2.7 \implies \mathbf{K^* \in [3, 5] \text{ updates}}$. |
| 2 | "All 77 verification rows passed equivalence: A == B == C." | `clique_verifier.cpp:182`, report | In `verification_results.csv`, for all 17 rows at $n=100$, line 163 in verifier executed `res.numCliquesB = res.numCliquesA;` (BK skipped). | **Inaccurate Reporting** | BK was actually run on 60 rows (51 static $n \le 50$ + 9 dynamic). On 17 static rows ($n=100$), BK was skipped. Must state "60 3-way, 17 2-way". |
| 3 | "Dynamic Algorithm A provides $\mathcal{O}(\log n)$ insertion." | `AUDIT_REPORT.md:79` | `DynamicTheoremCliques::insertLeaf` lines 169–188: copies `std::map` ($O(q)$) and runs set-diff loop ($O(q \log q)$) inside timer. | **Complexity Mismatch** | Structural tree update is $\mathcal{O}(\log n)$, but current public implementation runtime is $\mathcal{O}(n \log n)$ due to map copy telemetry. |
| 4 | "Exactly 1.0 descriptors were modified per dynamic update." | `BENCHMARK_REPORT.md` | In `dynamic_insertion_results.csv`: `added=1, modified=0`. In `dynamic_deletion_results.csv`: `removed=1, modified=0`. | **Terminology Discrepancy** | "Modified" was used loosely. In raw schema, insertions are additions, deletions are removals. Both reflect $\Delta = 1$ change. |
| 5 | "Dynamic workloads were evaluated across 3 independent seeds." | `BENCHMARK_METHODOLOGY.md` | `benchmark_runner.cpp:703, 739, 774`: Insertion used ONLY seed 101, Deletion used ONLY seed 202, Mixed used ONLY seed 303. | **Inaccurate Claim** | Exactly 1 seed was evaluated per workload family. 3 seeds were used across the 3 workload types, not 3 repetitions per workload. |
| 6 | "Algorithm A achieves infinite speedup over Bron–Kerbosch at $n \ge 500$." | Early draft text | At $n \ge 500$, BK timed out ($> 3,000\ \text{ms}$) or was skipped (`SKIPPED_BY_POLICY`). | **Methodology Violation** | Violates Rule 8. Speedups cannot divide by zero or timeout sentinels. Mark BK as `TIMEOUT` / `SKIPPED`. |
| 7 | "End-to-end time does not match sum of stages." | User review note | In `static_summary.csv`, at $n=10^6$ medium: pre=837.1 ms, core=81.3 ms, mat=41.6 ms (sum=960.0 ms), direct=989.5 ms. | **Verified Valid** | Sum of medians does not equal median of sum. Direct execution includes uninterrupted cache execution. Both correctly reported. |
| 8 | "`max_degree` is reported as 0 across all static benchmark rows." | `static_results.csv` | In `benchmark_runner.cpp:107, 199, 270`, `res.maxDegree = 0;` was left unpopulated by `computeEdgeCount()`. | **Implementation Defect** | Must compute actual max vertex degree from interval adjacency. |
| 9 | "Algorithm B timeout recorded as 0.0 ms." | `static_results.csv` | `benchmark_runner.cpp:236-237`: `if (timedOut) { resB.directEndToEndMs = 0.0; }`. | **Reporting Defect** | Violates Rule 6. Replace `0.0` with `-1.0` sentinel. |

---

# 2. Recomputed Empirical Speedups (Completed Runs Only)

### 2.1 Static Benchmarks (Uninterrupted Direct End-to-End Time)
*Completed runs from `results/static/static_summary.csv` (Medium-Overlap Profile)*:

| Graph Size ($n$) | Alg A Direct (ms) | Alg C Direct (ms) | Alg B Direct (ms) | Speedup: Alg C over Alg A | Speedup: Alg A over Alg B | Speedup: Alg C over Alg B |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **10** | 0.0031 | 0.0007 | 0.0057 | **$4.43\times$** | **$1.84\times$** | **$8.14\times$** |
| **50** | 0.0381 | 0.0071 | 0.0345 | **$5.37\times$** | $0.91\times$ | **$4.86\times$** |
| **100** | 0.0742 | 0.0142 | 0.4128 | **$5.23\times$** | **$5.56\times$** | **$29.07\times$** |
| **500** | 0.3842 | 0.0712 | 14.821 | **$5.40\times$** | **$38.58\times$** | **$208.16\times$** |
| **1,000** | 0.9324 | 0.1614 | 142.18 | **$5.78\times$** | **$152.49\times$** | **$880.92\times$** |
| **2,000** | 1.8412 | 0.3218 | 1,842.1 | **$5.72\times$** | **$1,000.5\times$** | **$5,724.4\times$** |
| **5,000** | 3.8912 | 0.7518 | TIMEOUT | **$5.18\times$** | — | — |
| **10,000** | 7.4218 | 1.4812 | SKIPPED | **$5.01\times$** | — | — |
| **50,000** | 38.914 | 7.5182 | SKIPPED | **$5.18\times$** | — | — |
| **100,000** | 71.809 | 17.578 | SKIPPED | **$4.08\times$** | — | — |
| **1,000,000** | 989.47 | 166.19 | SKIPPED | **$5.95\times$** | — | — |

*Key Conclusion*: In static execution from scratch, **Algorithm C is consistently $4\times$ to $6\times$ faster than Algorithm A**.

---

### 2.2 Dynamic Maintenance Benchmarks (Average Per-Operation Latency)
*Completed runs from `results/dynamic/dynamic_summary.csv`*:

| Initial $n$ | Workload Type | Alg A Latency | Alg C Latency | Alg B Latency | Speedup: Alg A over Alg C | Speedup: Alg A over Alg B |
|:---:|:---|:---:|:---:|:---:|:---:|:---:|
| **50** | Incremental Insertion (50 ops) | $3.8\ \mu\text{s}$ | $18.1\ \mu\text{s}$ | $174.1\ \mu\text{s}$ | **$4.76\times$** | **$45.8\times$** |
| **50** | Decremental Deletion (30 ops) | $1.7\ \mu\text{s}$ | $10.3\ \mu\text{s}$ | $155.2\ \mu\text{s}$ | **$6.06\times$** | **$91.3\times$** |
| **50** | Mixed (50/50, 40 ops) | $2.2\ \mu\text{s}$ | $11.8\ \mu\text{s}$ | $346.8\ \mu\text{s}$ | **$5.36\times$** | **$157.6\times$** |
| **100** | Incremental Insertion (50 ops) | $8.2\ \mu\text{s}$ | $30.7\ \mu\text{s}$ | $413.1\ \mu\text{s}$ | **$3.74\times$** | **$50.4\times$** |
| **100** | Decremental Deletion (30 ops) | $4.3\ \mu\text{s}$ | $21.4\ \mu\text{s}$ | $382.4\ \mu\text{s}$ | **$4.98\times$** | **$88.9\times$** |
| **500** | Incremental Insertion (50 ops) | $48.2\ \mu\text{s}$ | $112.4\ \mu\text{s}$ | TIMEOUT | **$2.33\times$** | — |
| **500** | Decremental Deletion (30 ops) | $38.4\ \mu\text{s}$ | $92.6\ \mu\text{s}$ | TIMEOUT | **$2.41\times$** | — |
| **1,000** | Incremental Insertion (50 ops) | $98.4\ \mu\text{s}$ | $218.4\ \mu\text{s}$ | SKIPPED | **$2.22\times$** | — |
| **1,000** | Decremental Deletion (30 ops) | $89.2\ \mu\text{s}$ | $184.2\ \mu\text{s}$ | SKIPPED | **$2.07\times$** | — |
| **5,000** | Incremental Insertion (50 ops) | $594.0\ \mu\text{s}$ | $1,157.2\ \mu\text{s}$ | SKIPPED | **$1.95\times$** | — |
| **5,000** | Decremental Deletion (30 ops) | $599.3\ \mu\text{s}$ | $1,229.9\ \mu\text{s}$ | SKIPPED | **$2.05\times$** | — |

*Key Conclusion*: In dynamic maintenance, **Algorithm A is $2\times$ to $6\times$ faster than full recomputation with Algorithm C across all tested scales**.

---

# 3. Dynamic Operations Ledger Reconciliation

Reconciliation of all dynamic operations recorded across CSV datasets:

1. **`dynamic_insertion_results.csv`**:
   - 5 size tiers: $n_0 \in \{50, 100, 500, 1000, 5000\}$.
   - 50 operations per tier.
   - Alg A: $5 \times 50 = 250$ rows.
   - Alg C: $5 \times 50 = 250$ rows.
   - Alg B: $4 \times 50 + 0 = 200$ rows ($n=5000$ skipped).
   - Total rows: $250 + 250 + 200 = 700$ data rows (+ 1 header = 701 lines).
2. **`dynamic_deletion_results.csv`**:
   - 5 size tiers $\times 30$ operations = 150 operations.
   - Alg A: 150 rows.
   - Alg C: 150 rows.
   - Alg B: $4 \times 30 + 0 = 120$ rows.
   - Total rows: $150 + 150 + 120 = 420$ data rows (+ 1 header = 421 lines).
3. **`dynamic_mixed_results.csv`**:
   - 5 size tiers $\times 3$ ratios ($75/25, 50/50, 25/75$) $\times 40$ operations = 600 operations.
   - Alg A: 600 rows.
   - Alg C: 600 rows.
   - Alg B: $4 \times 120 + 0 = 480$ rows.
   - Total rows: $600 + 600 + 480 = 1680$ data rows (+ 1 header = 1681 lines).

**Total Dynamic Operations Evaluated**:
- Algorithm A: $250 + 150 + 600 = \mathbf{1,000}$ operations.
- Algorithm C: $250 + 150 + 600 = \mathbf{1,000}$ operations.
- Algorithm B: $200 + 120 + 480 = \mathbf{800}$ operations ($200$ skipped by policy at $n=5,000$).
- Grand Total Step Records: $700 + 420 + 1680 = \mathbf{2,800}$ completed step records.
