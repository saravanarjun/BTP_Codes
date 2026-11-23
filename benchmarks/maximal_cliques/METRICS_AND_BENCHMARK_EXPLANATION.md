# Comprehensive Metrics and Benchmark Explanation Guide
## Understanding Maximal Clique Measurements and Telemetry

**Repository**: `C:/Users/sarav/OneDrive/Desktop/BTP_Codes`  
**Subdirectory**: `benchmarks/maximal_cliques/`  
**Audience**: Thesis Committee, Research Examiners, and Peer Reviewers  
**Date**: November 2026  

---

# 1. Introduction

This guide provides an exhaustive, mathematically rigorous, and accessible explanation of every metric, column, timer, and telemetry field used in the static and dynamic maximal clique benchmarks. 

---

# 2. Fundamental Structural Concepts

### 2.1 Number of Maximal Cliques ($q$) vs. Total Memberships ($S$)
In graph theory, a **clique** is a subset of vertices that are mutually adjacent. A clique is **maximal** if it cannot be extended by adding another vertex from the graph.
- **$q$ (Number of Maximal Cliques)**:
  $$q = |\mathcal{M}(G)|$$
  This is the count of distinct maximal cliques in the graph. In a proper interval graph with $n$ vertices, $q$ is strictly bounded by $n$:
  $$1 \le q \le n$$
- **$S$ (Total Memberships / Total Output Size)**:
  $$S = \sum_{C \in \mathcal{M}(G)} |C|$$
  This represents the sum of the cardinalities of all maximal cliques. A single vertex may belong to multiple maximal cliques.
- **Why this distinction matters**:
  - Compact descriptors represent the $q$ cliques in $\mathcal{O}(q)$ space.
  - Materializing the full vertex lists requires $\mathcal{O}(S)$ space.
  - In dense graphs or high-overlap graphs, while $q \le n$, $S$ can be $\Theta(n \cdot \omega(G)) \le \Theta(n^2)$.

---

### 2.2 Compact Descriptors vs. Explicit Cliques
- **Compact Descriptor (Lemma 3.15)**:
  Each maximal clique is characterized by a pair of boundary coordinates $[l_p, l_{c_p}]$, where $p$ is a non-leaf parent in the distance tree $T(G)$ and $c_p$ is its rightmost child.
  - Storage: Storing two vertex IDs or two double bounds takes $\mathcal{O}(1)$ space per clique, yielding $\mathcal{O}(q)$ total space.
- **Explicit Materialization**:
  The actual list of vertex IDs belonging to the clique:
  $$C(p) = \{ v \in V : l_p \le l_v \le l_{c_p} \}$$
  Expanding this set requires enumerating every member, incurring $\mathcal{O}(|C(p)|)$ work.

---

# 3. Static Benchmark Metrics Dictionary

| Column Name | Metric / Formula | Unit | Measurement Boundary | Aggregation Method | Limitations / Nuance |
|:---|:---|:---:|:---|:---:|:---|
| `algorithm` | Algorithmic Approach | Text | Identification string | N/A | Algorithm A, B, or C |
| `variant` | Specific algorithmic strategy | Text | Variant identifier | N/A | Compact, Pivoted, or Sweep |
| `graph_family` | Structural graph category | Text | Graph topology descriptor | N/A | 16 experimental configs |
| `n` | Vertex count $|V|$ | Count | Total intervals in graph | Exact | Must be $> 0$ |
| `m` | Edge count $|E|$ | Count | All intersecting interval pairs | Exact ($\mathcal{O}(n)$ sweep) | $m \le n(n-1)/2$ |
| `average_degree` | $2m / n$ | Real | Mean edges per vertex | Exact calculation | Indicates graph density |
| `max_degree` | $\max_{v \in V} \text{deg}(v)$ | Count | Maximum neighbor count | Exact | Was uncalculated in early runs |
| `q` | Maximal clique count | Count | Output collection size | Exact count | $q \le n$ on proper graphs |
| `total_memberships` | $S = \sum |C_j|$ | Count | Sum of all clique sizes | Exact sum | Can scale up to $\mathcal{O}(n^2)$ |
| `average_clique_size` | $S / q$ | Real | Mean vertices per clique | Exact division | Reflects overlap width |
| `max_clique_size` | $\omega(G) = \max |C_j|$ | Count | Clique number of $G$ | Exact maximum | Size of largest clique |
| `status` | Execution outcome | Enum | Status enum | Direct | `SUCCESS`, `TIMEOUT`, etc. |
| `preprocessing_ms` | Initial setup time | ms | Distance tree or event sort | Median over repetitions | Excludes core algorithm |
| `core_ms` | Core identification time | ms | Descriptor generation / sweep | Median over repetitions | Pure clique discovery |
| `materialization_ms` | Explicit expansion time | ms | Expanding descriptors to sets | Median over repetitions | Depends directly on $S$ |
| `postprocessing_total_ms` | `core_ms` + `materialization_ms` | ms | Sum of core and expansion | Derived sum | Excludes preprocessing |
| `end_to_end_sum_ms` | `pre` + `core` + `mat` | ms | Sum of all three stages | Derived sum | Sum of stage medians |
| `direct_end_to_end_ms` | Unbroken continuous execution | ms | Single timer start to finish | Median over repetitions | Gold standard for total time |
| `min_direct_e2e_ms` | Minimum continuous time | ms | Single timer across reps | Minimum value | Best-case execution |
| `max_direct_e2e_ms` | Maximum continuous time | ms | Single timer across reps | Maximum value | Worst-case execution / jitter |
| `repetitions` | Sample count | Count | Repetition loop count | Integer | Adaptive based on $n$ |
| `estimated_descriptor_bytes` | $q \times 2 \times \text{sizeof}(\text{int})$ | Bytes | $8q$ bytes | Analytical calculation | Payload only (no map overhead) |
| `estimated_explicit_bytes` | $S \times \text{sizeof}(\text{int})$ | Bytes | $4S$ bytes | Analytical calculation | Payload only (no vector overhead) |

