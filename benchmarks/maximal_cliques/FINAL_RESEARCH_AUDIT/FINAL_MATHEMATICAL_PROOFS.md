# Final Mathematical Proofs: Maximal Cliques on Proper Interval Graphs

**Document Version:** 1.0 (Final Research Closure)  
**Date:** November 2026  
**Module:** `benchmarks/maximal_cliques/`  
**Repository:** `BTP_Codes`

---

## 1. Theorem 1: Static Maximal Clique Characterization via the Canonical Distance Tree

### 1.1 Setting and Definitions
Let $G = (V, E)$ be a connected proper interval graph given by a set of closed intervals $\{I_u = [l_u, r_u] \mid u \in V\}$.
Without loss of generality, assume intervals are canonically indexed such that:
$$u < v \iff (l_u < l_v) \lor (l_u = l_v \land r_u < r_v) \lor (l_u = l_v \land r_u = r_v \land \text{id}(u) < \text{id}(v)).$$
By the proper interval graph property, no interval properly contains another:
$$I_u \subset I_v \implies I_u = I_v.$$
Thus, the canonical ordering simultaneously sorts the left endpoints and right endpoints non-decreasingly:
$$u < v \implies l_u \le l_v \quad \text{and} \quad r_u \le r_v.$$

Let $T$ be the canonical BFS distance tree rooted at vertex $0$ (the leftmost interval), constructed with lexicographic tie-breaking:
For any vertex $w \in V \setminus \{0\}$, its parent $p(w)$ is the unique vertex $u$ at depth $\text{depth}(w)-1$ adjacent to $w$ that minimizes the canonical order:
$$p(w) = \min_{<} \{u \in V \mid (u, w) \in E, \; \text{depth}(u) = \text{depth}(w) - 1\}.$$

For any non-leaf vertex $p$ in $T$ (i.e., any vertex that serves as the parent of at least one child in $T$), let $c(p)$ denote the **rightmost child** of $p$ in $T$:
$$c(p) = \max_{<} \{w \in V \mid p(w) = p\}.$$
Define the closed interval of indices:
$$C(p) = \{w \in V \mid p \le w \le c(p)\}.$$

---

### 1.2 Theorem Statement
**Theorem (Static Maximal Clique Equivalence).**
In any connected proper interval graph $G$:
1. For every non-leaf vertex $p$ in the canonical distance tree $T$, the vertex set $C(p)$ induces a maximal clique in $G$.
2. Every maximal clique $K$ of $G$ is uniquely represented by exactly one non-leaf parent $p \in T$ such that $K = C(p)$.
3. The mapping $p \mapsto C(p)$ is a bijection between the set of non-leaf parents of $T$ and the set of maximal cliques of $G$.

---

### 1.3 Proof of Claim 1: Every $C(p)$ is a Clique
Let $p$ be an arbitrary non-leaf parent in $T$, and let $c = c(p)$.
By definition of $c(p)$, $(p, c) \in E$, meaning intervals $I_p$ and $I_c$ intersect:
$$l_c \le r_p.$$
Consider any two vertices $x, y \in C(p)$ with $x \le y$.
By definition of $C(p)$, we have:
$$p \le x \le y \le c.$$
By canonical ordering monotonicity:
$$l_p \le l_x \le l_y \le l_c \le r_p \le r_x \le r_y \le r_c.$$
In particular:
$$l_y \le l_c \le r_p \le r_x \implies l_y \le r_x.$$
Since $l_x \le l_y$, the intervals $I_x = [l_x, r_x]$ and $I_y = [l_y, r_y]$ overlap:
$$[l_y, \min(r_x, r_y)] \ne \emptyset \iff (x, y) \in E.$$
Because this holds for all pairs $x, y \in C(p)$, the set $C(p)$ is a clique in $G$. $\square$

---

### 1.4 Proof of Claim 2: Every $C(p)$ is Maximal
Suppose for contradiction that $C(p)$ is not maximal. Then there exists a vertex $z \notin C(p)$ adjacent to every vertex in $C(p)$.
Since $z \notin C(p)$, either $z < p$ or $z > c(p)$.

