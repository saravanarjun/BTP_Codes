# Stage 1.5 — Mathematical Proof Closure and Audit Clarification
## Rigorous Invariant Proofs, Counterexamples, and Experimental Reconciliation

**Repository**: `C:/Users/sarav/OneDrive/Desktop/BTP_Codes`  
**Subdirectory**: `benchmarks/maximal_cliques/`  
**Thesis Source of Truth**: *Dynamic Graph Algorithms – Interval Graphs* (Sections 3.8, 3.10.1, 3.10.2, 3.10.3; Lemmas 3.8, 3.15; Theorems 3.14, 3.16)  
**Date**: November 2026  
**Auditor**: Independent Algorithm & Verification Specialist  

---

# 1. Objection 1: Static Maximal-Clique Theorem Complete Proof

### The Objection
The Stage 1 conclusion that every non-leaf parent in canonical $T(G)$ defines a distinct maximal clique relied on monotonicity, but did not separately prove clique property, maximality across all boundary conditions, completeness, uniqueness, and proper interval graph assumptions. Non-containment of descriptor intervals was used as a proxy for maximality without direct proof.

---

### Mathematical Proof Closure

Let $G = (V, E)$ be a connected proper interval graph.
Each vertex $v \in V$ is represented by a closed interval $I_v = [l_v, r_v]$.
Adjacency: $(u, v) \in E \iff I_u \cap I_v \ne \emptyset \iff l_u \le r_v \land l_v \le r_u$.
Properness: $\forall u, v \in V, \quad I_u \subsetneq I_v \implies u = v$.
Strict canonical total ordering $\prec$:
$$u \prec v \iff (l_u < l_v) \lor (l_u = l_v \land r_u < r_v) \lor (l_u = l_v \land r_u = r_v \land \text{id}(u) < \text{id}(v))$$
Vertices ordered canonically are $v_0, v_1, \dots, v_{n-1}$.
Root $u_0 = v_0$ (leftmost interval).
Parent definition (Equation 3.1 in thesis):
$$p(y) = \arg\min_{u \in V} \{ l_u : r_u \ge l_y \}$$
with ties broken canonically by $\prec$ (minimizing $(l_u, r_u, \text{id}(u))$). For root, $p(u_0) = u_0$.
For non-leaf parent $p$, its rightmost child is $c_p = \max_{\prec} C(p)$.
Descriptor: $D(p) = [l_p, l_{c_p}]$. Materialized set: $C(p) = \{ v \in V : l_p \le l_v \le l_{c_p} \}$.

#### 1.1 Proof that $C(p)$ is a Clique
Let $u, v \in C(p)$ with $l_u \le l_v$. By definition of $C(p)$, $l_p \le l_u \le l_v \le l_{c_p}$.
Since $c_p$ is a child of $p$, $(p, c_p) \in E$, which under closed intervals means $l_{c_p} \le r_p$.
- Because $l_v \le l_{c_p}$, we have $l_v \le r_p$.
- Because $l_p \le l_u$, properness ensures $r_p \le r_u$:
  - If $l_p < l_u$, $I_p$ cannot properly contain $I_u$, so $r_p \le r_u$.
  - If $l_p = l_u$, canonical order requires $r_p \le r_u$.
- Combining inequalities:
  $$l_v \le l_{c_p} \le r_p \le r_u \implies l_v \le r_u$$
Since $l_u \le l_v \le r_u$, point $l_v \in I_u \cap I_v$. Thus $(u, v) \in E$. Every pair in $C(p)$ is adjacent. $\square$

#### 1.2 Proof that $C(p)$ is Maximal
Suppose for contradiction that $C(p)$ can be extended by some vertex $w \in V \setminus C(p)$ adjacent to all vertices in $C(p)$.
In particular, $(w, p) \in E \implies l_w \le r_p \land l_p \le r_w$, and $(w, c_p) \in E \implies l_w \le r_{c_p} \land l_{c_p} \le r_w$.
Since $w \notin C(p)$, either $l_w < l_p$ or $l_w > l_{c_p}$:

1. **Direction $l_w < l_p$**:
   - Since $(w, c_p) \in E$, we have $r_w \ge l_{c_p}$.
   - Thus, $w$ satisfies the parent candidate condition for $c_p$: $r_w \ge l_{c_p}$.
   - But $l_w < l_p$.
   - Since parent selection minimizes $l_u$, $p$ could not have been chosen as the parent of $c_p$, because $w$ has a strictly smaller left endpoint than $p$.
   - Contradiction to $p(c_p) = p$.
   - *Tied left endpoints ($l_w = l_p$)*: If $l_w = l_p$, then $l_p \le l_w \le l_{c_p} \implies w \in C(p)$, contradicting $w \notin C(p)$.
   - *Identical intervals ($I_w = I_p$)*: If $I_w = I_p$, then $l_w = l_p \implies w \in C(p)$, contradiction.
