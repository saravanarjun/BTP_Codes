# BTP Maximal Clique Experimental Benchmarking: Methodology

**Project:** Dynamic Graph Algorithms – Interval Graphs  
**Benchmark Target:** Empirical Evaluation of Maximal Clique Algorithms on Connected Proper Interval Graphs  
**Location:** `benchmarks/maximal_cliques/`  
**Date:** November 2026 (Revised & Corrected Edition)  
**Baseline Preservation:** Original unadjusted baseline run preserved in `benchmarks/maximal_cliques/baseline_run/`  

---

## 1. Objective and Research Questions

The objective of this experimental evaluation is to rigorously evaluate the empirical execution time, memory scalability, and practical behavior of three maximal-clique algorithms on connected proper interval graphs:

1. **Algorithm A (Theorem-Based / Thesis Proposed):**
   - Construction based on Chapter 3, Lemma 3.8 ($C(u)$ node cliques) and Lemma 3.15 (compact representation $[\ell_p, \ell_c]$ for parent nodes $p$).
   - Pruning non-leaf parent nodes in the distance tree $T(G)$, preserving the documented leaf qualification.
2. **Algorithm B (General Graph Baseline — Bron–Kerbosch with Pivoting):**
   - The recursive depth-first backtracking enumerator from Phase 5 using the Tomita et al. pivot selection heuristic ($u \in P \cup X$ maximizing $|P \cap N(u)|$).
   - Graph-agnostic, checking edge existence via interval intersection.
3. **Algorithm C (Independent Domain Standard — Interval Sweep-Line):**
   - A specialized sweep-line algorithm operating directly on sorted interval endpoints.
   - Independent of the distance tree, Lemma 3.8, and Lemma 3.15.
   - Respects closed intervals by processing interval START events before END events at equal coordinates.

### Core Research Questions
- **RQ1 (Asymptotic Verification):** Does Algorithm A's core identification run in strictly linear $\mathcal{O}(n)$ time across all graph overlap profiles?
- **RQ2 (Core vs. Materialization):** What is the exact empirical boundary where explicit membership expansion ($\mathcal{O}(S)$, $S = \sum |C_j|$) dominates the core clique identification time?
- **RQ3 (Scalability Horizon):** At what graph size $n$ does general backtracking (Bron–Kerbosch) hit practical wall-clock limits compared to $\mathcal{O}(n)$ and $\mathcal{O}(n \log n)$ specialized algorithms?
- **RQ4 (Overlap Sensitivity):** How do graph overlap profiles (sparse-overlap, medium-overlap, dense-overlap) influence clique count $q$, average degree, and total memberships $S$?

---

## 2. Timing Methodology and Explicit Stage Separation

To ensure a fair and rigorous comparison among algorithms that produce compact descriptors versus those that materialize explicit vertex sets, all timing measurements explicitly distinguish between preprocessing, core identification, and output materialization.

### 2.1 Metric Definitions

| Metric Field | Definition | Mathematical Nature |
| :--- | :--- | :--- |
| `preprocessing_ms` | Time required to prepare internal data structures prior to clique extraction (e.g. Distance Tree construction for Alg A; endpoint event sorting for Alg C). | $\mathcal{O}(n)$ for Alg A; $\mathcal{O}(n \log n)$ for Alg C; $\mathcal{O}(1)$ for Alg B. |
| `core_ms` | Time required to identify the complete family of maximal cliques in compact descriptor format. | $\mathcal{O}(n)$ for Alg A and Alg C. |
| `materialization_ms` | Time required to allocate and populate explicit `std::vector<int>` vertex ID sets for all $q$ maximal cliques. | $\mathcal{O}(S)$ where $S = \sum_{j=1}^q \|C_j\|$. |
| `postprocessing_total_ms` | Post-preprocessing algorithm execution: $\text{core\_ms} + \text{materialization\_ms}$. | Useful for isolating pure clique extraction when preprocessing is amortized. |
| `end_to_end_sum_ms` | Sum of separately timed stages: $\text{preprocessing\_ms} + \text{core\_ms} + \text{materialization\_ms}$. | Validates stage measurement consistency. |
| `direct_end_to_end_ms` | **Directly timed continuous execution:** Single uninterrupted timing block from graph input to fully materialized cliques. | Primary reported value for complete end-to-end performance. |