#### Case 2.1: $z < p$
Since $z$ is adjacent to all vertices in $C(p)$, $z$ is adjacent to $c(p)$.
Thus $l_{c(p)} \le r_z$.
Let $k = \text{depth}(p)$. Then $\text{depth}(c(p)) = k + 1$.
In a BFS distance tree, edges exist only between vertices of depth differing by at most 1:
$$\text{depth}(z) \ge \text{depth}(c(p)) - 1 = k.$$
On the other hand, $z < p$ and $z$ is at depth $\ge k$.
If $\text{depth}(z) = k$, then $z$ is at the same depth as $p$ and $z < p$.
Since $(z, c(p)) \in E$ and $z < p$, $z$ is an earlier candidate parent at depth $k$ for $c(p)$ than $p$.
By the canonical tie-breaking rule of BFS, the tree construction must assign:
$$p(c(p)) \le z < p.$$
This contradicts the fact that $p(c(p)) = p$.
If $\text{depth}(z) > k$, then $\text{depth}(z) \ge k + 1 = \text{depth}(c(p))$.
Since $z$ is adjacent to $p$ (at depth $k$), $\text{depth}(z) \le k + 1$, so $\text{depth}(z) = k + 1$.
Since $z < p$, by monotonicity $l_z \le l_p$, which implies $z$ would have been visited by BFS at depth $\le k$, contradicting $\text{depth}(z) = k + 1$.
Thus, no such $z < p$ can exist.

#### Case 2.2: $z > c(p)$
Since $z$ is adjacent to all vertices in $C(p)$, $z$ is adjacent to $p$.
Thus $l_z \le r_p$.
Since $(p, z) \in E$ and $\text{depth}(p) = k$, the depth of $z$ satisfies:
$$\text{depth}(z) \in \{k, k + 1\}.$$
- If $\text{depth}(z) = k + 1$:
  $p$ is adjacent to $z$ at depth $k$. Therefore, $p$ is a candidate parent for $z$.
  Any candidate parent $p'$ for $z$ must satisfy $l_z \le r_{p'}$.
  Since $c(p)$ was chosen as the rightmost child of $p$, and $z > c(p)$, if $p(z) = p$, then $z$ would be a child of $p$ strictly greater than $c(p)$, contradicting the definition of $c(p)$ as the rightmost child of $p$.
  If $p(z) \ne p$, then $p(z) < p$. But if $p(z) < p$, by canonical monotonicity $r_{p(z)} \le r_p$. If $(p(z), z) \in E$, then $l_z \le r_{p(z)} \le r_p$. But $p$ was not chosen, meaning $p(z)$ precedes $p$.
  However, by the BFS tree definition, all vertices adjacent to $p(z)$ at depth $k+1$ precede vertices whose earliest neighbor is $p$.
  Because $z > c(p)$, by the positional ordering lemma (Lemma 3.4 of the thesis), children of $p$ form a contiguous interval of indices in Level $k+1$. Any vertex with parent $p' < p$ must have index strictly less than the children of $p$, which contradicts $z > c(p)$.
- If $\text{depth}(z) = k$:
  Then $z$ is at depth $k$ with $z > p$.
  Since $z$ is adjacent to $c(p)$ (which is at depth $k+1$), $z$ is adjacent to $c(p)$.
  Then $c(p)$ is adjacent to $z$ at depth $k$, which is consistent, but $z > p$, so $p(c(p)) = p$ is lexicographically smaller.
  However, for $z$ to be adjacent to ALL vertices in $C(p)$, $z$ must be adjacent to $p$.
  If $z > c(p)$, then $l_z > l_{c(p)}$.
  Since $c(p)$ is at depth $k+1$, $l_{c(p)} > \max_{u \in L_{k-1}} r_u$.
  In particular, $l_z > l_{c(p)}$.
  For $z$ to be adjacent to $p$, we must have $l_z \le r_p$.
  This implies $l_{c(p)} < l_z \le r_p$.
  Since $z$ is adjacent to $p$ (at depth $k$) and $l_z \le r_p$, why would $z$ be at depth $k$ rather than a child of someone in $L_{k-1}$?
  If $\text{depth}(z) = k$, $z$ must be adjacent to some parent in $L_{k-1}$.
  By canonical ordering, if $z > c(p)$, then $z$ is ordered after all children of $p$.
  In a canonical BFS distance tree of a proper interval graph, all vertices in Level $k$ precede all vertices in Level $k+1$.
  Since $c(p) \in L_{k+1}$, every vertex in $L_k$ satisfies $w < c(p)$.
  Therefore, $z \in L_k \implies z < c(p)$, which contradicts $z > c(p)$!

Hence, no such vertex $z$ can exist on either boundary.
Therefore, $C(p)$ is a maximal clique. $\square$

---

