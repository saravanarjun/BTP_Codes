# Validation Report: Mathematical Verifications and Test Ledger

This report tracks the formal verification and empirical testing of each definition, lemma, theorem, and algorithm implemented from the thesis:
**"Dynamic Graph Algorithms - Interval Graphs"** by **K. Saravanarjun** (IIT Bhubaneswar, August 2026).

---

## Status Ledger

| Reference | Mathematical Result / Algorithm | Implementation Function | Verification Method | Status | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Section 2.2, 2.5** | Interval Representation & Ordering | `Interval::operator<`, `IntervalGraph::initFromIntervals` | Strict weak ordering, ID tie-breaker | **PASS** | Phase 1 unit tests |
| **Section 2.3** | Properness Condition ($\neg \exists u \neq v: I_u \subset I_v$) | `IntervalGraph::validateProperness` | $O(n)$ check on sorted endpoints | **PASS** | Tested against containment |
| **Section 2.1** | Graph Connectedness | `IntervalGraph::validateConnectedness` | $O(n)$ sweep-line gap detection | **PASS** | Tested on single & split graphs |
| **Section 2.2** | Interval Adjacency ($I_u \cap I_v \neq \emptyset$) | `IntervalGraph::isAdjacent` | Direct overlap test $\ell_u \le r_v \land \ell_v \le r_u$ | **PASS** | Verified on overlapping & touching |
| **Eq (2.1), (3.1)** | Distance Tree Parent Construction | `DistanceTree::build`, `computeParentsBruteForce` | Evaluated vs $O(n^2)$ definition: $\arg\min_{u: r_u \ge \ell_v} \ell_u$ | **PASS** | Validated on 7 test topologies |
| **Section 2.8, 3.3** | Level Structure & Tree Traversals | `DistanceTree::computeLevelOrder`, `computePreorder`, `computePostorder` | BFS level order, preorder, postorder verification | **PASS** | 0-based depth & 1-based level |
| **Eq (2.2)** | Distance Encoding Formula | `DistanceVerification::evaluateEquation2_2Raw`, `getDistanceFormula` | Full pairwise comparison with BFS ground truth on interval graph | **PASS** | 7,856 pairs tested across 51 graphs, 0 mismatches |
| **Section 3.10, Lemma 3.1** | Parent-children clique: $\{u, u_1, \dots, u_k\}$ forms a clique in $G$ | `StructuralAnalysis::verifyLemma3_1` | Pairwise interval intersection check of $\{u\} \cup \text{Children}(u)$ for all nodes | **PASS** | 56 graphs, 0 violations |
| **Section 3.10, Observation** | Depth locality of non-level edges: if $uv \in E$ and $\text{depth}(u) \neq \text{depth}(v)$, then $\|\text{depth}(u) - \text{depth}(v)\| = 1$ | `StructuralAnalysis::verifyDepthLocality` | Depth difference audit across all intersecting interval pairs | **PASS** | 56 graphs, 0 violations |
| **Section 3.10, Lemma 3.2** | Child cannot be adjacent to parent's left sibling: $qv \notin E$ | `StructuralAnalysis::verifyLemma3_2` | Negative interval overlap check between child $v$ and left peers/siblings $q$ of $parent(v)$ | **PASS** | 56 graphs, 0 violations |
| **Section 3.10, Lemma 3.3** | Adjacency with right siblings of parent: child $v$ of $u$ is adjacent to right sibling $w$ of $u$ ($vw \in E$) | `StructuralAnalysis::verifyLemma3_3` | Pairwise interval intersection test of $(v, w)$ for all right peers/siblings $w$ | **PASS** | 56 graphs, 0 violations |
| **Section 3.10, Lemma 3.4** | Clique formed by node, children, and right siblings: $\{u\} \cup \text{Children}(u) \cup \text{RightSiblings}(u)$ forms a clique | `StructuralAnalysis::verifyLemma3_4` | All-pairs interval intersection within set $S_u$ for all nodes $u$ | **PASS** | 56 graphs, 0 violations |
| **Section 3.10, Lemma 3.5** | Mutual adjacency of siblings' children: child $x$ of left sibling $v$ is adjacent to child $w$ of right sibling $u$ ($xw \in E$) | `StructuralAnalysis::verifyLemma3_5` | Pairwise interval intersection check between child sets of sibling pairs | **PASS** | 56 graphs, 0 violations |
| **Section 3.10, Lemma 3.6** | Propagation of adjacency to children: if $u, v \in L_d$ and $uv \in E$, then every child of $u$ is adjacent to every child of $v$ | `StructuralAnalysis::verifyLemma3_6` | Cross-child interval intersection checks for adjacent same-depth pairs | **PASS** | 56 graphs, 0 violations |
| **Section 3.10, Theorem 3.7** | Each depth level induces a clique: all vertices occurring at depth $i$ form a clique in $G$ | `StructuralAnalysis::verifyTheorem3_7` | Exhaustive all-pairs interval intersection check within every level $L_d$ | **PASS** | 56 graphs, 297 level-clique pairs, 0 violations |
| **Section 3.10, Lemma 3.8** | Maximal clique characterization: $C(u) = \{u\} \cup \text{Children}(u) \cup \text{Right}(u) \cup \text{Children}(\text{Left}(u))$ | `MaximalCliques::constructCliqueLemma3_8`, `extractUniqueMaximalCliques` | Clique check + maximality check + Bron-Kerbosch comparison | **PASS** | Validated: non-leaf nodes define all maximal cliques; leaf nodes produce sub-cliques |
| **Section 3.10, Lemma 3.15** | Compact representation $[\ell_p, \ell_c]$: $p$ is parent, $c$ is rightmost child | `MaximalCliques::constructCompactCliques` | Expanded interval vs Bron-Kerbosch and Lemma 3.8 | **PASS** | 56 graphs, 373 total maximal cliques, 100% exact match |
| **Section 3.10, Lemma 3.11** | MIS bounds: $\lfloor h/2 \rfloor \le \alpha(G) \le h-1$ ($h \ge 2$) | `IndependentSet::verifyBounds` | Bounds check against exact $\alpha(G)$; explicit handling of $h=1$ | **PASS** | 56 graphs tested; $h=1$ boundary case verified separately |
| **Section 3.10, Theorem 3.12** | 2-approximation for MIS: size $\lceil h/2 \rceil$ | `IndependentSet::computeApproximation` | Independence check + ratio $\le 2.0$ guarantee | **PASS** | 56 graphs tested; 100% independent and $\le 2.0$ ratio |
| **Section 3.10.1, Theorem 3.13** | Exact MIS via DP ($O(n)$ time, $O(n)$ space) | `IndependentSet::computeExactDP` | Verified against independent brute-force oracle on small graphs | **PASS** | 56 graphs tested; 100% agreement with brute-force optimum |
| **Theorem 3.17** | Dominating set upper bound $\lceil (h-1)/2 \rceil$ (and $\le (h-1)/2$ for odd $h$) | `DominatingSet::constructDominatingSet`, `verifyDominatingSet` | Independent interval intersection domination check + brute-force MDS oracle | **PASS** | 58 graphs tested; 100% covered, 100% bound compliance, $h=1$ handled |
| **Theorem 3.14** | Incremental MIS maintenance ($O(\log n)$ internal update, $O(n)$ validated) | `IncrementalIndependentSet::insertLeaf` | Dynamic update vs static Phase 6 DP and brute-force MIS oracle | **PASS** | Rightmost append-only leaf model; 19 unit suites + 180 random sequential insertions (100% exact match) |
| **Theorem 3.16** | Decremental child relocation ($O(\log n)$ internal update, $O(n)$ validated) | `DecrementalDistanceTree::deleteVertex` | Full rebuild comparison & brute-force MIS oracle | **PASS** | Observation 3.10.3; 17 unit suites + 240 random sequential deletions (100% exact match) |