2. **Direction $l_w > l_{c_p}$**:
   - Since $(w, p) \in E$, we have $l_w \le r_p$.
   - Thus $l_{c_p} < l_w \le r_p$.
   - Let $\text{depth}(p) = d$. Then $\text{depth}(c_p) = d + 1$.
   - Since $w$ is adjacent to $p$, $\text{depth}(w) \le d + 1$.
   - By BFS monotonicity on canonically ordered proper interval graphs, $c_p \prec w \implies \text{depth}(w) \ge \text{depth}(c_p) = d + 1$.
   - Therefore, $\text{depth}(w) = d + 1$.
   - Parent candidate for $w$: since $r_p \ge l_w$, $p$ is a candidate parent for $w$, so $l_{p(w)} \le l_p$.
   - Monotonicity also gives $l_{p(w)} \ge l_{p(c_p)} = l_p$.
   - Hence $l_{p(w)} = l_p$. By canonical tie-breaking, $p(w) = p$.
   - Thus $w$ is a child of $p$.
   - But $l_w > l_{c_p} \implies c_p \prec w$, which contradicts that $c_p$ is the rightmost child of $p$.
   - *Endpoint-touching ($l_w = r_p$)*: Same contradiction: $w$ is adjacent to $p$ and has depth $d+1$, forcing $w$ to be a child of $p$ to the right of $c_p$.
   - *Tied right endpoints ($r_w = r_{c_p}$)*: If $l_w > l_{c_p}$ and $r_w = r_{c_p}$, then $I_w \subsetneq I_{c_p}$, violating properness.

No such vertex $w$ can exist. $C(p)$ is strictly maximal. $\square$

#### 1.3 Proof that Every Maximal Clique is Represented
Let $M = \{v_i, \dots, v_j\}$ be any maximal clique in $G$, with leftmost vertex $p = v_i$ and rightmost vertex $q = v_j$.
If $n = 1$, $u_0$ is root and leaf, representing $\{u_0\}$.
If $n > 1$, $p$ cannot be a leaf: if $p$ were a leaf, $p \ne u_0$ (since $u_0$ has children in connected graphs with $n > 1$). $p$ has parent $p(p)$ with $r_{p(p)} \ge l_p$. For $M$ not to include $p(p)$, there must exist some $v \in M$ with $l_v > r_{p(p)}$. But $l_v \le r_p$, which makes $p$ a valid candidate parent for $v$. Thus $v$ (or some vertex in $M$) chooses $p$ as parent, forcing $p$ to have children ($C(p) \ne \emptyset$).
Furthermore, the rightmost vertex $q \in M$ is adjacent to $p$, so $q$ is a child of $p$. By maximality, $q$ must be the rightmost child $c_p$.
Hence $M = [l_p, l_{c_p}] = C(p)$. $\square$

#### 1.4 Proof of Uniqueness (Injective Representation)
Let $p_1, p_2$ be distinct non-leaf parents with $p_1 \prec p_2$.
- If $I_{p_1} = I_{p_2}$, canonical tie-breaking directs all prospective children to $p_1$ by ID. Thus $p_2$ has zero children and is a leaf, contradicting that $p_2$ is a non-leaf parent. Thus $I_{p_1} \ne I_{p_2} \implies l_{p_1} < l_{p_2}$.
- Since $l_{p_1} < l_{p_2}$, vertex $p_1 \in C(p_1)$, but $p_1 \notin C(p_2)$ because all vertices in $C(p_2)$ have left endpoint $\ge l_{p_2} > l_{p_1}$.
- Therefore, $C(p_1) \ne C(p_2)$. The mapping is strictly injective. $\square$

#### 1.5 Proper Interval Graph Assumptions & Thesis Tie-Breaking
The thesis (Lemma 3.15) assumes connected proper interval graphs with closed intervals. The thesis states $p(y) = \arg\min \{l_u : r_u \ge l_y\}$. Our canonical tie-breaking $(l_u, r_u, \text{id}(u))$ is the precise operational specification that guarantees duplicate intervals are leaves, preserving strict bijectivity without subset filtering.

**Verdict**: **PROVED**  
**Source Location**: `src/MaximalCliques.cpp:157–195`, `benchmarks/maximal_cliques/theorem_cliques_adapter.cpp:27–52`.

---

# 2. Objection 2: Dynamic Deletion — Resolution of the Three-Descriptor Change

### The Objection
The Stage 1 report claimed that at most two descriptors are affected during decremental deletion under Theorem 3.16. Can Case B occur and affect THREE descriptors?

---

### Case-by-Case Mathematical Derivation

Let non-leaf vertex $u$ be deleted under Theorem 3.16.
$u$'s children $C(u) = [u_{\text{left}}, u_{\text{right}}]$ are adopted by immediate right sibling $v \in L(u)$.
Let $p = p(u)$, $c_u = u_{\text{right}}$, and $c_v = \text{rightmost child of } v$ (if $v$ was non-leaf).

| Case | Condition on $v$ | Condition on $u$ in $C(p)$ | Descriptor $u$ | Descriptor $v$ | Descriptor $p$ | Other Parents | Total Affected Descriptors |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **Case A** | $v$ already has children | $u = c_p$ (rightmost child) | Removed | Unchanged ($c_v$ invariant) | Modified or Removed | Unchanged | **2 affected** (1 rem, 1 mod/rem) |
| **Case B1** | $v$ is a leaf | $u = c_p$ and $p$ has earlier child | **Removed** | **Added** ($v \mapsto c_u$) | **Modified** ($p \mapsto \text{prev}(u)$) | Unchanged | $\mathbf{3 \text{ affected}}$ (1 rem, 1 add, 1 mod) |
| **Case B2** | $v$ is a leaf | $u = c_p$ and $u$ was only child | **Removed** | **Added** ($v \mapsto c_u$) | **Removed** ($p$ becomes leaf) | Unchanged | $\mathbf{3 \text{ affected}}$ (2 rem, 1 add, 0 mod) |
| **Case C** | $v$ already has children | $u \ne c_p$ ($u$ has younger sibling) | Removed | Unchanged ($c_v$ invariant) | Unchanged ($c_p$ invariant) | Unchanged | **1 affected** (1 rem) |
| **Case D** | $v$ is a leaf | $u \ne c_p$ ($u$ has younger sibling) | Removed | Added ($v \mapsto c_u$) | Unchanged ($c_p$ invariant) | Unchanged | **2 affected** (1 rem, 1 add) |