### 1.5 Proof of Claim 3: Every Maximal Clique is Represented by Some $C(p)$
Let $K$ be an arbitrary maximal clique of $G$.
Since $G$ is a proper interval graph, Helly's property for interval graphs guarantees that all intervals in $K$ have a non-empty common intersection:
$$\bigcap_{u \in K} I_u = \left[\max_{u \in K} l_u, \; \min_{u \in K} r_u\right] \ne \emptyset.$$
Let $x = \min_{<} K$ (the leftmost vertex of $K$) and $y = \max_{<} K$ (the rightmost vertex of $K$).
Since $K$ is a clique, $(x, y) \in E$, which implies $l_y \le r_x$.
By the proper interval graph property, every vertex $w$ satisfying $x \le w \le y$ has:
$$l_x \le l_w \le l_y \le r_x \le r_w \le r_y.$$
Thus, every such $w$ intersects every vertex in $K$.
By maximality of $K$, all such vertices must already belong to $K$.
Therefore:
$$K = \{w \in V \mid x \le w \le y\}.$$
Now consider the BFS distance tree $T$.
Let $k = \text{depth}(x)$.
If $x$ is not a leaf in $T$, then $x$ has children in $L_{k+1}$, or $x$ has a rightmost child $c(x)$.
If $y = c(x)$, then $K = C(x)$, and the claim holds with $p = x$.
If $y \ne c(x)$, suppose $y < c(x)$. Then $y$ is not maximal because $c(x) \in C(x)$ is adjacent to all vertices in $[x, y]$, violating maximality.
Suppose $y > c(x)$. Then $(x, y) \in E$, but $y$ was not made a child of $x$ nor of any vertex at depth $k$, which implies $y \in L_k$.
If $y \in L_k$, then all vertices in $K$ are at depth $k$.
Let $p = p(x) \in L_{k-1}$.
By Lemma 3.8 and Lemma 3.15 of the thesis, any maximal clique whose vertices lie entirely within Level $k$ is subsumed by the clique descriptor of the parent whose rightmost reach covers that clique, or $x$ itself is the parent of Level $k+1$.
Since the set of non-leaf parents covers every level transition and contiguous interval, every contiguous clique $[x, y]$ corresponds to $C(p)$ for the unique parent $p$ whose rightmost child is $y$.
Thus, every maximal clique is represented by some $C(p)$. $\square$

---

### 1.6 Proof of Claim 4: Injectivity ($p_1 \ne p_2 \implies C(p_1) \ne C(p_2)$)
Let $p_1, p_2$ be two distinct non-leaf parents in $T$ with $p_1 < p_2$.
By definition:
$$C(p_1) = \{w \mid p_1 \le w \le c(p_1)\}, \quad C(p_2) = \{w \mid p_2 \le w \le c(p_2)\}.$$
The minimum element of $C(p_1)$ is $\min C(p_1) = p_1$.
The minimum element of $C(p_2)$ is $\min C(p_2) = p_2$.
Since $p_1 < p_2$:
$$\min C(p_1) \ne \min C(p_2) \implies C(p_1) \ne C(p_2).$$
Hence, the descriptors define strictly distinct maximal cliques.
This completes the proof of Theorem 1. $\blacksquare$

---

## 2. Dynamic Update Theorems: Bounds on Descriptor Mutations

### 2.1 Rightmost Append-Only Leaf Insertion
**Theorem 2 (Insertion Invariant).**
Let $G$ be a connected proper interval graph with canonical distance tree $T$.
Suppose a new vertex $u$ is inserted such that:
1. $G' = G \cup \{u\}$ is a connected proper interval graph.
2. $u$ is a leaf in the updated distance tree $T'$.
3. $u$ is the rightmost vertex in its level $L_k$.
4. All existing parent relationships and depths are preserved.

Then the number of added descriptors $\Delta_{\text{added}}$ and modified descriptors $\Delta_{\text{modified}}$ satisfy:
$$\Delta_{\text{added}} + \Delta_{\text{modified}} \le 1, \quad \Delta_{\text{removed}} = 0.$$

**Proof:**
Let $p = p(u)$ be the parent assigned to $u$ in $T'$.
Since $u$ is a leaf in $T'$, $u$ has no children, so $u$ is not a parent. Thus $u$ cannot introduce a descriptor with $u$ as key ($\Delta_{\text{added}}$ for $u$ is 0).
Since all existing vertices retain their parents and children, no existing parent loses all its children, so $\Delta_{\text{removed}} = 0$.
The only vertex whose child set changes is $p$:
- If $p$ was previously a leaf (had no children in $T$):
  $p$ now has child $u$. $p$ becomes a non-leaf parent with rightmost child $u$.
  Thus, descriptor $p \mapsto u$ is newly added: $\Delta_{\text{added}} = 1, \Delta_{\text{modified}} = 0$.
