# Proof Gaps, Unresolved Mathematical Invariants, and Clarifications
## Maximal Clique Maintenance on Proper Interval Graphs

**Repository**: `C:/Users/sarav/OneDrive/Desktop/BTP_Codes`  
**Subdirectory**: `benchmarks/maximal_cliques/`  
**Audience**: Thesis Committee, Research Advisors, Developers  
**Date**: November 2026  

---

# 1. Executive Identification of the Primary Proof Gap

### The Fundamental Gap: MIS vs. Maximal Cliques
The thesis *Dynamic Graph Algorithms – Interval Graphs* establishes dynamic maintenance for two specific graph structures:
1. **Theorem 3.14 (Thesis Section 3.10.2)**: Incremental maintenance of a **Maximum Independent Set (MIS)** under leaf insertion.
2. **Theorem 3.16 (Thesis Section 3.10.3)**: Decremental maintenance of the **Distance Tree $T(G)$** under vertex deletion.

> [!CRITICAL]
> **Neither theorem addresses maximal cliques.** Conflating Distance Tree maintenance or MIS maintenance with maximal clique maintenance is mathematically invalid.
> - An **Independent Set** is a set of pairwise non-adjacent vertices, governed by dynamic programming prefix tables across tree levels ($\alpha(u) = 1 + \max_{w: l_w > r_u} \alpha(w)$).
> - A **Maximal Clique** is a set of mutually intersecting intervals, characterized by compact descriptors $[l_p, l_{c_p}]$ for non-leaf distance tree parents (Lemma 3.15).

To claim dynamic maximal clique maintenance as a thesis contribution, one must provide a standalone proof that the dictionary of Lemma 3.15 descriptors remains sound and complete under tree updates.

---

# 2. Status of Key Mathematical Invariants

### Invariant 1: Soundness of Descriptors Under Rightmost Leaf Insertion
- **Status**: **RESOLVED & PROVED**
- **Theorem**: Appending a new interval $y$ as a leaf to the right of all existing vertices modifies $\le 1$ descriptor in the Lemma 3.15 dictionary.
- **Proof Summary**:
  - $y$ has no children, so it does not create a new non-leaf descriptor.
  - No existing non-leaf node $u \ne p(y)$ gains or loses children, so their rightmost children remain unchanged.
  - Parent $p(y)$ either transitions from leaf to non-leaf (adding descriptor $p \mapsto y$) or updates its rightmost child to $y$ (modifying descriptor $p \mapsto c_p^{\text{old}}$ to $p \mapsto y$).
  - All other descriptors are strictly invariant.

---

### Invariant 2: Soundness of Descriptors Under Theorem 3.16 Deletion
- **Status**: **RESOLVED & PROVED (Bound Revised to $\le 3$)**
- **Theorem**: Deleting vertex $u$ with immediate right sibling $v$ at the same depth relocates $u$'s children under $v$ and mutates $\le 3$ descriptors in the dictionary.
- **Proof Summary**:
  - If $u$ was in the dictionary, it is erased ($\le 1$ removal).
  - If $v$ had no prior children ($v$ was a leaf), $v$ gains descriptor $v \mapsto u_{\text{right}}$ ($\le 1$ addition). If $v$ already had children, $v$'s rightmost child remains its existing rightmost child, so $v$'s descriptor is unchanged.
  - If $u$ was the rightmost child of $p(u)$, $p(u)$'s rightmost child changes to $\text{prevSibling}(u)$ ($\le 1$ modification) or is erased if $u$ was an only child ($\le 1$ removal).
  - When $v$ was a leaf and $u = c_{p(u)}$ (with $p(u)$ possessing an earlier child), $u$ is removed, $v$ is added, and $p(u)$ is modified, producing **exactly 3 descriptor changes simultaneously** (see 7-vertex counterexample in `STAGE_1_5_PROOF_CLOSURE.md`).
  - Strongest tight bound: $\Delta_{\text{added}} \le 1$, $\Delta_{\text{removed}} \le 2$, $\Delta_{\text{modified}} \le 1$, and $\mathbf{\Delta_{\text{affected}} \le 3}$.

---

