# Complete Algorithmic Explanations: Maximal Cliques on Proper Interval Graphs
## Algorithms A, B, and C in Static and Dynamic Settings

**Repository**: `C:/Users/sarav/OneDrive/Desktop/BTP_Codes`  
**Subdirectory**: `benchmarks/maximal_cliques/`  
**Audience**: Algorithms Researchers, Thesis Reviewers, Implementers  
**Date**: November 2026  

---

# 1. Overview of Evaluated Algorithms

This document provides complete, publication-grade algorithmic specifications for the three maximal clique algorithms evaluated in this repository:

1. **Algorithm A (Theorem-Based Structural Algorithm)**:
   - *Static*: Uses Distance Tree $T(G)$ and Lemma 3.15 compact descriptors $[l_p, l_{c_p}]$.
   - *Dynamic*: Maintains $T(G)$ links and the descriptor dictionary under leaf insertions (Section 3.10.2) and child relocations (Section 3.10.3 / Theorem 3.16).
2. **Algorithm B (Pivoted Bron–Kerbosch with Tomita Pivoting)**:
   - Classic backtracking search tree with degree-maximizing pivot selection.
   - Evaluated statically from scratch and dynamically via full recomputation.
3. **Algorithm C (Standard Interval Sweep-Line Algorithm)**:
   - Sorts interval endpoints, maintains an active set using the Helly property, and emits maximal cliques upon interval departure.
   - Evaluated statically from scratch and dynamically via full recomputation.

---

# 2. Algorithm A: Theorem-Based Structural Algorithm

### 2.1 Mathematical Intuition
In a connected proper interval graph, intervals ordered canonically by left endpoints are also ordered monotonically by right endpoints ($l_u < l_v \implies r_u \le r_v$).
A Breadth-First Search tree rooted at the leftmost interval $u_0$ creates a distance tree $T(G)$ where parent selection is canonical:
$$p(y) = \arg\min_{u \in V} \{ l_u : r_u \ge l_y \}$$
Every maximal clique in $G$ corresponds to the mutual intersection of intervals between a parent node $p$ and its rightmost child $c_p$.
Because $c_p$ is adjacent to $p$, $l_{c_p} \le r_p$. By properness, every interval $v$ whose left endpoint lies in $[l_p, l_{c_p}]$ satisfies $l_v \le l_{c_p} \le r_p \le r_v$, guaranteeing that all such vertices mutually intersect.

---

### 2.2 Static Algorithm A Specification

#### Data Structures
- `IntervalGraph`: Canonically sorted interval array with $\mathcal{O}(1)$ index lookups.
- `DistanceTree`: Level hierarchy arrays, parent array, and ordered child lists.
- `TheoremCliqueDescriptor`: Struct containing `parentId`, `rightmostChildId`, `leftBound`, `rightBound`, `parentSortedIndex`, `rightmostChildSortedIndex`, and `size`.

#### Pseudocode
```text
Algorithm StaticAlgorithmA(Graph G):
1. Construct distance tree T(G) from root u_0 = v_0.
2. Initialize empty list of descriptors Descriptors.
3. If |V(G)| == 1:
       Add descriptor [l_{u_0}, l_{u_0}] and return Descriptors.
4. For each vertex p in canonically sorted order:
       If Children(p) is not empty:
           c_p = RightmostChild(p)
           Add descriptor [l_p, l_{c_p}] with size (index(c_p) - index(p) + 1)
5. Return Descriptors.

Algorithm MaterializeCliques(Descriptors, Graph G):
1. Initialize empty list of cliques Cliques.
2. For each desc in Descriptors:
       Clique = []
       For idx from desc.parentSortedIndex to desc.rightmostChildSortedIndex:
           Clique.append(G.sortedIntervals[idx].id)
       Cliques.append(Clique)
3. Return Cliques.
```

#### Correctness Proof
1. **Clique Property**: $l_{c_p} \le r_p$. For any $u, v \in [l_p, l_{c_p}]$, properness forces $l_u \le l_v \le l_{c_p} \le r_p \le r_u$, so $I_u \cap I_v \ne \emptyset$.
2. **Maximality**: Any vertex $w < p$ has $r_w < l_{c_p}$ (otherwise $w$ would be parent of $c_p$ instead of $p$), so $w \notin N(c_p)$. Any vertex $z > c_p$ has $l_z > r_p$, so $z \notin N(p)$. Thus no vertex outside can be added.
3. **Completeness & Uniqueness**: Each maximal clique has a unique leftmost interval $p$ that has children in $T(G)$. Monotonicity ensures distinct parents produce distinct, non-nested cliques.