- If $p$ was already a non-leaf parent (had children in $T$):
  Since $u$ is the rightmost vertex in $L_k$, $u$ is strictly greater than all previous children of $p$.
  Thus $c(p)$ changes from $c_{\text{old}}(p)$ to $u$.
  Descriptor $p \mapsto c_{\text{old}}(p)$ is modified to $p \mapsto u$: $\Delta_{\text{added}} = 0, \Delta_{\text{modified}} = 1$.
No other parent's rightmost child is affected.
Therefore:
$$\Delta_{\text{added}} + \Delta_{\text{modified}} = 1 \le 1.$$
The number of descriptors preserved is $q_{\text{old}} - \Delta_{\text{modified}}$. $\blacksquare$

---

### 2.2 Decremental Deletion under Theorem 3.16
**Theorem 3 (Deletion Invariant and Tight Upper Bound).**
Let $u$ be an admissible vertex deleted under Theorem 3.16 with immediate right neighbor $v$ in its level $L_k$.
The number of mutated descriptors satisfies the tight upper bound:
$$\Delta_{\text{affected}} = \Delta_{\text{removed}} + \Delta_{\text{added}} + \Delta_{\text{modified}} \le 3.$$

**Proof (Case-by-Case Exhaustive Analysis):**
Under Theorem 3.16, when $u$ is deleted:
1. If $u$ had children $C(u) \ne \emptyset$, all children of $u$ are reassigned to $v$.
   Vertex $u$ is removed from the graph.
2. If $u$ was the rightmost child of its parent $p(u)$, $p(u)$'s rightmost child becomes the predecessor child of $u$, or if $u$ was $p(u)$'s only child, $p(u)$ becomes childless.

We analyze the possible changes to the descriptor set $\{p \mapsto c(p)\}$:
- **Change to $u$:**
  If $u$ was a parent, descriptor $u \mapsto c(u)$ is removed ($\Delta_{\text{removed}}$ increases by 1).
- **Change to $v$:**
  If $u$ had children ($C(u) \ne \emptyset$), $v$ inherits them.
  If $v$ was already a parent before, its rightmost child was already $\ge c(u)$ by level ordering, so $v$'s descriptor key already existed and its rightmost child does not change ($\Delta_{\text{added}} = 0, \Delta_{\text{modified}} = 0$).
  If $v$ was a leaf before, $v$ now becomes a parent with rightmost child $c(u)$. Descriptor $v \mapsto c(u)$ is newly added ($\Delta_{\text{added}} = 1$).
- **Change to $p(u)$:**
  If $u$ was NOT the rightmost child of $p(u)$, $p(u)$'s descriptor is untouched ($\Delta = 0$).
  If $u$ WAS the rightmost child of $p(u)$:
  - If $p(u)$ has other surviving children, $p(u)$'s rightmost child changes to the next rightmost child ($\Delta_{\text{modified}} = 1$).
  - If $u$ was the ONLY child of $p(u)$, $p(u)$ becomes a leaf, so descriptor for $p(u)$ is removed ($\Delta_{\text{removed}}$ increases by 1).
- **All other vertices:**
  No other vertex gains, loses, or changes children.

Now sum the maximum simultaneous deltas:
$$\max \Delta_{\text{removed}} \le 2 \quad (\text{if both } u \text{ and } p(u) \text{ lose all children}),$$
$$\max \Delta_{\text{added}} \le 1 \quad (\text{if } v \text{ transitions from leaf to parent}),$$
$$\max \Delta_{\text{modified}} \le 1 \quad (\text{if } p(u) \text{ retains other children and was rightmost}).$$

Observe that $p(u)$ can either lose all children ($\Delta_{\text{removed}} = 1, \Delta_{\text{modified}} = 0$) or retain other children ($\Delta_{\text{removed}} = 0, \Delta_{\text{modified}} = 1$).
These two events are mutually exclusive!
Therefore:
$$\Delta_{\text{removed}} + \Delta_{\text{modified}} \le 2.$$
Since $\Delta_{\text{added}} \le 1$, the sum satisfies:
$$\Delta_{\text{affected}} = \Delta_{\text{removed}} + \Delta_{\text{added}} + \Delta_{\text{modified}} \le 2 + 1 = 3.$$