---

## Detailed Phase 7 Validation: Dominating Set (Theorem 3.17)

### 1. Theorem Statement & Reference
- **Thesis Reference**: Chapter 3, Section 3.10, page 47 (numbered page 54 of thesis PDF).
- **Exact Formulation**:
  Let $G$ be a proper interval graph whose distance tree $T(G)$ has height $h$. Then
  $$\gamma(G) \le \left\lceil \frac{h-1}{2} \right\rceil$$
  In particular, when $h$ is odd, $\gamma(G) \le \frac{h-1}{2}$.

### 2. Thesis Construction & Mathematical Derivation
1. **Level 2 Rightmost Vertex Selection**:
   - Select the rightmost vertex $u^*_2$ of Level 2 (1-based level 2, 0-based depth 1).
   - **Level 1 Domination**: Since $u^*_2$ is at depth 1, its distance from root $v_1$ is 1, so $u^*_2$ is adjacent to $v_1$.
   - **Level 2 Domination**: By Theorem 3.7, Level 2 induces a clique, so $u^*_2$ is adjacent to every other vertex in Level 2.
   - **Level 3 Domination**: Every vertex $v \in L_3$ has its parent $p \in L_2$.
     - If $p = u^*_2$, $v$ is a child of $u^*_2$, so $u^*_2 v \in E$.
     - If $p < u^*_2$, notice that all vertices in Level 2 share the same parent (root $v_1$). Thus $p$ and $u^*_2$ are genuine tree siblings. By Lemma 3.3 ("child is adjacent to parent's right sibling"), child $v$ is adjacent to $u^*_2$.
     - Thus $u^*_2$ dominates all vertices in Levels 1, 2, and 3.
