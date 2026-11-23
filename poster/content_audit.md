# Academic Poster Content Audit & Mathematical Traceability Matrix

**Project:** B.Tech Thesis — Dynamic Graph Algorithms on Proper Interval Graphs  
**Institution:** Department of Computer Science and Engineering, School of Electrical and Computer Sciences, Indian Institute of Technology Bhubaneswar  
**Author:** K. Saravanarjun (Roll No: 23CS01028)  
**Supervisor:** Dr. Manas Jyothi Kashyop  
**Target Format:** Academic Research Poster (42" $\times$ 21" Landscape, 2:1 Aspect Ratio)  

---

## 1. Scope, Guardrails, and Exclusion Verification

| Scope Item | Policy / Requirement | Verification Status | Notes / Location |
|:---|:---|:---|:---|
| **Primary Scope** | Chapter 3 of BTP Thesis: Structural Analysis of Proper Interval Graphs | **INCLUDED** | Focused on distance tree geometry, maximal cliques, and MIS/MDS. |
| **Dominant Contribution** | Maximal Clique Characterization and Static/Dynamic Algorithms | **EMPHASIZED** | Sections 3 and 4 feature prominently as the central contribution. |
| **Supporting Contribution** | Maximum Independent Set & Dominating Set | **INCLUDED** | Section 5 presents MIS/MDS bounds and Algorithm 1 linear-time DP. |
| **Compiler Design Motivation** | Real-world application in register allocation | **INCLUDED** | Section 1 details interference graphs and straight-line code restrictions. |
| **Graph Burning Exclusion** | Completely exclude Section 3.11, supernode structures, burning numbers, Theorems 3.18, 3.19 | **EXCLUDED** | Zero mentions of burning number $b(G)$ or burning algorithms anywhere on the poster or in its source files. |
| **Terminology Guardrails** | Avoid informal "Chapter 3", textbook section names | **VERIFIED** | Formally structured into scientific contribution titles. |
| **Canonical Parent Rule** | $p(v) = \arg\min \{l_u \mid r_u \ge l_v\}$ (earliest-starting reaching neighbor) | **VERIFIED** | Formally defined and cited with deterministic tie-breaking. |
| **Descriptor Semantics** | $C(p) = [p, c(p)] = \{v \in V \mid p \le v \le c(p)\}$ | **VERIFIED** | Contiguous index interval representation in $2 \times 4$ bytes. |
| **Case B Correction** | Up to 3 descriptor edits in decremental deletion (counterexample to thesis claim of $\le 2$) | **VERIFIED** | Highlighted in Section 4 with minimal 7-vertex counterexample reference. |
| **Affiliations & QR** | IIT Bhubaneswar CSE, Dr. Manas Jyothi Kashyop, GitHub QR code | **VERIFIED** | Exact text matching thesis front matter; vector QR code compiled natively. |

---

## 2. Mathematical Content Traceability Matrix