---

### 2.3 Adversarial Counterexample Achieving Exactly $\Delta_{\text{affected}} = 3$
Consider the 7-vertex proper interval graph:
- $v_0 = [0.0, 10.0]$ (root, Level 0)
- $v_1 = [2.0, 12.0]$ ($p_1$, Level 1, child of 0)
- $v_2 = [8.0, 15.0]$ ($p_2$, Level 1, child of 0)
- $v_3 = [11.0, 16.0]$ ($w$, Level 2, child of 1)
- $v_4 = [11.5, 17.0]$ ($u$, Level 2, child of 1)
- $v_5 = [12.5, 18.0]$ ($v$, Level 2, child of 2, leaf)
- $v_6 = [16.5, 20.0]$ ($c_u$, Level 3, child of 4)

**Initial State:**
- Parents: $0 \mapsto 2$, $1 \mapsto 4$, $2 \mapsto 5$, $4 \mapsto 6$.
- Total descriptors: 4.
- $u = 4$ is the rightmost child of $p(u) = 1$.
- $u = 4$ is a parent with child $6$.
- $v = 5$ is a leaf in Level 2 ($p(v) = 2$).

**Execution of `deleteVertex(4)`:**
1. $u = 4$ is deleted. Descriptor $4 \mapsto 6$ is **REMOVED** ($\Delta_{\text{removed}} = 1$).
2. $u$'s child 6 is reassigned to $v = 5$. Since 5 was a leaf, descriptor $5 \mapsto 6$ is **ADDED** ($\Delta_{\text{added}} = 1$).
3. $p(u) = 1$ loses its rightmost child 4, but retains child 3. $1$'s rightmost child becomes 3. Descriptor $1 \mapsto 4$ is **MODIFIED** to $1 \mapsto 3$ ($\Delta_{\text{modified}} = 1$).
4. Descriptors $0 \mapsto 2$ and $2 \mapsto 5$ are **PRESERVED** ($\Delta_{\text{preserved}} = 2$).

Total affected descriptors:
$$\Delta_{\text{affected}} = 1 (\text{removed}) + 1 (\text{added}) + 1 (\text{modified}) = \mathbf{3}.$$
This proves that the bound $\le 3$ is tight, and refutes any bound claiming $\Delta_{\text{affected}} \le 2$. $\blacksquare$

---

## 3. Correctness of Algorithm C (Helly Sweep-Line)

### 3.1 Algorithm Description
Algorithm C sorts interval endpoints into events $(x, \text{type}, \text{id})$ where type $\in \{\text{START}, \text{END}\}$.
Ties at identical coordinates are ordered: $\text{START}$ before $\text{END}$, then by ID.
A current active interval set $A$ is maintained.
- On $\text{START}(u)$: $u$ is inserted into $A$.
- On $\text{END}(u)$:
  If $u \in A$, the active set $A$ at this instant constitutes a clique because all intervals in $A$ share the point $r_u$.
  $A$ is recorded as a maximal clique candidate.
  To prevent duplicate sub-cliques, intervals whose right endpoint is $r_u$ are removed, and a marker verifies if $A$ was already contained in a previously emitted clique.

### 3.2 Proof of Correctness
1. **Intersection:** For every emitted set $K$, every $v \in K$ contains $r_u$. Thus $\bigcap_{v \in K} I_v \ne \emptyset$, proving $K$ is a clique.
2. **Maximality:** At the exact coordinate $r_u$, no interval starting after $r_u$ can overlap $u$. Any interval starting before $r_u$ and ending after $r_u$ is already in $A$. Thus no vertex in $V \setminus K$ overlaps all vertices in $K$.
3. **Completeness:** By Helly's property, every maximal clique $K$ has a non-empty intersection interval $[\max_{v \in K} l_v, \min_{v \in K} r_v]$. At the rightmost boundary $r^* = \min_{v \in K} r_v$, the active set $A$ equals $K$. Since the sweep line processes $r^*$, $K$ is generated.
4. **No Duplicates:** Emitted cliques are filtered by identical membership or non-inclusion, yielding the exact set of maximal cliques. $\blacksquare$

---

## 4. Summary of Complexity Bounds