2. **Successive Level Pairs $(4, 5), (6, 7), (8, 9), \dots$**:
   - For each pair $(i, i+1)$ with $i \le h$, select the rightmost vertex $u^*_i$ of Level $i$.
   - **Level $i$ Domination**: By Theorem 3.7, Level $i$ induces a clique, so $u^*_i$ dominates all vertices in Level $i$.
   - **Level $i+1$ Domination & Formal Geometric Proof**:
     - *Issue with "Sibling" Terminology*: Vertices in Level $i \ge 4$ generally have different parents in Level $i-1$. They are *same-level peers*, not tree siblings. Lemma 3.3 ("child is adjacent to parent's right sibling") strictly requires vertices to share the same parent and cannot be applied to arbitrary peers.
     - *Inapplicability of Lemma 3.6*: Lemma 3.6 states that if $u, v \in L_d$ and $uv \in E$, then every child of $u$ is adjacent to every child of $v$. While $p = \text{parent}(y) \in L_i$ and $u^*_i \in L_i$ are adjacent by Theorem 3.7, Lemma 3.6 establishes an edge between *two children* at Level $i+1$ (child-to-child), NOT between a parent-level node $u^*_i \in L_i$ and a child $y \in L_{i+1}$ (node-to-child). Furthermore, if $u^*_i$ is a leaf, it has no children. Hence, Lemma 3.6 does not establish node-to-child adjacency.
     - *Analytical Gap in the Thesis*: In the proof of Theorem 3.17 (page 47), the thesis asserts without proof or citation: *"The selected vertex dominates the vertices in levels i and i+1."* The thesis leaves a gap by not providing a lemma connecting a same-level rightmost peer to all children in the next level.
     - *Formal Geometric Derivation via Common Endpoint $r_p$*:
       Let $y \in L_{i+1}$ with parent $p = \text{parent}(y) \in L_i$. Let $u^*_i$ be the rightmost vertex of Level $i$.
       We prove that the point $r_p$ belongs to both intervals $I_{u^*_i} = [\ell_{u^*_i}, r_{u^*_i}]$ and $I_y = [\ell_y, r_y]$, establishing $r_p \in I_{u^*_i} \cap I_y$ and hence $u^*_i y \in E$:
       1. **Chain 1: $\ell_{u^*_i} \le r_p \le r_{u^*_i}$ (proving $r_p \in I_{u^*_i}$)**:
          - *Inequality $\ell_{u^*_i} \le r_p$*: Both $p$ and $u^*_i$ belong to Level $i$. By Theorem 3.7, every depth level induces a clique, so $p u^*_i \in E$. Since $u^*_i$ occurs to the right of $p$ in the canonical ordering ($p \le u^*_i$), interval intersection $I_p \cap I_{u^*_i} \neq \emptyset$ requires that the left endpoint of the right interval cannot exceed the right endpoint of the left interval: $\ell_{u^*_i} \le r_p$.
          - *Inequality $r_p \le r_{u^*_i}$*: By definition, $u^*_i$ is the rightmost vertex in Level $i$. Thus $p \le u^*_i$ in the canonical ordering. By the monotonic right-endpoint property of proper interval graphs (Section 2.5), $p \le u^*_i \implies r_p \le r_{u^*_i}$.
          - Combining both: $\ell_{u^*_i} \le r_p \le r_{u^*_i}$, so $r_p \in I_{u^*_i}$.
       2. **Chain 2: $\ell_y \le r_p \le r_y$ (proving $r_p \in I_y$)**:
          - *Inequality $\ell_p \le \ell_y$*: By the distance-tree parent definition (Equation 2.1), $p = \arg\min_{u \in V} \{\ell_u : r_u \ge \ell_y\}$. Since $y$ itself trivially satisfies $r_y \ge \ell_y$, $y$ is a valid candidate in its own parent candidate set. By minimality of $p$, $\ell_p \le \ell_y$.
          - *Inequality $r_p \le r_y$*: By the properness condition (Section 2.3, no proper containment), $\ell_p \le \ell_y \implies r_p \le r_y$. Otherwise, if $r_p > r_y$, then $I_y = [\ell_y, r_y]$ would be properly contained in $I_p = [\ell_p, r_p]$. In the boundary case where $\ell_p = \ell_y$, properness also permits identical intervals, for which the right endpoints are equal ($r_p = r_y$). Thus, $r_p \le r_y$ holds in all cases.
          - *Inequality $\ell_y \le r_p$*: By the parent candidate condition, $p$ must satisfy $r_p \ge \ell_y$, which is $\ell_y \le r_p$.
          - Combining these: $\ell_y \le r_p \le r_y$, which establishes that $r_p \in I_y$ directly without requiring strict inequalities or excluding identical intervals.
       3. **Synthesis**:
          Since $\ell_{u^*_i} \le r_p \le r_{u^*_i}$ and $\ell_y \le r_p \le r_y$, we have:
          $$\max(\ell_{u^*_i}, \ell_y) \le r_p \le \min(r_{u^*_i}, r_y)$$
          Therefore, $r_p \in I_{u^*_i} \cap I_y \neq \emptyset$, proving that $u^*_i$ and $y$ intersect ($u^*_i y \in E$) in $G$.
          This proof relies entirely on non-strict inequalities derived directly from Equation (2.1), Section 2.3 (properness), Section 2.5 (endpoint ordering), and Theorem 3.7 (level cliques).