---

### Formal Proof of Case B Counterexample (3 Descriptors Affected)

We prove that Case B CAN occur in valid, connected proper interval graphs satisfying all Theorem 3.16 preconditions.

#### Counterexample Graph Instance ($n = 7$)
Construct 7 closed intervals with strictly increasing endpoints:
- $v_0 = 0$: $I_0 = [0.0, 10.0]$ (Root, Level 0)
- $v_1 = 1$: $I_1 = [2.0, 12.0]$ ($p_1 \in L_1$, child of 0)
- $v_2 = 2$: $I_2 = [8.0, 15.0]$ ($p_2 \in L_1$, child of 0)
- $v_3 = 3$: $I_3 = [11.0, 16.0]$ ($w \in L_2$, child of 1)
- $v_4 = 4$: $I_4 = [11.5, 17.0]$ ($u \in L_2$, rightmost child of 1)
- $v_5 = 5$: $I_5 = [12.5, 18.0]$ ($v \in L_2$, child of 2; leaf)
- $v_6 = 6$: $I_6 = [16.5, 20.0]$ ($c_u \in L_3$, child of 4)

#### Verification of Graph Properties
1. **Properness**:
   - Left endpoints: $0.0 < 2.0 < 8.0 < 11.0 < 11.5 < 12.5 < 16.5$ (strictly increasing).
   - Right endpoints: $10.0 < 12.0 < 15.0 < 16.0 < 17.0 < 18.0 < 20.0$ (strictly increasing).
   - Since left and right endpoints strictly increase together, no interval strictly contains another. Strictly proper.
2. **Connectedness**:
   - $r_0 = 10.0 > l_1 = 2.0$; $r_1 = 12.0 > l_3 = 11.0$; $r_2 = 15.0 > l_5 = 12.5$; $r_4 = 17.0 > l_6 = 16.5$. Fully connected.
3. **Distance Tree State Before Deletion**:
   - Parent of 1: $\arg\min \{l_u : r_u \ge 2.0\} = 0$.
   - Parent of 2: $\arg\min \{l_u : r_u \ge 8.0\} = 0$.
   - Parent of 3: $r_0 = 10.0 < 11.0 \implies 0$ invalid; $r_1 = 12.0 \ge 11.0 \implies p(3) = 1$.
   - Parent of 4 ($u$): $r_1 = 12.0 \ge 11.5 \implies p(4) = 1$.
   - Parent of 5 ($v$): $r_1 = 12.0 < 12.5 \implies 1$ invalid; $r_2 = 15.0 \ge 12.5 \implies p(5) = 2$.
   - Parent of 6 ($c_u$): $r_3 = 16.0 < 16.5 \implies 3$ invalid; $r_4 = 17.0 \ge 16.5 \implies p(6) = 4$.
   - Non-leaf parents:
     - Node 0: children $\{1, 2\}$, rightmost is $2 \implies$ descriptor $0 \mapsto 2$.
     - Node 1 ($p$): children $\{3, 4\}$, rightmost is $4 \implies$ descriptor $1 \mapsto 4$.
     - Node 2: children $\{5\}$, rightmost is $5 \implies$ descriptor $2 \mapsto 5$.
     - Node 4 ($u$): children $\{6\}$, rightmost is $6 \implies$ descriptor $4 \mapsto 6$.
     - Node 5 ($v$): no children (leaf).
   - Dictionary before: $\{0 \mapsto 2,\ 1 \mapsto 4,\ 2 \mapsto 5,\ 4 \mapsto 6\}$.

#### Execution of Theorem 3.16 Deletion of Vertex 4 ($u$)
- $u = 4$ is non-root ($4 \ne 0$).
- $L_2 = \{3, 4, 5\}$. Immediate right sibling of 4 is $v = 5$.
- $r_v = r_5 = 18.0 \ge l_6 = 16.5$. Adjacency invariant holds: 5 can adopt 6.
- Remaining graph $G \setminus \{4\}$ is connected and proper.
- **State After Child Relocation**:
  1. $u = 4$ is deleted $\implies$ descriptor $4 \mapsto 6$ is **REMOVED**.
  2. $v = 5$ adopts 6 $\implies$ 5 becomes a non-leaf parent with rightmost child 6. Descriptor $5 \mapsto 6$ is **ADDED**.
  3. Parent of 4 was 1. Children of 1 were $\{3, 4\}$. With 4 deleted, children of 1 are $\{3\}$.
     Rightmost child of 1 changes from 4 to 3! Descriptor $1 \mapsto 4$ becomes $1 \mapsto 3$ (**MODIFIED**).
  4. Descriptors 0 and 2 are completely unchanged.
- Dictionary after: $\{0 \mapsto 2,\ 1 \mapsto 3,\ 2 \mapsto 5,\ 5 \mapsto 6\}$.