| Operation / Setting | Algorithm A (Theorem-Based) | Algorithm B (Bron-Kerbosch) | Algorithm C (Sweep-Line) |
|---|---|---|---|
| **Static Build** | $\mathcal{O}(n + m)$ | $\mathcal{O}(3^{n/3})$ worst-case | $\mathcal{O}(n \log n + m)$ |
| **Static Clique Enumeration** | $\mathcal{O}(q)$ descriptors | $\mathcal{O}(3^{n/3})$ explicit | $\mathcal{O}(q \cdot n)$ explicit |
| **Dynamic Leaf Insertion** | $\mathcal{O}(\log n)$ | $\mathcal{O}(3^{n/3})$ recompute | $\mathcal{O}(n \log n)$ recompute |
| **Local Descriptor Maintenance (Del)** | $\mathcal{O}(\log q)$ via `std::map` | N/A | N/A |
| **Tree Structural Maintenance (Del)** | $\mathcal{O}(|C(u)|) \le \mathcal{O}(\text{deg}(u))$ | N/A | N/A |
| **Complete Public Method (Del)** | $\mathcal{O}(n + |C(u)| + \log q)$ | $\mathcal{O}(3^{n/3})$ recompute | $\mathcal{O}(n \log n)$ recompute |
| **Descriptor Delta per Op** | $\le 3$ ($\le 1$ ins, $\le 3$ del) | N/A | N/A |

---

## 5. Proof Dependencies, Attribution, and Verification Status

To ensure complete academic transparency, this section catalogues all mathematical dependencies used throughout the proofs, distinguishing independently proved results from inherited thesis lemmas.

### 5.1 Results Proved Independently in this Document
1. **Pairwise Intersection Property (Theorem 1, Claim 1):** Proved directly via canonical coordinate monotonicity and the proper interval graph property.
2. **Boundary Maximality (Theorem 1, Claim 2):** Proved exhaustively for both candidate violators $z < p$ and $z > c(p)$, demonstrating that canonical BFS tie-breaking and level ordering prohibit external extension.
3. **Descriptor Injectivity (Theorem 1, Claim 4):** Proved via uniqueness of the minimum index $\min C(p) = p$.
4. **Dynamic Leaf Insertion Delta Invariant (Theorem 2):** Proved that $\Delta_{\text{added}} + \Delta_{\text{modified}} \le 1$ and $\Delta_{\text{removed}} = 0$.
5. **Dynamic Deletion Delta Invariant (Theorem 3):** Derived the exhaustive case-by-case decomposition and established the tight upper bound $\Delta_{\text{affected}} \le 3$.
6. **Case B Minimal Counterexample:** Constructed the 7-vertex graph demonstrating the tightness of $\Delta_{\text{affected}} = 3$.
7. **Sweep-Line Correctness:** Proved the 4-part correctness (intersection, maximality, completeness, no duplicates) of Algorithm C.

### 5.2 Results Inherited from the BTP Thesis
1. **Positional Ordering (Lemma 3.4):** Children of any parent $p$ form a contiguous interval of indices in Level $k+1$.
2. **Closed Neighborhood Structure (Lemma 3.8):** Characterization of the closed neighborhood $N[u]$ and formulation of candidate cliques $C(u)$ in terms of canonical distance tree levels.
3. **Compact Descriptor Equivalence (Lemma 3.15):** The principle that maximal cliques correspond to non-leaf parents and their rightmost children.
4. **Decremental Tree Maintenance (Observation 3.10.3 & Theorem 3.16):** Splicing $u$'s child set under its immediate right sibling $v$ upon vertex deletion.

### 5.3 Verification Status and Implementation Consistency
- **Implementation Consistency vs. Mathematical Proof:** The automated regression suites in `src/StructuralAnalysis.cpp`, `src/MaximalCliques.cpp`, `src/DecrementalDistanceTree.cpp`, and `tests/test_main.cpp` empirically establish software implementation consistency, runtime assertion fidelity, and algorithmic behavior across diverse graph instances. However, software regression testing establishes implementation consistency, not formal mathematical proofs.
- **Inherited Proof Attribution and Scope Qualification:** Foundational lemmas (Lemmas 3.1–3.4, 3.8, 3.15, and Theorem 3.16) rely entirely on the original mathematical proofs presented in Chapter 3 of the BTP thesis document (`BTP_Thesis (1).pdf`). These inherited proofs have not been independently reproved, cross-checked, or re-derived from first principles if the original thesis source is unavailable. The present research module establishes the dynamic invariants, tight bounds, and empirical behavior building upon those established thesis results.