3. **Cardinality & Bound Match**:
   - Total vertices selected: $1 + \lceil \frac{h-3}{2} \rceil = \lceil \frac{h-1}{2} \rceil$.
   - When $h$ is odd, $\lceil \frac{h-1}{2} \rceil = \frac{h-1}{2}$.

### 3. Boundary Cases Analysis
1. **$h = 1$ (Single-Vertex Graph)**:
   - What the thesis literally states: $\gamma(G) \le \lceil (1-1)/2 \rceil = 0$.
   - Mathematical Reality: Any non-empty graph ($n \ge 1$) has $\gamma(G) \ge 1$. A dominating set of size 0 is impossible.
   - Distance Tree Reality: Level 2 does not exist when $h=1$; only root level (depth 0) exists.
   - Implementation Handling: $h=1$ is explicitly detected as a boundary case (`isBoundaryCaseH1 = true`). The algorithm selects the single root vertex $v_1$ ($|D| = 1$), which gives 100% domination. The theoretical bound is documented as 1, while recording the raw formula output (0) as degenerate.
2. **$h = 2$ (Two-Level Graph)**:
   - What the thesis literally states: $\gamma(G) \le \lceil (2-1)/2 \rceil = 1$.
   - Construction: Level 2 exists. The rightmost vertex of Level 2 is selected.
   - Domination: As proved above, the rightmost vertex of Level 2 is adjacent to root $v_1$ (Level 1) and all other vertices in Level 2 (clique). Level 3 is empty.
   - Result: $\{ u^*_2 \}$ dominates all vertices in $G$. Size $|D| = 1 \le \lceil (2-1)/2 \rceil = 1$. Both the construction and the bound hold with zero exceptions.

### 4. Computational Complexity
- **Given Distance Tree Level Sets**: The construction inspects only the back element of Level 2 and Levels $4, 6, 8, \dots \le h$.
- **Time Complexity**: $O(h)$ operations, where $h$ is the number of levels ($h \le n$).
- **Space Complexity**: $O(h)$ auxiliary space to store the selected dominating set.
- **Independent Verification Time**: $O(n \cdot |D|) \le O(nh) \le O(n^2)$ time using raw interval intersection tests.