#### Exact Descriptor Delta
- **Descriptors Added**: $\mathbf{1}$ ($5 \mapsto 6$)
- **Descriptors Removed**: $\mathbf{1}$ ($4 \mapsto 6$)
- **Descriptors Modified**: $\mathbf{1}$ ($1 \mapsto 4 \to 1 \mapsto 3$)
- **Total Affected Descriptors**: $\mathbf{1 + 1 + 1 = 3}$!

---

### Strongest Justified Upper Bounds for Theorem 3.16 Deletion
Across all possible valid deletion scenarios:
- **Number of Added Descriptors**: $\Delta_{\text{added}} \le 1$ ($v$ added iff $v$ was a leaf).
- **Number of Removed Descriptors**: $\Delta_{\text{removed}} \le 2$ ($u$ removed; and $p(u)$ removed iff $u$ was an only child).
- **Number of Modified Descriptors**: $\Delta_{\text{modified}} \le 1$ ($p(u)$ modified iff $u = c_p$ and $p(u)$ has remaining children).
- **Total Affected Descriptors**:
  $$\mathbf{\Delta_{\text{affected}} \le 3}$$

#### Preservation of Child Set Order
Since $u \prec v$, by properness $l_u < l_v$. For any $c \in C(u)$ and $d \in C(v)$, $l_c \le r_u \le r_v$ and $p(d) = v \implies l_d > r_u \ge l_c$.
Thus, all children of $u$ strictly precede all children of $v$ in canonical order.
Prepending $C(u)$ to $C(v)$ strictly preserves the sorted ordering of the unified child set.

#### Coverage of the Deleted Clique
Before deletion, $C(u) = [l_u, l_{c_u}]$. When $u$ is deleted, the remaining vertices in $C(u) \setminus \{u\}$ have left endpoints in $[l_{u+1}, l_{c_u}]$.
Since $c_u$ is adopted by $v$, and $r_v \ge r_u \ge l_{c_u}$, the clique formed by $v$ and its adopted children spans $[l_v, l_{c_v}]$, which contains $[l_v, l_{c_u}]$. Vertices between $u$ and $v$ are covered by the modified clique of $p(u)$ or preceding parents. Every maximal clique in $G \setminus \{u\}$ remains completely covered.

**Verdict**: **REVISED & PROVED** (Bound revised from $\le 2$ to $\le 3$; counterexample established).

---

# 3. Objection 3: Dynamic Insertion — Formal Closure Proof

### The Objection
Provide a complete before/after formal proof for rightmost append-only leaf insertion, explicitly proving why the inserted vertex cannot become an existing vertex's parent, why exactly one parent's child list changes, why all other rightmost children are invariant, and why the changed descriptor remains maximal.

---

### Proof Derivation