### 2.1 Core Structural Foundations (Chapter 3)
- **Thesis Reference:** Chapter 3, Sections 3.1–3.4, Definition 3.1, Lemmas 3.1–3.6, Theorem 3.7.
- **Repository Implementation:** `src/distance_tree.py`, `benchmarks/maximal_cliques/distance_tree.py`.
- **Formally Stated & Verified Theorems on Poster (Section 2):**
  1. *Lemma 3.1 (Parent--Children Clique):* For any $u \in T(G)$ with children $u_1, \dots, u_k$, $\{u, u_1, \dots, u_k\}$ forms a clique in $G$.
  2. *Lemma 3.2 (No Edge to Parent's Left Sibling):* If $v$ is a child of $p$, and $q$ is a left sibling of $p$, then $qv \notin E$.
  3. *Lemma 3.3 (Adjacency with Parent's Right Siblings):* If $v$ is a child of $u$, and $w$ is a right sibling of $u$, then $v$ is adjacent to $w$ in $G$.
  4. *Lemma 3.4 (Clique with Children and Right Siblings):* Node $u$, its children, and all right siblings of $u$ form a clique in $G$.
  5. *Lemma 3.5 (Mutual Adjacency of Siblings' Children):* For siblings $u, v$ ($v$ left of $u$), child $w$ of $u$ and child $x$ of $v$ are adjacent in $G$.
  6. *Lemma 3.6 (Propagation of Adjacency to Children):* Vertices $u, v$ at same depth with $uv \in E$ implies every child of $u$ is adjacent to every child of $v$ in $G$.
  7. *Theorem 3.7 (Each Depth Level Induces a Clique):* For every depth $i$, all vertices at depth $i$ form a complete subgraph (clique) in $G$.
  8. *Lemma 3.8 (Maximal Clique Characterization):* $\{u\} \cup \text{children}(u) \cup \text{right-siblings}(u) \cup \text{children of left-siblings}(u)$ forms a maximal clique in $G$.
  9. *Lemma 3.10 (Bound on Maximal Cliques Containing a Vertex):* Upper bound on maximal cliques containing vertex $u$ using Iverson brackets.

### 2.2 Maximal Clique Characterization & Compact Descriptors (Section 3)
- **Thesis Reference:** Chapter 3, Section 3.6, Lemma 3.15, Theorem 3.10.
- **Key Theorem (Parent Bijection):** In a connected proper interval graph, there is a strict bijection between the set of non-leaf parents $\mathcal{P}_{nl}(T)$ and the set of maximal cliques $\mathcal{M}(G)$:
  $$\mathcal{M}(G) = \{ C(p) = [p, c(p)] \mid p \in \mathcal{P}_{nl}(T) \}, \quad \text{where } c(p) = \max \text{children}(p).$$
- **Lemma 3.15 (Interval Representation):** Every maximal clique is an index interval $[\ell_1, \ell_2] = [\ell_p, \ell_c]$.
- **Memory Complexity:** Storing $[p, c(p)]$ as a pair of 32-bit integers ($2 \times 4 = 8$ bytes) reduces storage from $\mathcal{O}(n \cdot \omega(G))$ to $\mathcal{O}(q)$ words ($q \le n$).
- **Experimental Verification:** $90.2\%$ space savings at $n=50$, $95.2\%$ at $n=1,000$, $97.6\%$ at $n=5,000$, and $99.0\%$ at $n=100,000$.

### 2.3 Dynamic Maximal Clique Maintenance (Section 4)
- **Thesis Reference:** Chapter 3, Section 3.7, Theorem 3.16.
- **Implementation:** `benchmarks/maximal_cliques/dynamic_descriptors.py`.
- **Incremental Maintenance (Leaf Insertion):**
  - Newly inserted interval $v_{n+1}$ attaches at the right boundary.
  - Parent search: Binary search locates $p(v_{n+1}) = \arg\min \{l_u \mid r_u \ge l_{v_{n+1}}\}$ in $\mathcal{O}(\log n)$ time.
  - Descriptor update: At most **1 descriptor edit** (boundary extension or new parent descriptor).
- **Decremental Maintenance (Vertex Deletion & Sibling Adoption):**
  - Leaf deletion: $\le 1$ descriptor boundary update.
  - Internal parent deletion: Children $\text{children}(u)$ are adopted by predecessor sibling $v = \arg\max_{w < u} \{r_w \ge l_{\min \text{children}(u)}\}$.
  - Recomputed update time: $\mathcal{O}(n + |C(u)| + \log q)$.
  - **Audit Finding & Case B Bound Correction:** The thesis originally claimed $\le 2$ descriptor edits. Our audit disproved this: parent adoption can cause simultaneous descriptor invalidation, adopting parent boundary extension, and adjacent boundary adjustment. We proved that **at most 3 edits** occur, confirmed by our minimal tight 7-vertex counterexample ($N=7, q=4$).

### 2.4 Maximum Independent Set & Dominating Set (Section 5)
- **Thesis Reference:** Chapter 3, Section 3.8, Lemma 3.11, Theorems 3.12, 3.13, 3.17.
- **Structural Bounds:**
  - *Lemma 3.11 (MIS Bounds):* $\lfloor h/2 \rfloor \le \alpha(G) \le h - 1$.
  - *Theorem 3.12 (2-Approximation):* Alternate level selection yields 2-approximation in $\mathcal{O}(h)$ time.
  - *Theorem 3.17 (Minimum Dominating Set Bound):* $\gamma(G) \le \lceil(h-1)/2\rceil$; for odd $h$, $\gamma(G) \le (h-1)/2$.
- **Exact Linear DP (Algorithm 1):**
  - Evaluates $DP[i][v] = 1 + \max\{BEST[i-2], PREFIX[i-1][k-1]\}$.
  - Complexity: Exact linear $\mathcal{O}(n)$ time and $\mathcal{O}(n)$ space without interval tree overhead.

---

## 3. Experimental Data & Results Traceability

All numbers and figures featured on the poster are directly sourced from verified benchmark CSV files:

| Metric | Tested Sizes ($n$) | Reported Value | Benchmark Source CSV |
|:---|:---|:---|:---|
| **Static Runtime (Algorithm A)** | $n=50$ to $1,000,000$ | $0.0197$ ms ($n=50$), $0.38$ ms ($n=10^3$), $2.12$ ms ($n=5\cdot 10^3$), $421.3$ ms ($n=10^6$) | `benchmarks/maximal_cliques/results/raw_benchmark_results.csv` |
| **Static Baseline (Sweep-Line)** | $n=50$ to $1,000,000$ | $0.0039$ ms ($n=50$), $0.1039$ ms ($n=10^3$), $0.6341$ ms ($n=5\cdot 10^3$), $128.4$ ms ($n=10^6$) | `benchmarks/maximal_cliques/results/raw_benchmark_results.csv` |
| **Bron–Kerbosch Limit** | $n \ge 5,000$ | Times out / combinatorial explosion on dense graphs | `benchmarks/maximal_cliques/results/raw_benchmark_results.csv` |
| **Memory Savings** | $n=10^3$ to $10^5$ | 90.2% to 99.0% reduction vs explicit vertex lists | `benchmarks/maximal_cliques/results/memory_comparison.csv` |
| **Dynamic Update Latency** | $n=50$ to $5,000$ | $0.34\ \mu\text{s}$ ($n=50$), $0.65\ \mu\text{s}$ ($n=10^3$), $1.32\ \mu\text{s}$ ($n=5\cdot 10^3$) | `benchmarks/maximal_cliques/results/dynamic_benchmark_results.csv` |
| **Dynamic Speedup vs Recomp** | $n=50$ to $5,000$ | $370\times$ ($n=50$), $628\times$ ($n=10^3$), $912\times$ ($n=5\cdot 10^3$) | `benchmarks/maximal_cliques/results/dynamic_speedup.csv` |
| **Amortized Break-Even Model** | $n=50, 1000, 5000$ | $K^* = 4$ operations across all tested graph sizes | `benchmarks/maximal_cliques/FINAL_RESEARCH_AUDIT/FINAL_BENCHMARK_REPORT.md` |
| **Verification Suite** | 17 graph families | 77 test configurations, 165 dynamic steps, 56 unit regression tests (100% pass) | `benchmarks/maximal_cliques/tests/` |

---

## 4. Visual Layout & Geometric Audit Metrics

The compiled PDF (`poster/poster.pdf`) was validated using PyMuPDF:

```
=== POSTER VISUAL & STRUCTURAL AUDIT ===
Total Page Dimensions: 3024.0 x 1512.0 pt (42.00 x 21.00 in)
Total text blocks detected: 200
Content Bounding Box:
  X: 49.1 to 2975.9 pt (Margins: Left=49.1 pt, Right=48.1 pt)
  Y: 36.5 to 1274.9 pt (Margins: Top=36.5 pt, Bottom=237.1 pt)

Column 1 Vertical Span: 247.1 to 1255.9 pt (Height: 1008.8 pt)
Column 2 Vertical Span: 247.1 to 1271.4 pt (Height: 1024.4 pt)
Column 3 Vertical Span: 247.1 to 1274.9 pt (Height: 1027.8 pt)

Column Balance Variance: max-min = 19.0 pt (Target < 150 pt)
[PASS] Columns are exceptionally well balanced!
[PASS] Zero Overfull \vbox warnings detected in LaTeX log!
```

---

## 5. Audit Sign-off
- **Mathematical rigor:** Formally verified 9 core structural theorems/lemmas from Chapter 3.
- **Scientific Narrative:** Perfectly follows the storyline (Motivation $\to$ Structural Foundations $\to$ Maximal Clique Characterization $\to$ Dynamic Maintenance $\to$ MIS DP $\to$ Benchmarks \& Baselines $\to$ Break-Even Analysis).
- **Scope Restriction Satisfied:** Completely excluded Graph Burning (Section 3.11, supernode structures, $b(G)$).
- **Deliverable State:** Print-ready PDF generated and verified at exactly 42 in $\times$ 21 in landscape.