### 5. Verification Results
- **Handcrafted Edge Cases**:
  - Single-vertex graph ($n=1, h=1$): $|D|=1$, independent check PASS.
  - Two-vertex connected graph ($n=2, h=2$): $|D|=1$, independent check PASS.
  - Thesis 8-vertex example ($n=8, h=4$): $|D|=2$ (vertices $\{4, 8\}$), matches true optimal $\gamma(G)=2$, independent check PASS.
  - Complete graph ($n=5, h=2$): $|D|=1$, independent check PASS.
  - Path-like graph ($n=6, h=6$): $|D|=3$, independent check PASS.
  - Odd-height graph ($n=5, h=5$): $|D|=2$, independent check PASS.
  - Multiple branches per level ($n=8, h=4$): $|D|=2$, independent check PASS.
  - Endpoint-touching graph ($n=4, h=3$): $|D|=1$, independent check PASS.
- **Random Graphs Verification Suite**:
  - 50 reproducible random connected proper interval graphs ($n = 3 \dots 20$).
  - Evaluated across 28 odd-height and 22 even-height graphs.
  - Domination Coverage: 100% PASS (0 uncovered vertices across all instances).
  - Theorem 3.17 Bound Satisfied: 100% PASS ($|D| \le \lceil (h-1)/2 \rceil$ for all $h \ge 2$).
  - Ground Truth Comparison: Compared against independent exponential Brute-Force Minimum Dominating Set (MDS) solver on all small graphs: $|D| \ge \gamma(G)$ always held, with $|D| = \gamma(G)$ achieved in over 85% of instances.

---

## Detailed Phase 8 Validation: Incremental Maintenance of MIS (Theorem 3.14)

### 1. Theorem Statement & Reference
- **Thesis Reference**: Chapter 3, Section 3.10.2, pages 44–45 (numbered pages 51–52 of thesis PDF).
- **Exact Formulation**:
  > **Theorem 3.14**: *Under a leaf insertion into a proper interval graph $G$, the maximum independent set size $\alpha(G)$ and the dynamic programming values can be updated in $O(\log n)$ worst-case time, assuming the parent of the new vertex in the distance tree can be identified in $O(\log n)$ time.*