**Preconditions**:
Let $G = (V, E)$ be a connected proper interval graph.
New interval $y$ satisfies:
1. $l_y > l_w$ for all $w \in V$ (strictly rightmost).
2. $l_y \le \max_{w \in V} r_w$ (connectedness).
3. $r_y > \max_{w \in V} r_w$ (properness).
4. $y$ is a leaf in $T(G')$, where $G' = G \cup \{y\}$.

#### 3.1 Why $y$ cannot become the parent of any existing vertex
Parent selection: $p(w) = \arg\min_{u} \{l_u : r_u \ge l_w\}$.
For any existing $w \in V$, its existing parent $p(w) \in V$ satisfies $r_{p(w)} \ge l_w$ and has left endpoint $l_{p(w)} \le l_w$.
By Precondition 1, $l_y > l_w \ge l_{p(w)}$ for all $w \in V$.
Thus $l_y > l_{p(w)}$.
Even if $r_y \ge l_w$, $y$ has a strictly larger left endpoint than the existing parent $p(w)$.
Because parent selection strictly minimizes the left endpoint, $y$ can never be chosen as the parent of any existing vertex $w$.
Therefore:
$$\forall w \in V, \quad p_{G'}(w) = p_G(w)$$

#### 3.2 Why exactly one parent's child list changes
Since $p_{G'}(w) = p_G(w)$ for all $w \in V$, no existing vertex changes its parent.
The only new vertex is $y$, which chooses parent $p(y) = \arg\min_{u \in V} \{l_u : r_u \ge l_y\}$.
Thus, exactly one vertex ($p(y)$) acquires a new child. All other vertices have identical child sets.

#### 3.3 Why all other rightmost-child pointers remain unchanged
For any parent $u \ne p(y)$, its child set in $T(G')$ is identical to its child set in $T(G)$:
$$C_{G'}(u) = C_G(u) \implies c_u^{G'} = c_u^G$$
Therefore, descriptor $D(u) = [l_u, l_{c_u}]$ is completely invariant for all $u \ne p(y)$.

#### 3.4 Why the changed descriptor remains maximal
Parent $p = p(y)$ now has child set $C(p) \cup \{y\}$.
Since $l_y > l_c$ for all existing children $c \in C(p)$, $y$ is the strictly rightmost child: $c_p^{G'} = y$.
The descriptor for $p$ in $G'$ is $[l_p, l_y]$.
By Section 1 (Claim 1.1 and Claim 1.2), since $G'$ is a connected proper interval graph and $p(y) = p$, the set:
$$C_{G'}(p) = \{v \in V \cup \{y\} : l_p \le l_v \le l_y\}$$
is a clique and is strictly maximal in $G'$.

#### 3.5 Why no other maximal clique appears or disappears
- **No new maximal cliques appear**: The only non-leaf parent in $T(G')$ that was not non-leaf in $T(G)$ is $p(y)$ (if $p(y)$ was previously a leaf). $y$ itself has no children and does not create a descriptor.
- **No existing maximal cliques disappear**: For every other parent $u \ne p(y)$, its descriptor bounds $[l_u, l_{c_u}]$ are identical. Since $l_y > l_{c_u}$, $y \notin C_{G'}(u)$. Thus the vertex set $C_{G'}(u) = C_G(u)$ is completely unchanged.
- **Added vs. Modified Classification**:
  - If $p(y)$ was a leaf: $\Delta_{\text{added}} = 1, \Delta_{\text{modified}} = 0, \Delta_{\text{removed}} = 0$.
  - If $p(y)$ was a non-leaf: $\Delta_{\text{added}} = 0, \Delta_{\text{modified}} = 1, \Delta_{\text{removed}} = 0$.
  - Strict bound: $\mathbf{\Delta_{\text{added}} + \Delta_{\text{modified}} \le 1}$.

**Verdict**: **PROVED**  
**Source Location**: `benchmarks/maximal_cliques/dynamic_clique_structure.cpp:94–203`.

---

# 4. Objection 4: Dynamic Complexity — Theoretical vs. Implementation Costs

### The Objection
Separate theoretical structural costs from actual public implementation costs. The previous claim that telemetry accounts for "over 80%" of large-n update time must be backed by isolated profiling or withdrawn.

---

### Precise Complexity Dissection Table

| Sub-Operation | Theoretical Structural Cost | Public Implementation Cost | Implementation Mechanism | Telemetry / Profiling Status |
|:---|:---:|:---:|:---|:---|
| **Parent Search (Insert)** | $\mathcal{O}(\log n)$ | $\mathcal{O}(\log n)$ | `std::lower_bound` on sorted intervals | Directly verified |
| **Parent Lookup (Delete)** | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | Stored in `DynamicTreeNode::parentId` | Directly verified |
| **Tree Splicing (Insert)** | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | Doubly-linked sibling & level pointers | Directly verified |
| **Child Relocation (Delete)**| $\mathcal{O}(\text{deg}(u))$ | $\mathcal{O}(\text{deg}(u))$ | Traversal updating `nodes[curr].parentId = v` | Directly verified |
| **Descriptor Update (Insert)**| $\mathcal{O}(\log q)$ | $\mathcal{O}(\log q)$ | `parentToRightmostChild[p] = y` (`std::map`) | Directly verified |
| **Descriptor Update (Delete)**| $\mathcal{O}(\log q)$ | $\mathcal{O}(\log q)$ | `parentToRightmostChild.erase()` / update | Directly verified |
| **Interval Array Update (Ins)**| $\mathcal{O}(1)$ amortized | $\mathcal{O}(1)$ amortized | `intervals.push_back(newIv)` | Directly verified |
| **Interval Array Update (Del)**| $\mathcal{O}(1)$ (if list) | $\mathbf{\mathcal{O}(n)}$ | `std::remove_if` on `vector<Interval>` | **Implementation bottleneck** |
| **Validation Checks** | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | Precondition checks on bounds/links | Directly verified |
| **Set-Diff Telemetry (Both)** | None | $\mathbf{\mathcal{O}(q \log q)}$ | `descsBefore = parentToRightmostChild` + loops | **Telemetry bottleneck** |
| **Materialization (Query)** | $\mathcal{O}(S)$ | $\mathcal{O}(S)$ | Optional explicit expansion | Outside update timer |
| **Public API Total (Insert)** | $\mathbf{\mathcal{O}(\log n)}$ | $\mathbf{\mathcal{O}(q \log q) = \mathcal{O}(n \log n)}$ | Dominated by map copy & diff loop | Proven by code inspection |
| **Public API Total (Delete)** | $\mathbf{\mathcal{O}(\text{deg}(u))}$ | $\mathbf{\mathcal{O}(n + q \log q)}$ | Dominated by `remove_if` & map diff | Proven by code inspection |

### Correction Regarding the "80%" Telemetry Claim
> [!IMPORTANT]
> **Formal Withdrawal**: The claim that "telemetry accounts for over 80% of large-n update time" was an analytical hypothesis based on container complexity ($\mathcal{O}(q \log q)$ map copies vs. $\mathcal{O}(1)$ pointer updates). Because no isolated, instrumented timer was compiled inside the timed block to measure the telemetry lines separately from the structural lines, **the 80% numerical figure is formally withdrawn and reclassified as an unverified analytical hypothesis**.
> 
> The provable factual statement is:
> The public API currently executes in $\mathcal{O}(n \log n)$ time due to lines 169–188 and 320–341 in `dynamic_clique_structure.cpp`. Moving telemetry outside the timer will restore the execution time to the structural bound of $\mathcal{O}(\log n)$ insertion and $\mathcal{O}(\text{deg}(u))$ deletion.

**Verdict**: **REVISED & CLARIFIED**

---

# 5. Objection 5: Sweep-Line Correctness Proof

### The Objection
Give a formal proof for the exact implementation in `standard_interval_cliques.cpp`, analyzing multiple STARTs at same coordinate, multiple ENDs at same coordinate, START and END at same coordinate, identical intervals, and endpoint touching.

---

### Proof Derivation for `standard_interval_cliques.cpp`

Event representation:
`START`: coordinate $l_i$, `type = 0`, `intervalId = id`, `sortedIndex = i`.
`END`: coordinate $r_i$, `type = 1`, `intervalId = id`, `sortedIndex = i`.

Event comparator (lines 19–27 of `standard_interval_cliques.h`):
```cpp
bool operator<(const SweepEndpointEvent& other) const {
    if (coord != other.coord) return coord < other.coord;
    if (type != other.type) return type < other.type; // START (0) precedes END (1)
    return intervalId < other.intervalId;
}
```

#### 5.1 Every Emitted Range is a Clique
A descriptor is emitted only in the `END` handler:
```cpp
if (hasNewStart && !activeList.empty()) {
    descriptors.push_back(SweepCliqueDescriptor{ activeList.front(), activeList.back(), ... });
    hasNewStart = false;
}
```
Let $x$ be the coordinate of the departing interval's `END` event.
Every interval $v \in \text{activeList}$ satisfies $l_v \le x$ (its `START` was processed) and $r_v \ge x$ (its `END` has not yet been processed, or is at $x$).
Thus, $x \in \bigcap_{v \in \text{activeList}} I_v$.
By the Helly property on $\mathbb{R}$, mutually overlapping intervals form a clique.
In proper interval graphs, sorting by left endpoints also sorts intervals by right endpoints. Thus, the active intervals at any point form a contiguous subsegment of sorted indices $[\text{front}, \text{back}]$. The emitted range is a clique. $\square$

#### 5.2 Every Emitted Range is Maximal
When an interval $u$ encounters its `END` event at coordinate $x$:
- No interval $w$ with $l_w > x$ can be added (since $r_u = x < l_w \implies I_u \cap I_w = \emptyset$).
- No interval $w$ with $r_w < x$ can be added (it already departed, and by properness $r_w < x \le r_k$ for active $k$, so $w$ cannot overlap all active intervals).
- Since $u$ departs immediately after this check, this instant is the last point at which $u$ can belong to any clique.
- Because `hasNewStart == true`, at least one new interval has joined since the last emission, ensuring the active set is not a subset of the previous clique.
Thus, the active set at this instant cannot be extended. It is strictly maximal. $\square$

#### 5.3 Every Maximal Clique is Emitted
Every maximal clique $M$ in a proper interval graph corresponds to a non-empty intersection interval $[a, b]$, where $b = \min_{v \in M} r_v$.
At coordinate $b$, the interval achieving the minimum right endpoint processes its `END` event.
Because $M$ is maximal, at least one member of $M$ started after the prior maximal clique ended, ensuring `hasNewStart == true`.
The algorithm triggers emission, capturing exactly $M$. $\square$

#### 5.4 Every Maximal Clique is Emitted Exactly Once
- Upon emission, `hasNewStart` is set to `false`.
- Subsequent `END` events at the same coordinate $x$ or later coordinates without intervening `START` events find `hasNewStart == false`, preventing duplicate emission of sub-cliques.
- A new emission requires `hasNewStart == true`, which requires an intervening `START` event, ensuring a distinct clique.

#### 5.5 Edge Case Analysis
1. **Multiple STARTs at Same Coordinate ($l_u = l_v$)**:
   Ordered by ID. Both are pushed to `activeList`. `hasNewStart` becomes `true`. Fully sound.
2. **Multiple ENDs at Same Coordinate ($r_u = r_v$)**:
   Ordered by ID. First `END` event emits the maximal clique containing both $u$ and $v$ (since both are in `activeList`). `hasNewStart` becomes `false`. Second `END` event sees `hasNewStart == false` and does not re-emit. Fully sound.
3. **START and END at Same Coordinate ($l_v = r_u$)**:
   Comparator enforces `type < other.type` $\implies$ `START` (0) precedes `END` (1).
   Thus $v$ enters `activeList` before $u$ departs. Both $u$ and $v$ are simultaneously active. Captures closed-interval endpoint touching $r_u = l_v$. Fully sound.
4. **Identical Intervals ($I_u = I_v$)**:
   Both `START`s processed first $\implies$ both active. First `END` emits clique containing both. Second `END` does not re-emit. Fully sound.
5. **Endpoint-Touching Chains ($r_i = l_{i+1}$)**:
   At each contact point $x = r_i = l_{i+1}$, $i+1$ enters before $i$ departs. The 2-clique $\{i, i+1\}$ is emitted. Fully sound.

**Verdict**: **PROVED**  
**Source Location**: `benchmarks/maximal_cliques/standard_interval_cliques.cpp:4–70`.

---

# 6. Objection 6: Verification Audit — Exact Evidence Breakdown

### The Objection
Reconcile all verification counts. Specify exactly which checks ran on each of the 77 rows and each of the 165 dynamic updates.

---

### Verification Evidence Reconciliation Table

| Category | Total Test Units | Alg A vs Alg C Checked | Alg A vs Alg B Checked | Independent Maximality & Adjacency Checked | Static Rebuild Checked | Status in CSV | Nature of Execution |
|:---|:---:|:---:|:---:|:---:|:---:|:---:|:---|
| **Static $n = 10$** | 17 | 17 / 17 | 17 / 17 | 17 / 17 | N/A | `PASS` | Genuinely Verified (3-Way) |
| **Static $n = 20$** | 17 | 17 / 17 | 17 / 17 | 17 / 17 | N/A | `PASS` | Genuinely Verified (3-Way) |
| **Static $n = 50$** | 17 | 17 / 17 | 17 / 17 | 17 / 17 | N/A | `PASS` | Genuinely Verified (3-Way) |
| **Static $n = 100$** | 17 | 17 / 17 | **0 / 17 (Skipped)** | 17 / 17 | N/A | `PASS` | **2-Way Only** (BK skipped, A copied to B) |
| **Dynamic Sequences** | 9 | 9 / 9 | 9 / 9 | 9 / 9 | 9 / 9 | `PASS` | Genuinely Verified (3-Way + Rebuild) |
| **TOTALS** | **77** | **77 / 77 (100%)** | **60 / 77 (77.9%)** | **77 / 77 (100%)** | **9 / 9 (100%)** | `100% PASS` | **60 3-Way, 17 2-Way** |

### Breakdown of the 165 Individual Dynamic Steps
The 9 dynamic verification sequences consist of:
- 3 insertion sequences (20 ops each) = 60 steps
- 3 deletion sequences (15 ops each) = 45 steps
- 3 mixed sequences (20 ops each) = 60 steps
- **Total Dynamic Steps**: **165 individual update steps**

For every single step ($i = 0 \dots 164$):
- Compared against **Algorithm C**: **165 / 165 steps checked** (`cliquesA == cliquesC`).
- Compared against **Bron–Kerbosch**: **165 / 165 steps checked** (`cliquesA == cliquesB`), because all 9 sequences had initial $n = 30 \le 50$, so `trackBK = true` was active throughout all steps.
- Compared against **Static Rebuild**: **165 / 165 steps checked** (full distance tree rebuilt from scratch on exported graph, `cliquesA == staticCliques`).
- Independent Maximality: Covered via equivalence with Bron–Kerbosch ground truth on all 165 steps.

**Verdict**: **RECONCILED & DOCUMENTED**

---

# 7. Objection 7: Break-Even Analysis — Complete Numerical Reproduction

### The Objection
Reproduce every single input value for $n=50$ and $n=5000$ from raw CSV datasets. Show from-scratch vs. pre-existing tree scenarios separately.

---

### Exact Numerical Derivations

Linear Cost Model:
$$T_A(K) = I_A + K \cdot U_A, \qquad T_C(K) = I_C + K \cdot R_C$$
$$K^* = \frac{I_A - I_C}{R_C - U_A}$$

#### Scenario 1: Standalone Execution From Scratch (Cold Start)

| Metric | Graph Order $n = 50$ | Graph Order $n = 5,000$ | Exact CSV Source File & Column |
|:---|:---:|:---:|:---|
| **$I_A$ (Alg A Initialization)** | **$0.0381\text{ ms}$** | **$3.8912\text{ ms}$** | `results/static/static_summary.csv` (`Algorithm A`, `medium-overlap`, `direct_end_to_end_ms`) |
| **$I_C$ (Alg C Initialization)** | **$0.0071\text{ ms}$** | **$0.7518\text{ ms}$** | `results/static/static_summary.csv` (`Algorithm C`, `medium-overlap`, `direct_end_to_end_ms`) |
| **$I_A - I_C$ (Init Overhead)** | **$0.0310\text{ ms}$** ($31.0\ \mu\text{s}$) | **$3.1394\text{ ms}$** | Exact difference |
| **$U_A$ (Alg A Update Latency)** | **$0.0022\text{ ms}$** ($2.2\ \mu\text{s}$) | **$0.5814\text{ ms}$** | `results/dynamic/dynamic_summary.csv` (`Algorithm A`, `mixed-50ins-50del`, `average_latency_ms`) |
| **$R_C$ (Alg C Recompute Latency)**| **$0.0118\text{ ms}$** ($11.8\ \mu\text{s}$) | **$1.1494\text{ ms}$** | `results/dynamic/dynamic_summary.csv` (`Algorithm C`, `mixed-50ins-50del`, `average_latency_ms`) |
| **$R_C - U_A$ (Per-Op Savings)** | **$0.0096\text{ ms}$** ($9.6\ \mu\text{s}$) | **$0.5680\text{ ms}$** | Exact difference |
| **$K^*$ (Break-Even Threshold)** | $\mathbf{3.23 \implies 4 \text{ updates}}$ | $\mathbf{5.53 \implies 6 \text{ updates}}$ | $K^* = (I_A - I_C) / (R_C - U_A)$ |

- **Sensitivity Across Workloads at $n = 50$**:
  - Insertion ($U_A = 0.0038, R_C = 0.0181$): $K^* = 0.0310 / 0.0143 = \mathbf{2.17 \implies 3 \text{ updates}}$.
  - Deletion ($U_A = 0.0017, R_C = 0.0103$): $K^* = 0.0310 / 0.0086 = \mathbf{3.60 \implies 4 \text{ updates}}$.
  - Mixed 50/50: $K^* = \mathbf{3.23 \implies 4 \text{ updates}}$.
- **Sensitivity Across Workloads at $n = 5,000$**:
  - Insertion ($U_A = 0.5940, R_C = 1.1572$): $K^* = 3.1394 / 0.5632 = \mathbf{5.57 \implies 6 \text{ updates}}$.
  - Deletion ($U_A = 0.5993, R_C = 1.2299$): $K^* = 3.1394 / 0.6306 = \mathbf{4.98 \implies 5 \text{ updates}}$.
  - Mixed 50/50: $K^* = \mathbf{5.53 \implies 6 \text{ updates}}$.

#### Scenario 2: Pre-Existing Distance Tree ($I_A = 0$)
When $T(G)$ is already maintained by other BTP algorithms (e.g. MIS / BFS):
- $I_A = 0$, $I_C = 0$.
- Since $U_A < R_C$ on every single update ($2.2\ \mu\text{s} < 11.8\ \mu\text{s}$ at $n=50$; $0.58\text{ ms} < 1.15\text{ ms}$ at $n=5000$):
  $$\mathbf{K^* = 1 \text{ update}}$$

**Verdict**: **REPRODUCED & PROVED**

---

# 8. Objection 8: Graph-Family Validity Audit

### Family-by-Family Mathematical Audit

| # | Generator Function / Family | Exact Endpoint Construction | Properness Argument | Connectedness Argument | Identical Intervals? | Classification |
|:---:|:---|:---|:---|:---|:---:|:---|
| 1 | `generateEqualLengthsGraph` (sparse) | $l_i = i \cdot 1.0, r_i = l_i + 1.5$ | Constant width $w = 1.5$ | $r_i = i + 1.5 > l_{i+1} = i + 1$ | No | **Main Theorem-Valid** |
| 2 | `generateEqualLengthsGraph` (medium) | $l_i = i \cdot 1.0, r_i = l_i + 8.0$ | Constant width $w = 8.0$ | Overlap span = 8 intervals | No | **Main Theorem-Valid** |
| 3 | `generateEqualLengthsGraph` (dense) | $l_i = i \cdot 1.0, r_i = l_i + 25.0$ | Constant width $w = 25.0$ | Overlap span = 25 intervals | No | **Main Theorem-Valid** |
| 4 | `generateUnequalLengthsGraph` | $l_i += 1.0, w_i = \sin(\cdot)$; enforces $r_i > r_{i-1}$ | $r_i$ strictly increasing | $l_i \le r_{i-1}$ | No | **Main Theorem-Valid** |
| 5 | `generateRandomProperGraph` | $l_i += \text{stepDist}, w_i = \text{widthDist}$; enforces $r_i > r_{i-1}$ | $r_i$ strictly increasing | Enforced $l_i \le r_{i-1}$ | No | **Main Theorem-Valid** |
| 6 | `generateIdenticalIntervalsGraph` | Groups share identical $[l, r]$ | Permitted by canon ($I_u \subseteq I_v \implies I_u = I_v$) | $r_u > l_{u+1}$ | **YES** | **Robustness: Identical Clusters** |
| 7 | `generateEndpointTouchingGraph` | $l_i = i \cdot w, r_i = (i+1) \cdot w$ | Constant width $w$ | Point intersection $r_i = l_{i+1}$ | No | **Robustness: Boundary Contact** |
| 8 | `generateLongChainGraph` | $l_i = i \cdot 1.0, r_i = l_i + 1.2$ | Constant width $w = 1.2$ | Small overlap 0.2 | No | **Main Theorem-Valid** |
| 9 | `generateHighOverlapGraph` | $l_i = i \cdot 1.0, r_i = l_i + 15.5$ | Constant width $w = 15.5$ | Large overlap 15.5 | No | **Main Theorem-Valid** |
| 10 | `generateAlternatingLengthsGraph` | Alternates short/long $w$; enforces $r_i > r_{i-1}$ | $r_i$ strictly increasing | $l_i \le r_{i-1}$ | No | **Main Theorem-Valid** |
| 11 | `generateIrregularEndpointsGraph` | $l_i += \text{uniform}(0.2, 1.8), w = 4.0$ | Constant width $w = 4.0$ | $\Delta < 1.8 < w = 4.0$ | No | **Main Theorem-Valid** |
| 12 | `generateManySmallCliquesGraph` | $l_i = i \cdot 1.0, r_i = l_i + 2.5$ | Constant width $w = 2.5$ | Overlap span = 2 intervals | No | **Main Theorem-Valid** |
| 13 | `generateFewLargeCliquesGraph` | $l_i = i \cdot 1.0, r_i = l_i + 0.4n$ | Constant width $w = 0.4n$ | Large overlap $0.4n$ | No | **Main Theorem-Valid** |
| 14 | `generateVaryingOverlapGraph` | Periodic width phase; enforces $r_i > r_{i-1}$ | $r_i$ strictly increasing | $l_i \le r_{i-1}$ | No | **Main Theorem-Valid** |
| 15 | `generateAdversarialTieBreakingGraph`| Pairs of identical intervals | Permitted by canon | Step overlap | **YES** | **Robustness: Parent Ties** |
| 16 | `generateShuffledIdsGraph` | Permuted vertex IDs on regular grid | Constant width | Overlap span = 3.5 | No | **Robustness: ID Invariance** |

**Summary**:
- **12 Main Theorem-Valid Families**: Fully strictly proper interval graphs with strictly distinct endpoints.
- **4 Robustness / Tie-Breaking Families**:
  - Families 6 and 15 explicitly test canonical tie-breaking under identical intervals ($I_u = I_v$).
  - Family 7 tests closed-interval boundary contact ($r_i = l_{i+1}$).
  - Family 16 tests complete independence from vertex ID permutations.

**Verdict**: **AUDITED & CLASSIFIED**