---

# 4. Stage Timing vs. Direct End-to-End Timing

In `static_results.csv`, two different end-to-end metrics are reported:
1. **`end_to_end_sum_ms`**:
   $$\text{end\_to\_end\_sum\_ms} = \text{median}(\text{pre}) + \text{median}(\text{core}) + \text{median}(\text{mat})$$
2. **`direct_end_to_end_ms`**:
   $$\text{direct\_end\_to\_end\_ms} = \text{median}(\text{direct})$$
   where `direct` measures the entire algorithm in a single, uninterrupted `steady_clock` block.

### Why do they differ slightly?
- **Non-additivity of Medians**: In statistics, the sum of medians of separate random variables is not mathematically equal to the median of their sum:
  $$\text{median}(X + Y + Z) \ne \text{median}(X) + \text{median}(Y) + \text{median}(Z)$$
- **Cache Warming & Hardware State**: In the separated timer loop, the processor executes preprocessing, stops the timer, executes core, stops the timer, and executes materialization. In the direct timer, execution flows without intermediate timer calls, benefiting from instruction cache continuity.
- **Reporting Convention**: Both numbers are reported to provide full transparency into stage contributions while preserving uninterrupted system execution measurements.

---

# 5. Dynamic Benchmark Metrics Dictionary

| Column Name | Metric / Meaning | Unit | Measurement Scope | Formula / Source |
|:---|:---|:---:|:---|:---|
| `algorithm` | Dynamic Maintenance Strategy | Text | Implementation class | Algorithm A, B, or C |
| `variant` | Implementation variant | Text | Specific engine | Relocation, Recomputation |
| `workload` | Workload sequence type | Text | Operation distribution | Insertion, Deletion, Mixed |
| `graph_family` | Initial graph family | Text | Generator name | Initial topology |
| `initial_n` | Initial vertex count | Count | Before sequence begins | Graph order $n_0$ |
| `final_n` | Final vertex count | Count | After sequence finishes | Graph order $n_K$ |
| `seed` | Deterministic random seed | Integer | Generator seed | Seed value (101, 202, 303) |
| `update_index` | Sequence step index | Count | Step index $0 \le i < K$ | Loop counter |
| `op_type` | Operation executed | Enum | `INSERTION` or `DELETION` | Step type |
| `target_id` | Mutated vertex ID | Integer | Vertex identifier | Unique interval ID |
| `n_before` / `n_after` | Size before and after step | Count | Vertex counts | $|V_{i-1}|$ and $|V_i|$ |
| `q_before` / `q_after` | Clique count before/after | Count | Clique counts | $q_{i-1}$ and $q_i$ |
| `update_application_ms` | Input graph update time | ms | Graph modification | Adding/erasing interval |
| `algorithm_update_ms` | Algorithm update time | ms | Core structural update | Tree/clique maintenance |
| `total_latency_ms` | Step total execution time | ms | Full per-step latency | `update_application` + `alg_update` |
| `descriptors_added` | Descriptors newly created | Count | Set difference | Newly inserted parent keys |
| `descriptors_removed` | Descriptors deleted | Count | Set difference | Erased parent keys |
| `descriptors_modified` | Existing descriptors changed | Count | Set difference | Same parent, changed right child |
| `descriptors_preserved` | Descriptors unchanged | Count | Set difference | Identical key-value pairs |
| `status` | Step execution outcome | Enum | Step status | `SUCCESS`, `TIMEOUT`, etc. |

---

# 6. Statistical Latency Metrics

For a dynamic sequence of $K$ operations with measured latencies $\{t_1, t_2, \dots, t_K\}$:

1. **Cumulative Time (`cumulative_time_ms`)**:
   $$T_{\text{cum}} = \sum_{i=1}^K t_i$$
2. **Average Latency (`average_latency_ms`) / Amortized Latency**:
   $$\mu = \frac{1}{K} \sum_{i=1}^K t_i = \frac{T_{\text{cum}}}{K}$$
3. **Median Latency (`median_latency_ms`)**:
   The 50th percentile of sorted latencies. Robust against operating system scheduler spikes.
4. **Standard Deviation (`stddev_latency_ms`)**:
   $$\sigma = \sqrt{\frac{1}{K} \sum_{i=1}^K (t_i - \mu)^2}$$
   Measures latency consistency and jitter.
5. **90th and 95th Percentile Latencies (`p90_latency_ms`, `p95_latency_ms`)**:
   The value below which $90\%$ (or $95\%$) of update operations fall. Critical for real-time and interactive applications to guarantee tail latency bounds.

---

# 7. Telemetry Clarification: Additions, Removals, and Modifications

In previous reports, summaries frequently stated:
> "Exactly 1.0 descriptors were modified per dynamic update."

In technical parlance, the word "modified" was used casually to mean "affected" or "changed". However, in the raw CSV schema, these are four mutually exclusive categories:
- **`descriptors_added`**: A parent key that was not previously in the dictionary is inserted. (e.g., When a leaf becomes a parent upon leaf insertion).
- **`descriptors_removed`**: A parent key that was previously in the dictionary is erased. (e.g., When a parent vertex is deleted).
- **`descriptors_modified`**: A parent key remains in the dictionary, but its rightmost child ID changes. (e.g., When a parent acquires an additional child further right).
- **`descriptors_preserved`**: Both the parent key and its rightmost child ID are completely identical before and after the update.

In the raw CSV datasets:
- For incremental insertion sequences: `descriptors_added = 1`, `descriptors_removed = 0`, `descriptors_modified = 0`.
- For decremental deletion sequences: `descriptors_added = 0`, `descriptors_removed = 1`, `descriptors_modified = 0`.
Both reflect exactly **one descriptor change** per update, confirming the theoretical bound.

---

# 8. Timeout vs. Policy Skip

- **`TIMEOUT`**:
  The algorithm was actively executed, but its execution exceeded the configured safety wall-clock limit (e.g., $3,000\ \text{ms}$ or $60.0\ \text{s}$). Execution was aborted mid-run.
  - Reported time: Marked as `TIMEOUT` with latency recorded as `-1.0` (or the timeout threshold).
  - Never replaced with zero.
- **`SKIPPED_BY_POLICY`**:
  The algorithm was deliberately not launched because the graph size $n$ exceeded its empirically established scalability ceiling (e.g., $n > 2,000$ for Bron–Kerbosch).
  - Avoids hanging benchmark suites for hours on exponential algorithms.
  - Distinct from a timeout: no CPU cycles were consumed.
  - Latency is strictly recorded as `-1.0`.

---

# 9. Break-Even Analysis Cost Model

When comparing dynamic maintenance against static recomputation, two distinct deployment models must be considered:

### Model 1: Distance Tree Pre-Existing (Embedded System)
In many graph analytics pipelines, the Distance Tree $T(G)$ is already maintained by other algorithms (such as Maximum Independent Set or Breadth-First Search).
- Algorithm A initialization cost: $I_A = 0$.
- Algorithm C initialization cost: $I_C = 0$.
- Update costs: $U_A < R_C$.
- **Result**: Maintenance is faster on the very first operation:
  $$\mathbf{K^* = 1}$$

### Model 2: Standalone Execution From Scratch
If the system starts from a raw interval list and must initialize all structures:
$$T_A(K) = I_A + K \cdot U_A$$
$$T_C(K) = I_C + K \cdot R_C$$
where:
- $I_A$ is Algorithm A initialization time (Distance Tree build + compact descriptors).
- $I_C$ is Algorithm C initialization time (Event sorting + sweep).
- $U_A$ is Algorithm A's dynamic update latency.
- $R_C$ is Algorithm C's full recomputation latency.

The break-even update count $K^*$ is:
$$K^* = \frac{I_A - I_C}{R_C - U_A}$$
Using measured empirical values from the benchmarks:
- At $n = 50$: $I_A - I_C \approx 31\ \mu\text{s}$, $R_C - U_A \approx 11.5\ \mu\text{s} \implies \mathbf{K^* \approx 2.7 \to 3 \text{ updates}}$.
- At $n = 5,000$: $I_A - I_C \approx 2.6\ \text{ms}$, $R_C - U_A \approx 0.59\ \text{ms} \implies \mathbf{K^* \approx 4.4 \to 5 \text{ updates}}$.

**Conclusion**: When starting from scratch, Algorithm A breaks even after **3 to 5 dynamic updates**. Beyond 5 updates, incremental maintenance provides continuous compounding savings.