### 2. Exact Mathematical Recurrence & State Update
Let $u$ be a newly inserted vertex into $G$, and let $p = \text{parent}(u)$ be its parent in $T(G')$, where $p \in L_{i-1}$ ($i = \text{level}(u) = \text{level}(p) + 1$, depth $i-1$).
1. **DP Recurrence**:
   $$DP[i][u] = 1 + \max \left( BEST[i-2], \max_{\substack{v \in L_{i-1} \\ v < p}} DP[i-1][v] \right)$$
   where:
   - $BEST[i-2] = \max_{1 \le k \le i-2} \max_{w \in L_k} DP[k][w]$ (with $BEST[k] = 0$ for $k \le 0$).
   - $\max_{v \in L_{i-1}, v < p} DP[i-1][v]$ is the maximum DP value among all vertices strictly to the left of $p$ in Level $i-1$ in canonical order.
2. **Global MIS Maintenance**:
   $$\alpha(G') \leftarrow \max(\alpha(G), DP[i][u])$$
   $$BEST[i] \leftarrow \max(BEST[i], DP[i][u])$$

### 3. Leaf-Insertion Model & Validation Criteria
The incremental update of Theorem 3.14 strictly applies when the newly inserted vertex is a **leaf** in the updated distance tree $T(G')$.
To preserve mathematical exactness of the single-point DP recurrence, our implementation enforces the **Rightmost Append-Only Leaf Insertion Model**:
1. **Properness Invariant**: The new interval $I_u$ does not properly contain nor is properly contained in any existing interval $I_v \in V$ ($I_u \not\subset I_v \land I_v \not\subset I_u$).
2. **Connectedness Invariant**: $I_u$ intersects at least one existing interval in $G$ ($\exists v \in V: I_u \cap I_v \neq \emptyset$).
3. **Parent Existence**: At least one candidate $v \in V$ satisfies $r_v \ge \ell_u$. The parent $p = \arg\min_{v: r_v \ge \ell_u} \ell_v$ is identified.
4. **Leaf Condition & Tree Invariance**:
   - The new vertex must have **zero children** in $T(G')$.
   - Crucially, no existing vertex $w \in V$ can adopt $u$ as its new parent. That is, for every existing vertex $w \in V$:
     $$\text{if } r_u \ge \ell_w, \text{ then } (\ell_p, r_p, p) \le (\ell_u, r_u, u) \text{ where } p = \text{parent}(w)$$
     If any existing vertex $w$ would adopt $u$, the tree structure would be modified, violating the leaf-insertion model, and the insertion is rejected.
5. **Rightmost Append-Only Level Ordering**:
   - If $u$ is inserted into an existing level $L_i$ ($i \le h$), every existing vertex $v \in L_i$ must strictly precede $u$ in canonical order ($v < u$).
   - If $u$ creates a new deepest level $L_{h+1}$, it is automatically rightmost in that new level.
   - Any middle-of-level or leftmost insertion is rejected, with a clear diagnostic explaining that inserting before existing vertices in the same level invalidates DP values of existing descendants.

### 4. Implementation Data Structures & Complexity Distinctions

#### The Thesis Distinction:
Theorem 3.14 in the thesis states the result for leaf insertions without explicitly stating the additional rightmost-order restriction.
We do not rewrite the thesis theorem as though it explicitly stated this condition. Instead, we document rightmost append-only ordering as an **additional restriction imposed by our implementation** to ensure that the single-point DP update remains exact without requiring cascading DP invalidation of existing descendants.

#### The Adversarial Counterexample:
In `PHASE8_FINAL_AUDIT.md`, we constructed a minimal 4-vertex base graph:
$v_1=[0,4]$ (L1), $v_2=[2,6]$ (L2), $v_3=[5,9]$ (L3, parent $v_2$), $v_4=[8,12]$ (L4, parent $v_3$), with $\alpha(G) = 2$.
Inserting leaf $u=v_5=[4.5, 7.0]$ into Level 3 to the left of $v_3$:
- $v_5$ is a valid leaf in $T(G')$ (parent $v_2$, 0 children, no parent changes).
- However, $v_5$ increases the prefix maximum of $v_3$, which cascades to child $v_4$, increasing $DP[4][v_4]$ from 2 to 3.
- True maximum independent set becomes $\{v_1, v_5, v_4\}$ with size 3.
- Theorem 3.14 evaluated only $DP[3][v_5] = 2$ and output $\alpha = \max(2, 2) = 2$, failing to match the true optimum.
- Under our rightmost append-only restriction, this adversarial middle insertion is cleanly and deterministically **REJECTED**, preserving 100% exactness on all accepted insertions.

#### Complexity Distinction:
We clearly distinguish:
1. **Theorem 3.14 Conditional Update Complexity**:
   Assuming parent identification takes $O(\log n)$ and the input satisfies the supported leaf-insertion model, the mathematical state update takes $O(\log n)$ worst-case time.
2. **Actual Implementation Complexity**:
   - **Internal State Update Time**: The balanced tree operations (`LevelAVLTree`, `ParentAVLTree`) and DP recurrence take strictly **worst-case $O(\log n)$ time**.
   - **End-to-End Insertion Time**: Because `validateLeafInsertion()` performs pairwise checks over all existing vertices to verify properness, connectedness, and leaf invariants, the public end-to-end insertion operation takes **$O(n)$ worst-case time**.
   - The current single-point update fails on unrestricted middle insertions, and cascading DP invalidation can affect multiple existing vertices across deeper levels.

| Operation | Thesis Assumption | Implementation Method | Actual Complexity |
| :--- | :--- | :--- | :---: |
| Parent Lookup | $O(\log n)$ assumed | `ParentAVLTree::findParent` (Augmented AVL) | $O(\log n)$ |
| Prefix Maxima Query | $O(1)$ claimed | `LevelAVLTree::queryPrefixMaxStrictlyLeft` | $O(\log n)$ |
| DP Value Calculation | $O(1)$ | $1 + \max(BEST[i-2], \text{prefixMax})$ | $O(1)$ |
| Level Tree Insertion | Unspecified | `LevelAVLTree::insert` (Augmented AVL) | $O(\log n)$ |
| Parent Tree Insertion| Unspecified | `ParentAVLTree::insert` (Augmented AVL) | $O(\log n)$ |
| Global $\alpha(G)$ Update | $O(1)$ | $\max(\alpha(G), DP[i][u])$ | $O(1)$ |
| **Internal State Update** | **$O(\log n)$** | **Augmented AVL Pipeline** | **$O(\log n)$** |
| Precondition Validation | External Precondition | `validateLeafInsertion` | **$O(n)$** |
| **End-to-End Operation**| **Unspecified** | **Full Validated Insertion** | **$O(n)$** |

### 5. Verification Results (19 Dedicated Test Suites)
1. **Single-Vertex Base Graph ($n=1 \to 2$)**: Leaf inserted at Level 2, parent is root, $\alpha$ remains 1. PASS.
2. **Multiple Leaves at Level 2**: Two rightmost leaves added at Level 2, tree height 2, $\alpha$ remains 1. PASS.
3. **Deeper Level Leaf Insertion**: Leaf added at Level 3, increases $\alpha$ from 1 to 2. PASS.
4. **Parent is NOT Rightmost**: Handcrafted graph where parent $p=2$ is not the rightmost vertex in Level 2 ($u^*_2 = 3$). Verified parent selection and properness ($r_4 \ge r_3$). PASS.
5. **Parent IS Rightmost**: Leaf reaching $r_3 = 7.0$ with parent $p=3$. PASS.
6. **Multiple Sequential Leaves**: 5 consecutive rightmost leaf insertions, extending tree to height 6 and $\alpha=3$. PASS.
7. **Insertion Increasing Alpha**: Dedicated check verifying `alphaIncreased == true` and $\alpha$ increments by 1. PASS.
8. **Insertion Keeping Alpha Unchanged**: Dedicated check verifying $\alpha$ remains constant. PASS.
9. **Endpoint-Touching Leaf**: Touching endpoints $\ell_u = r_p$ handled cleanly without boundary degradation. PASS.
10. **Identical Intervals**: Inserting an identical interval at Level 2 with ID tie-breaking (precedes in canonical order). PASS.
11. **Rejection: Properness Violation**: Interval $[1.0, 4.0]$ inside $[0.0, 5.0]$ properly rejected. PASS.
12. **Rejection: Connectedness Violation**: Disconnected interval $[12.0, 15.0]$ with gap properly rejected. PASS.
13. **Rejection: Non-Leaf Insertion**: Interval $[5.0, 9.0]$ where an existing vertex would adopt the new vertex as parent properly rejected. PASS.
14. **Rejection: Middle-of-Level Insertion**: Inserting vertex 3 $[2.0, 7.0]$ between vertices 2 and 4 in Level 2 is correctly rejected under the rightmost append-only model. PASS.
15. **Randomized Stress Sequences**: 30 independent sequences of 6 leaf insertions each (180 dynamic leaf insertions total):
    - Incremental $\alpha(G)$ vs Static Phase 6 DP: **100% MATCH** across all 180 insertions.
    - Incremental $\alpha(G)$ vs Independent Brute-Force Oracle: **100% MATCH** across all 180 insertions.
    - Tree invariants, parent links, and leaf properties: **100% PASS**.
16. **Rejection: Leftmost Insertion**: Inserting before all existing vertices in a level is rejected. PASS.
17. **Rejection: Exact Adversarial Counterexample**: The 5-vertex adversarial case from `PHASE8_FINAL_AUDIT.md` is cleanly rejected. PASS.
18. **Acceptance: Deepest Level Extension**: Inserting a leaf that creates a new deepest level $L_{h+1}$ is accepted and verified. PASS.
19. **Acceptance: Existing Level Rightmost**: Inserting a leaf at the rightmost position of an existing level is accepted and verified. PASS.

---

## Detailed Phase 9 Validation: Decremental Maintenance of the Distance Tree (Theorem 3.16)

### 1. Theorem Statement & Scope
- **Thesis Reference**: Chapter 3, Section 3.10.3, Observation 3.10.3 & Theorem 3.16, pages 44–46 (PDF pages 51–54).
- **Exact Formulation**:
  *Consider the modified distance-tree representation in which the children of each vertex are maintained as an ordered linked set with only its boundary links explicitly maintained. Suppose that the parent of a deleted vertex can be found in $\mathcal{O}(\log n)$ time. Then the deletion of a vertex can be processed in $\mathcal{O}(\log n)$ time.*
- **Scope**: Decremental distance-tree maintenance under vertex deletions. Graph Burning (Section 3.11) is strictly excluded.

### 2. Algorithmic Realization: Linked Child-Set Model
1. **Relocation Rule (Observation 3.10.3)**:
   When $u \in L_i$ is deleted, its children are relocated under the immediate right vertex $v \in L_i$.
   Because all children of $u$ strictly precede all children of $v$ in canonical left-endpoint order ($\max_{c \in C(u)} \ell_c < \min_{c' \in C(v)} \ell_{c'}$), joining $C(u)$ and $C(v)$ is a direct concatenation requiring only $\mathcal{O}(1)$ boundary link updates:
   - $u_{\text{tail}} \to \text{nextSibling} = v_{\text{head}}$
   - $v_{\text{head}} \to \text{prevSibling} = u_{\text{tail}}$
   - $v \to \text{childHead} = u_{\text{head}}$
2. **Parent Boundary Link Updates**:
   If $u$ is the rightmost child of its parent $p$, $p \to \text{childTail} = u \to \text{prevSibling}$ in $\mathcal{O}(1)$.
3. **Level Splice**:
   $u$ is unlinked from $L_i$'s doubly linked list in $\mathcal{O}(1)$.
4. **Parent Search**:
   Parent lookup in $\mathcal{O}(\log n)$ via `ParentAVLTree` from Phase 8.

### 3. Complexity Separation
- **Internal Structural Deletion (Theorem 3.16)**: $\mathcal{O}(\log n)$ worst-case.
- **End-to-End Public Deletion with Validation**: $\mathcal{O}(n)$ worst-case (connectedness sweep).
- **Logical Tree Reconstruction**: $\mathcal{O}(n)$ worst-case.
- **Static MIS Recomputation**: $\mathcal{O}(n)$ worst-case.

### 4. Verification Results (17 Dedicated Unit Suites + 240 Sequential Random Deletions)
1. `testDecTree_1_LeafWithImmediateRight`: Delete leaf with right sibling. (PASS)
2. `testDecTree_2_NonLeafWithChildrenAndImmediateRight`: Delete vertex with child, relocated to right sibling. (PASS)
3. `testDecTree_3_JoinChildSets`: Delete vertex whose right sibling already has children; verifies correct concatenation. (PASS)
4. `testDecTree_4_RightmostChildMetadata`: Delete vertex that is rightmost child; verifies parent's `childTail` update. (PASS)
5. `testDecTree_5_EmptyChildSet`: Delete middle-of-level leaf. (PASS)
6. `testDecTree_6_EmptyChildSetRightmostOfLevel`: Delete rightmost leaf of deepest level; verifies tree height reduction. (PASS)
7. `testDecTree_7_RejectChildrenWithoutImmediateRight`: Reject deletion of vertex with children but no right sibling. (PASS)
8. `testDecTree_8_RejectRootDeletion`: Reject root deletion with informative diagnostic. (PASS)
9. `testDecTree_9_RejectDisconnectingDeletion`: Reject deletion that disconnects the graph. (PASS)
10. `testDecTree_10_RejectEmptyGraph`: Reject deletion leaving empty graph. (PASS)
11. `testDecTree_11_PropernessCheck`: Validate properness preservation. (PASS)
12. `testDecTree_12_RepeatedSequentialDeletions`: 4 consecutive deletions, verified at every step. (PASS)
13. `testDecTree_13_EndpointTouching`: Deletion in graphs with touching interval endpoints. (PASS)
14. `testDecTree_14_IdenticalIntervals`: Deletion with identical intervals disambiguated by ID. (PASS)
15. `testDecTree_15_AlphaChanged`: Deletion that strictly reduces $\alpha(G)$ by 1. (PASS)
16. `testDecTree_16_AlphaUnchanged`: Deletion that leaves $\alpha(G)$ invariant. (PASS)
17. `testDecTree_17_GeneratedSequences`: 30 reproducible random graphs tested across 240 deletions:
    - 240 / 240 accepted deletions.
    - 240 / 240 static tree rebuild comparisons (**100% exact match** on parents, depths, children, and levels).
    - 240 / 240 MIS comparisons (**100% exact match** with static DP and brute force).
    - **0 mismatches**. (PASS)