### 2.2 Algorithm Preprocessing Requirements Explained
- **Algorithm A:** Requires the Distance Tree $T(G)$. Preprocessing builds levels, parent pointers, children lists, and traversal sequences via a monotonic two-pointer sweep in $\mathcal{O}(n)$ time.
- **Algorithm B:** Does not require structural preprocessing; begins with candidate set $P = \{1, \dots, n\}$.
- **Algorithm C:** Requires sorting the $2n$ interval endpoints. Preprocessing extracts START and END events and sorts them in $\mathcal{O}(n \log n)$ time.
- *(Note on sorting: All algorithms are supplied with the graph's canonical sorted intervals. If raw intervals were unsorted, an initial $\mathcal{O}(n \log n)$ sort would be common to all).*

### 2.3 Statistical Protocol
- **Warm-Up:** 1 unrecorded warm-up run per benchmark configuration to prime CPU instruction pipelines and caches.
- **Repetitions:** 20 reps ($n \le 100$), 10 reps ($200 \le n \le 1,000$), 5 reps ($2,000 \le n \le 10,000$), 3 reps ($n \ge 20,000$).
- **Estimator:** Median execution time (`median_end_to_end_ms`) reported as primary robust metric, supplemented by minimum and maximum values.

---

## 3. Algorithm Architectures and Theoretical Complexity

### 3.1 Algorithm A — Theorem-Based (Lemma 3.8 & Lemma 3.15)
- **Foundation:**
  - **Lemma 3.8:** For any node $u \in T(G)$, the set $C(u)$ forms a clique. For leaves, $C(u)$ is non-maximal and properly contained in its parent's clique.
  - **Lemma 3.15:** Every maximal clique corresponds to a non-leaf parent node $p$ and its rightmost child $c \in C(p)$. The maximal clique is compactly defined as $[\ell_p, \ell_c]$, representing $\{ v \in V \mid \ell_v \in [\ell_p, \ell_c] \}$.
- **Complexity Breakdown:**
  - Preprocessing: $\mathcal{O}(n)$ time to construct $T(G)$.
  - Core: $\mathcal{O}(n)$ time to scan non-leaf parents and record $[\ell_p, \ell_c]$.
  - Output Storage: Compact descriptors require $\mathcal{O}(q)$ storage, where $q$ is the number of maximal cliques ($q \le n$).
  - Materialization: Expanding each descriptor into explicit vertex lists requires $\mathcal{O}(S)$ time and memory, where $S = \sum_{j=1}^q |C_j|$.
  - **Total End-to-End Complexity:** $\mathcal{O}(n + S)$. Compact descriptors avoid materialization only when downstream consumers can query descriptors directly.

### 3.2 Algorithm B — Bron–Kerbosch with Pivoting (Tomita Variant)
- **Foundation:**
  - Recursive backtracking maintaining disjoint sets $(R, P, X)$.
  - Pivot $u \in P \cup X$ chosen maximizing $|P \cap N(u)|$; branches restricted to $v \in P \setminus N(\text{pivot})$.
- **Adjacency:** Dynamic $\mathcal{O}(1)$ interval intersection check avoiding $n \times n$ adjacency matrix allocation.
- **Complexity:** Worst-case $\mathcal{O}(3^{n/3})$ on general graphs. On bounded-overlap proper interval graphs, candidate filtering per node takes $\mathcal{O}(|P \cup X| \cdot |P|)$, causing steep polynomial runtime.
- **Timeout Policy:** 3,000 ms wall-clock limit, polled every 500 calls. Cases exceeding the limit are classified as `TIMEOUT` (never plotted as zero).

### 3.3 Algorithm C — Standard Interval Sweep-Line
- **Foundation:**
  - By Helly's property, every maximal clique in an interval graph corresponds to the intersection of intervals at some point $x \in \mathbb{R}$.
  - Over closed intervals, active sets change only at endpoints. Sorting $2n$ endpoints and sweeping from left to right discovers all maximal cliques.
- **Complexity Breakdown:**
  - Preprocessing: Generating and sorting $2n$ events takes $\mathcal{O}(n \log n)$.
  - Core: Active list insertion and deletion take $\mathcal{O}(1)$ per event via doubly linked list with iterator handles; total core sweep takes $2n \times \mathcal{O}(1) = \mathcal{O}(n)$.
  - Materialization: Expanding contiguous active index ranges takes $\mathcal{O}(S)$ time.
  - **Total End-to-End Complexity:** $\mathcal{O}(n \log n + S)$.

---

## 4. Mathematical Correctness Proof of Algorithm C (Sweep-Line)

Let $G = (V, E)$ be a connected proper interval graph with closed intervals $I_i = [\ell_i, r_i]$. Let the $2n$ endpoint events be sorted such that for equal coordinates, START events strictly precede END events, with deterministic ID tie-breaking.

### Claim 1: Each Reported Active Set is a Clique
- **Proof:** A maximal clique candidate is recorded at the precise moment an END event occurs for some interval $u$. At this moment, every interval $v$ currently in the active set has started ($\ell_v \le r_u$) and has not yet ended ($r_v \ge r_u$). Therefore, $r_u \in I_v$ for every $v \in \text{active}$. Since all active intervals contain the common point $r_u \in \mathbb{R}$, by Helly's property for intervals, they are pairwise intersecting. Hence, the active set is a clique. $\blacksquare$

### Claim 2: Every Maximal Clique is Reported
- **Proof:** In an interval graph, every maximal clique $K$ is realized as the set of intervals containing some point $x \in \mathbb{R}$. As $x$ increases, active intervals change only at endpoints. Let $x^* = \min_{v \in K} r_v$ be the earliest right endpoint among all intervals in $K$. Because $K$ is a clique, all intervals in $K$ contain $x^*$, so $K \subseteq \text{active}(x^*)$. By maximality of $K$, no additional interval can be added, so $K = \text{active}(x^*)$. At the END event corresponding to $x^*$, all intervals starting at $x^*$ have already been processed (starts before ends), so $\text{active}$ precisely equals $K$. Furthermore, since $K$ contains intervals that were not present in previous cliques, at least one START event occurred before $x^*$, triggering the `hasNewStart` condition. Therefore, $K$ is reported. $\blacksquare$

### Claim 3: No Non-Maximal Clique is Reported
- **Proof:** An active set is emitted only when `hasNewStart == true` at an END event $e_k$. 
  - If no START event occurred between END event $e_{k-1}$ and $e_k$, then the active set at $e_k$ is obtained from the active set at $e_{k-1}$ by deleting intervals that have ended. Thus, $\text{active}(e_k) \subset \text{active}(e_{k-1})$ is a strict proper subset. Our algorithm skips $e_k$ because `hasNewStart` is false, eliminating all non-maximal sub-cliques.
  - If a START event occurred between $e_{k-1}$ and $e_k$, let $w$ be the newly started interval. Then $w \in \text{active}(e_k) \setminus \text{active}(e_{k-1})$, so $\text{active}(e_k) \not\subset \text{active}(e_{k-1})$. Moreover, the departing interval $u$ at $e_k$ satisfies $u \notin \text{active}(e_m)$ for any later event $e_m > e_k$. Hence, $\text{active}(e_k)$ is not contained in any earlier or later active set. By Helly's property, it is maximal. $\blacksquare$

### Claim 4: Duplicate Cliques are Avoided
- **Proof:** In proper interval graphs, intervals are ordered monotonically by left and right endpoints. Every maximal clique emitted has a unique leftmost departing interval or distinct new arrival. Since `hasNewStart` is reset to false immediately upon recording a clique, multiple intervals ending at the exact same coordinate $r$ emit only the first (largest) active set and skip all subsequent subsets. Hence, duplicate cliques are strictly avoided. $\blacksquare$

### Claim 5: Closed Endpoint Touching is Handled Correctly
- **Proof:** If interval $u$ ends at coordinate $x$ ($r_u = x$) and interval $v$ starts at $x$ ($\ell_v = x$), the closed interval convention dictates that $I_u \cap I_v = \{x\} \ne \emptyset$. Because our event ordering sorts START before END at equal coordinates, START($v$) is processed before END($u$). Consequently, $v$ enters the active set before $u$ leaves, ensuring both $u$ and $v$ are simultaneously active when the clique at $x$ is recorded. $\blacksquare$

---

## 5. Expanded Graph Families for Validation

To avoid claiming correctness based solely on equal-length synthetic graphs, the validation harness evaluates five diverse graph families:

1. **Family 1: Equal-Length Bounded-Overlap Profiles**
   - $I_i = [i \cdot d, i \cdot d + W]$ ($0 < d \le W$).
   - Sparse-Overlap ($W/d = 1.5, k = 1$), Medium-Overlap ($W/d = 8.0, k = 8$), Dense-Overlap ($W/d = 25.0, k = 25$).
2. **Family 2: Unequal Interval Lengths**
   - Intervals generated with varying widths $L_i \in [3.0, 8.0]$ and steps $\Delta \ell \in [0.8, 2.0]$.
   - Strict properness enforced via strictly increasing right endpoints ($r_{i+1} > r_i$).
3. **Family 3: Identical Intervals Clusters**
   - Clusters of 2 to 3 duplicate intervals sharing identical left and right endpoints ($\ell_i = \ell_{i+1}, r_i = r_{i+1}$).
   - Validates tie-breaking by vertex ID and confirms duplicate cliques are not generated.
4. **Family 4: Endpoint-Touching Intervals**
   - Intervals satisfying $r_i = \ell_{i+1}$ (touching at boundary points).
   - Validates closed-interval intersection and single-point clique formation.
5. **Family 5: Randomized Reproducible Proper Interval Graphs**
   - Generated with pseudo-random seed values across multiple graph orders.
   - Evaluates non-uniform degree distributions and irregular clique sizes.

---

## 6. Graph Density Terminology Clarification

In our equal-length benchmark family, interval width $W$ and step $d$ are constant parameters independent of $n$.
- Effective bandwidth $k = \lfloor W / d \rfloor$ is constant.
- Total edges $m = (n - k)k + \frac{k(k-1)}{2} = \mathcal{O}(k \cdot n) = \mathcal{O}(n)$.
- Average degree is:
  $$\bar{d} = \frac{2m}{n} \approx 2k$$
  (Sparse-overlap: $\bar{d} \approx 2.0$; Medium-overlap: $\bar{d} \approx 16.0$; Dense-overlap: $\bar{d} \approx 50.0$).
- **Clarification:** These graphs are **bounded-degree / bounded-bandwidth graphs with linear edge count $\mathcal{O}(n)$**. The term *"dense-overlap"* refers to high local interval overlap and large clique size ($|C_j| = 26$), **not** asymptotic quadratic edge density $\Theta(n^2)$.