### Invariant 3: Redundancy of Predecessor Subset Filter
- **Status**: **RESOLVED & PROVED**
- **Theorem**: Under canonical tie-breaking ($l_u < l_v \lor (l_u = l_v \land r_u < r_v) \lor (l_u = l_v \land r_u = r_v \land \text{id}(u) < \text{id}(v))$), every non-leaf parent in $T(G)$ defines a unique, non-nested maximal clique.
- **Proof Summary**:
  - Identical copies of a parent node never acquire children because tie-breaking by ID always directs prospective children to the first copy. Hence, identical intervals are leaves.
  - For any two non-leaf parents $p_1 \prec p_2$, we have $l_{p_1} < l_{p_2}$ and $l_{c_{p_1}} < l_{c_{p_2}}$.
  - The interval $[l_{p_1}, l_{c_{p_1}}]$ cannot contain $[l_{p_2}, l_{c_{p_2}}]$ nor vice versa.
  - Therefore, the subset condition $C(p) \not\subset C(\text{pred}(p))$ is always satisfied, and subset filtering is redundant.

---

# 3. Prioritized Questions Requiring Advisor / User Decisions

### Question Q-01: Scope of Dynamic Operation Classes
- **Current Situation**:
  - Insertion is restricted to **rightmost append-only leaf insertion** (Phase 8 model).
  - Deletion is restricted to **Theorem 3.16 conditions** (non-root vertex; if non-leaf, must possess an immediate right sibling at the same depth).
- **Question for Decision**:
  Does the thesis intend to present dynamic maximal clique maintenance specifically under this **structured, incremental/decremental maintenance model** (matching Theorems 3.14 and 3.16), or is an unrestricted, arbitrary dynamic interval graph algorithm expected?
- **Recommendation**:
  Present this as **Theorem 3.15-Dyn: Dynamic Maximal Clique Maintenance under Theorem 3.14/3.16 Distance-Tree Operations**. This aligns directly with the thesis's established structural framework without overclaiming general dynamic graph maintenance.

---

### Question Q-02: Telemetry Decoupling from Public API
- **Current Situation**:
  In `dynamic_clique_structure.cpp`, the methods `insertLeaf()` and `deleteVertex()` execute full `std::map` copying and global comparisons inside the timed block to record set-difference telemetry (`descriptorsAdded`, `descriptorsRemoved`, etc.). This causes the empirical latency to scale as $\mathcal{O}(n \log n)$ rather than $\mathcal{O}(\log n)$.
- **Question for Decision**:
  Should we decouple this telemetry so that the public API executes strictly in $\mathcal{O}(\log n)$ / $\mathcal{O}(\text{deg}(u))$ time, and perform global verification only in debug or test runs?
- **Recommendation**:
  Yes. Maintain local tracking of affected keys $(\le 2)$ to record additions and modifications in $\mathcal{O}(1)$ time, eliminating all global map copies from the timed benchmarks.

---

### Question Q-03: Treatment of Degenerate Graph Families
- **Current Situation**:
  The benchmark suite includes Families 4 (`identical-clusters`) and 13 (`adversarial-tie-breaking`), which feature multiple intervals with identical endpoints ($I_u = I_v$).
- **Question for Decision**:
  Should identical intervals be formally classified as a sub-case of proper interval graphs, or separated into a dedicated "Robustness and Tie-Breaking" section of the thesis?
- **Recommendation**:
  Document them as **canonical robustness benchmarks**. Point out that while strict proper interval graphs prohibit interval containment ($I_u \subsetneq I_v$), identical intervals satisfy $I_u \subseteq I_v$ without strict containment and are fully supported by our canonical total ordering.

---

### Question Q-04: Reporting Standards for Skipped / Timed-Out Algorithms
- **Current Situation**:
  Bron–Kerbosch times out or is skipped for $n > 2,000$. Previous text loosely referenced "orders of magnitude speedup" or "infinite speedup".
- **Question for Decision**:
  Confirm that all documentation, tables, and figures will strictly report Bron–Kerbosch as `TIMEOUT` ($> 3,000\ \text{ms}$) or `SKIPPED_BY_POLICY`, with speedups calculated only on completed runs.
- **Recommendation**:
  Adopt strict academic reporting standards: never divide by zero, never claim infinite speedup, and use `-1.0` as the numeric sentinel in CSV datasets.