#### Complexity Analysis
- **Preprocessing (Distance Tree)**: $\mathcal{O}(n + m)$ using BFS, or $\mathcal{O}(n)$ using two pointers on sorted intervals.
- **Core Descriptor Extraction**: $\mathcal{O}(n)$ by iterating over vertices and inspecting `childrenIds.back()`.
- **Materialization**: $\mathcal{O}(S)$ where $S = \sum |C_j|$.
- **Total Time**: $\mathcal{O}(n + S)$.
- **Auxiliary Space**: $\mathcal{O}(n)$ for tree nodes, $\mathcal{O}(q)$ for compact descriptors.

---

### 2.3 Dynamic Algorithm A Specification

#### Data Structures (`DynamicTheoremCliques`)
- `nodes`: Hash map of `DynamicTreeNode`, containing doubly-linked sibling pointers (`prevSibling`, `nextSibling`) and same-level pointers (`prevLevelSibling`, `nextLevelSibling`).
- `parentToRightmostChild`: `std::map<int, int>` storing $p \mapsto c_p$.
- `intervals`: Canonically sorted interval array.

#### Dynamic Insertion (Rightmost Append-Only Leaf)
```text
Algorithm DynamicInsertLeaf(NewInterval y):
1. Find parent p = lower_bound(intervals, y.left) such that r_p >= y.left.
2. Create node for y: depth = depth(p) + 1, level = level(p) + 1, parent = p.
3. Append y to child list of p:
       If childTail(p) == -1: childHead(p) = childTail(p) = y
       Else: link childTail(p) <-> y, childTail(p) = y
4. Append y to level list level(y).
5. Update descriptor dictionary:
       parentToRightmostChild[p] = y
6. Append y to intervals array.
7. Return success telemetry.
```

#### Dynamic Deletion (Theorem 3.16 Child Relocation)
```text
Algorithm DynamicDeleteVertex(VertexId u):
1. If u is root: reject (unsupported).
2. If u has children and u.nextLevelSibling == -1: reject (unsupported).
3. v = u.nextLevelSibling
4. If u has children:
       Prepend u's child list [u_left, u_right] to v's child list.
       For each child c in [u_left, u_right]: c.parentId = v.
5. Remove u from parent p(u)'s child list:
       If u == childTail(p(u)):
           childTail(p(u)) = u.prevSibling
6. Remove u from its level list.
7. Update descriptor dictionary:
       If u in parentToRightmostChild:
           Erase u.
           If v not in parentToRightmostChild:
               parentToRightmostChild[v] = u_right.
       If u was rightmost child of p(u):
           If childTail(p(u)) != -1: parentToRightmostChild[p(u)] = childTail(p(u))
           Else: Erase p(u)
8. Erase u from nodes and intervals.
9. Return success telemetry.
```

#### Descriptor Delta Bound
Across all possible valid deletion scenarios, at most 3 descriptors are affected:
- $\Delta_{\text{added}} \le 1$ ($v$ is added if and only if $v$ was previously a leaf).
- $\Delta_{\text{removed}} \le 2$ ($u$ is removed; and $p(u)$ is removed if and only if $u$ was the only child of $p(u)$).
- $\Delta_{\text{modified}} \le 1$ ($p(u)$ is modified if and only if $u$ was the rightmost child of $p(u)$ and $p(u)$ has remaining children).
- $\mathbf{\Delta_{\text{affected}} \le 3}$ (tight bound confirmed by 7-vertex Case B counterexample).

#### Complexity Analysis
- **Structural Insertion**: $\mathcal{O}(\log n)$ parent search + $\mathcal{O}(1)$ pointer updates + $\mathcal{O}(\log q)$ map update = $\mathcal{O}(\log n)$.
- **Structural Deletion**: $\mathcal{O}(\text{deg}(u))$ child parentId updates + $\mathcal{O}(1)$ pointer updates + $\mathcal{O}(\log q)$ map update = $\mathcal{O}(\text{deg}(u))$.
- **Current Implementation Note**: Map copying and global vector erasure inside the timed block add $\mathcal{O}(n \log n)$ instrumentation overhead.

---

# 3. Algorithm B: Pivoted Bron–Kerbosch

### 3.1 Mathematical Intuition
Bron–Kerbosch is a depth-first search backtracking algorithm that maintains three disjoint vertex sets:
- $R$: Currently accumulating clique.
- $P$: Prospective candidate vertices adjacent to all vertices in $R$.
- $X$: Excluded vertices already explored that cannot be added to $R$.

When $P$ and $X$ are both empty, $R$ is a maximal clique. To minimize search-tree branching, Tomita's pivot strategy selects a pivot $u \in P \cup X$ maximizing $|P \cap N(u)|$, and only branches on candidate vertices in $P \setminus N(u)$.

