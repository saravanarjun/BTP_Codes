# Final Open Issues and Future Directions: Maximal Clique Module

**Document Version:** 1.0 (Final Research Closure)  
**Date:** November 2026  
**Module:** `benchmarks/maximal_cliques/`  
**Repository:** `BTP_Codes`

---

## 1. Scope of Supported Dynamic Graph Operations

The current dynamic maintenance module implements the exact operations supported by the BTP thesis:
1. **Rightmost Append-Only Leaf Insertion:**
   Inserts a new vertex $u$ as the rightmost vertex in its level, preserving existing parent relationships and level depths.
2. **Decremental Deletion under Theorem 3.16:**
   Deletes an admissible non-root vertex $u$ that possesses an immediate right neighbor $v$ in the same level $L_k$ whenever $u$ has children.

### Open Direction:
Arbitrary dynamic operations — such as inserting a vertex into the interior of an existing level, deleting a vertex whose removal alters the distance levels of surviving vertices, or handling graph-disconnecting deletions — currently require a full $\mathcal{O}(n + m)$ static tree rebuild. Extending the distance tree maintenance to unrestricted dynamic vertex insertions and edge updates remains an open research problem in algorithmic graph theory.

---

## 2. Exponential Complexity and Timeout Limits of Bron–Kerbosch

Pivoted Bron–Kerbosch (Algorithm B) is known to have a worst-case runtime of $\mathcal{O}(3^{n/3})$ on general graphs. Even with Tomita degree-based pivoting:
- On sparse graphs with irregular branching or high combinatorial independence, the recursion tree can expand dramatically even for $n \in [50, 100]$.
- A strict timeout of $3000\text{ ms}$ is enforced in benchmarking, and graphs with $n > 2000$ are skipped by policy.

### Implication:
Bron–Kerbosch serves as a baseline for verifying small-scale ground truth. It is not intended as a competitive practical solver for large-scale proper interval graphs.

---

## 3. Scope of Graph Families and Real-World Workloads

The benchmark evaluates 14 synthetic and adversarial proper interval graph families:
- Equal-length profiles (sparse, medium, dense)
- Unequal lengths, identical clusters, endpoint-touching, long chains, high overlap
- Irregular spacing, many small cliques, few large cliques, varying overlap
- Adversarial parent-selection tie-breaking and shuffled IDs

### Future Direction:
While these families thoroughly explore the structural extremes and algorithmic edge cases, benchmarking on real-world interval workloads (e.g., DNA sequencing alignments, genomic intervals, task scheduling logs) represents an interesting future avenue of empirical study.

---

## 4. Analytical vs. System-Level Memory and Container Overhead

The memory metrics reported in benchmark CSV files:
- `estimated_descriptor_bytes`: $2q \times \text{sizeof}(\text{int})$
- `estimated_explicit_bytes`: $\sum_{K} |K| \times \text{sizeof}(\text{int})$

represent the theoretical minimal payload required to store the maximal cliques in compact vs. explicit form.
They do not capture standard library overheads:
- `std::vector` capacity padding and reallocation overhead.
- `std::map<int, int>` red-black tree node overhead (pointers to parent, left, right, plus color flag per node).
- Cache line alignment or OS page-table allocation.

### Container Choice and Constant-Factor Optimization:
In the current implementation of dynamic maintenance:
1. `std::map<int, int>` is utilized for descriptors, which introduces an $\mathcal{O}(\log q)$ lookup and update overhead rather than an idealized $\mathcal{O}(1)$ direct array lookup.
2. In the high-level `DynamicDistanceTree::deleteVertex` helper, an auxiliary `intervals.erase(std::remove_if(...))` is performed to maintain a contiguous vector of surviving intervals, which introduces an $\mathcal{O}(n)$ scan. While the tree's structural pointer maintenance and child adoption are purely $\mathcal{O}(|C(u)|)$, and descriptor updates are $\mathcal{O}(\log q)$, eliminating the $\mathcal{O}(n)$ interval vector compaction would require switching to an indexed node pool, doubly-linked list, or tombstone array. This is a clear, valuable target for future low-level systems optimization.
