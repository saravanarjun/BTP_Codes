# Academic Research Poster: Dynamic Graph Algorithms on Proper Interval Graphs

**Event:** B.Tech Thesis (BTP) Final Presentation  
**Institution:** Department of Computer Science and Engineering, Indian Institute of Technology Bhubaneswar  
**Candidate:** K. Saravanarjun (Roll No: 23CS01028)  
**Supervisor:** Dr. Manas Jyothi Kashyop  
**Repository:** [https://github.com/saravanarjun/BTP_Codes](https://github.com/saravanarjun/BTP_Codes)  

---

## 1. Poster Technical Specifications

| Parameter | Specification | Verification Result |
|:---|:---|:---|
| **Dimensions** | 42 inches $\times$ 21 inches (Landscape) | **Exact:** $3024.0 \times 1512.0$ pt ($42.00 \times 21.00$ in) |
| **Aspect Ratio** | 2:1 Landscape | **Verified:** 2:1 |
| **Page Count** | Exactly 1 Page (Standard Conference Poster) | **Verified:** 1 page, 0 overflow |
| **Engine / Class** | pdfTeX (`pdflatex`) with `beamer` + `beamerposter` | **Clean build:** Exit code 0, 0 pt Overfull `\vbox` |
| **Color Theme** | Restrained academic CS conference styling | Deep navy (`#0B203B`, `#1E3A8A`), crisp white cards, light slate canvas (`#F8FAFC`) |
| **Graphics Format** | 100% Vector Graphics (PDF / TikZ) | Crystal-clear rendering at arbitrary zoom; zero pixelation |
| **QR Code Target** | `https://github.com/saravanarjun/BTP_Codes` | Native vector QR code in header labeled "Code & Benchmarks" |
| **Column Balance** | 3-Column Balanced Layout | **19.0 pt variance** across all three columns (exceptional balance) |

---

## 2. Structural Content & Layout Map

The poster is partitioned into three balanced vertical columns (column width $\approx 0.322\textwidth$) enclosed in structured `tcolorbox` cards:

### Column 1: Foundations, Structural Theorems & Static Cliques
1. **Section 1 — Motivation & Problem Statement:**
   - Formal definition of proper interval graphs: $l_u < l_v \iff r_u < r_v$.
   - Compiler design application: **Register allocation** and interference graphs under uniform lifespans / straight-line execution blocks.
   - Core research problem: Exploiting distance tree geometry to characterize, compute, and maintain maximal cliques dynamically.
   - Canonical Parent Rule: $p(v) = \arg\min_{u < v} \{l_u \mid r_u \ge l_v\}$ with deterministic tie-breaking.
2. **Section 2 — Core Structural Findings:**
   - Parent, Child, and Sibling Adjacency:
     - *Lemma 3.1 (Parent-Children Clique):* $\{u, u_1, \dots, u_k\}$ forms a clique in $G$.
     - *Lemma 3.2 (No Edge to Parent's Left Sibling):* $qv \notin E$.
     - *Lemma 3.3 (Adjacency with Parent's Right Siblings):* $vw \in E$.
     - *Lemma 3.4 (Clique with Children and Right Siblings):* $\{u\} \cup \text{children}(u) \cup \text{right-siblings}(u)$ forms a clique.
   - Adjacency Propagation & Level Cliques:
     - *Lemma 3.5 (Mutual Adjacency of Siblings' Children):* $wx \in E$.
     - *Lemma 3.6 (Propagation of Adjacency to Children):* $uv \in E \implies \forall c_u, c_v, c_u c_v \in E$.
     - *Theorem 3.7 (Each Depth Level Induces a Clique):* Every BFS level $L_i$ in $T(G)$ is a clique.
   - Maximal Clique Characterization & Membership Bound:
     - *Lemma 3.8 (Maximal Clique Characterization):* $\{u\} \cup \text{children}(u) \cup \text{right-siblings}(u) \cup \text{children of left-siblings}(u)$ forms a maximal clique.
     - *Lemma 3.10 (Bound on Maximal Cliques Containing a Vertex):* Explicit upper bound using Iverson bracket notation.
   - Publication-grade TikZ vector diagram (`poster/figures/fig_distance_tree.pdf`).
3. **Section 3 — Maximal Clique Characterization & Static Algorithm:**
   - *Lemma 3.15 (Interval Representation):* $[\ell_1, \ell_2] = [\ell_p, \ell_c]$.
   - *Theorem (Parent Bijection):* Strict bijection between non-leaf parents $\mathcal{P}_{nl}(T)$ and maximal cliques $\mathcal{M}(G)$.
   - Compact descriptors: $2 \times 4$ bytes per clique, storing all $q \le n$ cliques in $\mathcal{O}(q)$ space.
   - Static Algorithm: $\mathcal{O}(n+m)$ tree traversal.
   - Embedded experimental memory table and vector log-scale plot (`poster/figures/plot_memory_comparison.pdf`) demonstrating up to **99.0% space reduction** over explicit lists.

### Column 2: Dynamic Maintenance & Graph Optimization (MIS / MDS)
4. **Section 4 — Dynamic Maximal Clique Maintenance:**
   - *Incremental Maintenance:* Leaf insertion in $\mathcal{O}(\log n)$ time; **at most 1 descriptor edit**.
   - *Decremental Maintenance:* Vertex deletion in $\mathcal{O}(n + |C(u)| + \log q)$ time; internal parent deletion triggers predecessor sibling adoption.
   - TikZ vector restructuring diagram (`poster/figures/fig_dynamic_adoption.pdf`).
   - *Audit finding & Case B resolution:* Corrected thesis claim of $\le 2$ to tight bound of **at most 3 edits**, verified by minimal 7-vertex counterexample ($N=7, q=4$).
5. **Section 5 — Maximum Independent Set & Dominating Set:**
   - *Distance Tree Bounds:* $\lfloor h/2 \rfloor \le \alpha(G) \le h - 1$ (Lemma 3.11).
   - *2-Approximation for MIS:* Alternate levels in $\mathcal{O}(h)$ time (Theorem 3.12).
   - *Minimum Dominating Set Bound:* $\gamma(G) \le \lceil(h-1)/2\rceil$ (Theorem 3.17).
   - *Algorithm 1 (Exact Linear DP):* Exact MIS in **$\mathcal{O}(n)$ time and $\mathcal{O}(n)$ space** using prefix accumulator array $PREFIX$.
   - Vector DP transition diagram (`poster/figures/fig_mis_dp.pdf`).

### Column 3: Experimental Evaluation, Break-Even & Impact
6. **Section 6 — Experimental Evaluation & Baseline Comparisons:**
   - Comprehensive differential testing: 77 configurations across 17 graph families (165 mutation steps, 100% agreement).
   - Static scalability vector plot (`poster/figures/plot_static_scalability.pdf`): Linear $\mathcal{O}(n+m)$ vs exponential Bron-Kerbosch.
   - Dynamic update latency vector plot (`poster/figures/plot_dynamic_speedup.pdf`): Sub-microsecond latency ($0.34 - 1.32\ \mu\text{s}$) with **$370\times$ to $912\times$ speedup** over static recomputation.
   - Comprehensive benchmark table from $n=50$ to $n=1,000,000$.
7. **Section 7 — Amortized Break-Even Analysis:**
   - Amortized break-even vector plot (`poster/figures/plot_break_even.pdf`):
     $$K^* = \left\lceil \frac{T_{\text{build}}(A)}{T_{\text{recomp}}(C) - T_{\text{dyn}}(A)} \right\rceil = 4$$
   - Dynamic maintenance breaks even after only **$K^* = 4$ operations** compared to full static recomputation.
8. **Section 8 — Key Contributions & Future Directions:**
   - Core theoretical and algorithmic achievements.
   - JIT compiler integration (LLVM/V8) and generalizations to circular-arc, permutation, and tolerance graphs.

---

## 3. Strict Scope & Exclusion Compliance

- **Graph Burning Exclusion:** Per project guidelines, Section 3.11, supernode structures, burning numbers $b(G)$, and Theorems 3.18/3.19 are **completely excluded** from the poster and all associated files.
- **Accurate Affiliations:** Candidate K. Saravanarjun (23CS01028), Supervisor Dr. Manas Jyothi Kashyop, Department of Computer Science and Engineering, IIT Bhubaneswar.

---

## 4. Directory Structure & Asset Inventory

```
poster/
├── poster.tex                     # Primary LaTeX source (beamer + beamerposter)
├── poster.pdf                     # Print-ready compiled PDF (42 in x 21 in, 3024 x 1512 pt)
├── content_audit.md               # Detailed mathematical and experimental traceability matrix
├── README.md                      # This comprehensive documentation and user guide
├── poster_preview_150dpi.png      # High-resolution raster preview for presentation/inspection
├── crop_header.png                # Zoomed crop of title and metadata block
├── crop_col1.png                  # Zoomed crop of Column 1 (Foundations & Distance Tree)
├── crop_col2.png                  # Zoomed crop of Column 2 (Maximal Cliques & Dynamic Alg.)
├── crop_col3.png                  # Zoomed crop of Column 3 (MIS, Benchmarks & Speedup)
├── figures/
│   ├── fig_distance_tree.tex      # Standalone TikZ source for interval model & distance tree
│   ├── fig_distance_tree.pdf      # Compiled vector PDF diagram
│   ├── fig_dynamic_adoption.tex   # Standalone TikZ source for parent deletion & child adoption
│   ├── fig_dynamic_adoption.pdf   # Compiled vector PDF diagram
│   ├── fig_mis_dp.tex             # Standalone TikZ source for linear MIS DP
│   ├── fig_mis_dp.pdf             # Compiled vector PDF diagram
│   ├── plot_static_scalability.pdf# Log-log static runtime comparison (n=10 to 1,000,000)
│   ├── plot_memory_comparison.pdf # Grouped log-scale memory footprint (Descriptors vs Lists)
│   ├── plot_dynamic_speedup.pdf   # Dual-axis dynamic update latency and speedup factor
│   └── plot_break_even.pdf        # Amortization curves establishing K* = 4
└── scripts/
    ├── generate_poster_plots.py   # Matplotlib vector plot generator
    ├── check_poster_layout.py     # Automated text and section presence verification
    ├── audit_poster_visuals.py    # Automated layout geometry, margins, and column balance audit
    └── render_inspection_crops.py # High-res raster renderer for poster inspection
```

---

## 5. Instructions for Compilation & Reproduction

### Prerequisites
- TeX distribution: MiKTeX or TeX Live with `pdflatex`.
- Python 3.10+ with `matplotlib`, `pandas`, `numpy`, `fitz` (`PyMuPDF`).

### Step 1: Regenerate Benchmark Vector Plots
```bash
python poster/scripts/generate_poster_plots.py
```

### Step 2: Compile Standalone TikZ Figures
```bash
pdflatex -interaction=nonstopmode -output-directory=poster/figures poster/figures/fig_distance_tree.tex
pdflatex -interaction=nonstopmode -output-directory=poster/figures poster/figures/fig_dynamic_adoption.tex
pdflatex -interaction=nonstopmode -output-directory=poster/figures poster/figures/fig_mis_dp.tex
```

### Step 3: Compile the Master Poster
```bash
pdflatex -interaction=nonstopmode -output-directory=poster poster/poster.tex
```

### Step 4: Validate Geometry & Column Balance
```bash
python poster/scripts/check_poster_layout.py
python poster/scripts/audit_poster_visuals.py
```

### Step 5: Render Raster Preview for Visual Inspection
```bash
python poster/scripts/render_inspection_crops.py
```