### 3.2 Pseudocode
```text
Algorithm BronKerboschPivoted(R, P, X, Graph G, Timeout):
1. If TimeoutExceeded(): mark timedOut and return.
2. If P is empty and X is empty:
       Emit R as a maximal clique and return.
3. Choose pivot u in P U X maximizing |P \cap N(u)|.
4. Candidates = P \ N(u).
5. For each vertex v in Candidates:
       R_next = R U {v}
       P_next = P \cap N(v)
       X_next = X \cap N(v)
       BronKerboschPivoted(R_next, P_next, X_next, G, Timeout)
       P = P \ {v}
       X = X U {v}
```

### 3.3 Complexity Analysis
- **Worst-Case Time**: $\mathcal{O}(3^{n/3})$ for general graphs (Moon–Moser bound).
- **On Interval Graphs**: While the number of maximal cliques is $q \le n$, the backtracking tree branches heavily when degree is high, resulting in exponential runtime on dense graphs.
- **Space**: $\mathcal{O}(n^2)$ recursion stack and candidate sets.

---

# 4. Algorithm C: Standard Interval Sweep-Line

### 4.1 Mathematical Intuition
In an interval graph, a maximal clique corresponds to a maximal set of intervals that share a common point. By scanning a vertical sweep line across all $2n$ interval endpoints from left to right:
- When a `START` event occurs, the interval enters the active set.
- When an `END` event occurs, the interval leaves the active set.
Just before an interval departs, the active set represents a locally maximal intersection. If at least one interval has entered since the last departure, the active set forms a maximal clique.

### 4.2 Pseudocode
```text
Algorithm SweepLineMaximalCliques(Graph G):
1. Events = []
2. For each interval iv with sorted index i:
       Events.append(Event{coord: iv.left, type: START, index: i})
       Events.append(Event{coord: iv.right, type: END, index: i})
3. Sort Events:
       coord ascending;
       if coordinates equal: START (0) precedes END (1);
       if types equal: interval ID ascending.
4. ActiveList = DoublyLinkedList()
5. Descriptors = []
6. hasNewStart = false
7. For each event ev in Events:
       If ev.type == START:
           ActiveList.push_back(ev.index)
           hasNewStart = true
       Else (ev.type == END):
           If hasNewStart and not ActiveList.empty():
               Descriptors.append(Descriptor{
                   firstIndex: ActiveList.front(),
                   lastIndex: ActiveList.back(),
                   size: ActiveList.size()
               })
               hasNewStart = false
           ActiveList.erase(ev.index)
8. Return Descriptors.
```

### 4.3 Complexity Analysis
- **Event Construction**: $\mathcal{O}(n)$.
- **Event Sorting**: $\mathcal{O}(n \log n)$ (or $\mathcal{O}(n)$ if intervals are already sorted by left and right endpoints).
- **Sweep Core**: $\mathcal{O}(n)$ using list iterators for $\mathcal{O}(1)$ deletions.
- **Materialization**: $\mathcal{O}(S)$ where $S = \sum |C_j|$.
- **Total Time**: $\mathcal{O}(n \log n + S)$.
- **Space**: $\mathcal{O}(n)$ for event queue and active list.

---

# 5. Implementation Mapping to Source Code

| Algorithm Component | Header File | Implementation File | Key Functions |
|:---|:---|:---|:---|
| **Distance Tree Construction** | `src/DistanceTree.h` | `src/DistanceTree.cpp` | `DistanceTree::build()`, `findParent()` |
| **Algorithm A Static** | `benchmarks/maximal_cliques/theorem_cliques_adapter.h` | `theorem_cliques_adapter.cpp` | `computeCompactDescriptors()`, `materializeCliques()` |
| **Algorithm A Dynamic** | `benchmarks/maximal_cliques/dynamic_clique_structure.h` | `dynamic_clique_structure.cpp` | `insertLeaf()`, `deleteVertex()`, `materializeCurrentCliques()` |
| **Algorithm B (Bron–Kerbosch)** | `benchmarks/maximal_cliques/bron_kerbosch_adapter.h` | `bron_kerbosch_adapter.cpp` | `runPivotedWithTimeout()`, `recursivePivoted()` |
| **Algorithm C (Sweep-Line)** | `benchmarks/maximal_cliques/standard_interval_cliques.h` | `standard_interval_cliques.cpp` | `buildSortedEvents()`, `sweepCore()`, `materializeCliques()` |
| **Verification Harness** | `benchmarks/maximal_cliques/clique_verifier.h` | `clique_verifier.cpp` | `verifyStaticGraph()`, `verifyDynamicSequence()`, `areEqual()` |
| **Benchmark Runner** | `benchmarks/maximal_cliques/benchmark_runner.h` | `benchmark_runner.cpp` | `evaluateStaticGraph()`, `evaluateDynamicSequence()`, `runStaticBenchmarks()` |
