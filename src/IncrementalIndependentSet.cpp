#include "IncrementalIndependentSet.h"
#include "IndependentSet.h"
#include <iostream>
#include <sstream>
#include <algorithm>

// ============================================================================
// ParentAVLTree Implementation
// ============================================================================

ParentAVLNode::ParentAVLNode(const Interval& iv)
    : rightKey(iv.right), idKey(iv.id),
      item{iv.left, iv.right, iv.id},
      minCandidate{iv.left, iv.right, iv.id},
      height(1), left(nullptr), right(nullptr) {}

ParentAVLTree::ParentAVLTree() : root(nullptr) {}

ParentAVLTree::~ParentAVLTree() {
    clear();
}

ParentAVLNode* ParentAVLTree::cloneTree(const ParentAVLNode* node) {
    if (!node) return nullptr;
    ParentAVLNode* copy = new ParentAVLNode(Interval{node->item.id, node->item.left, node->item.right});
    copy->height = node->height;
    copy->minCandidate = node->minCandidate;
    copy->left = cloneTree(node->left);
    copy->right = cloneTree(node->right);
    return copy;
}

ParentAVLTree::ParentAVLTree(const ParentAVLTree& other) : root(cloneTree(other.root)) {}

ParentAVLTree& ParentAVLTree::operator=(const ParentAVLTree& other) {
    if (this != &other) {
        clear();
        root = cloneTree(other.root);
    }
    return *this;
}

ParentAVLTree::ParentAVLTree(ParentAVLTree&& other) noexcept : root(other.root) {
    other.root = nullptr;
}

ParentAVLTree& ParentAVLTree::operator=(ParentAVLTree&& other) noexcept {
    if (this != &other) {
        clear();
        root = other.root;
        other.root = nullptr;
    }
    return *this;
}

void ParentAVLTree::clear() {
    destroyTree(root);
    root = nullptr;
}

void ParentAVLTree::destroyTree(ParentAVLNode* node) {
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

int ParentAVLTree::getHeight(ParentAVLNode* n) {
    return n ? n->height : 0;
}

int ParentAVLTree::getBalance(ParentAVLNode* n) {
    return n ? (getHeight(n->left) - getHeight(n->right)) : 0;
}

void ParentAVLTree::updateNode(ParentAVLNode* n) {
    if (!n) return;
    n->height = 1 + std::max(getHeight(n->left), getHeight(n->right));
    n->minCandidate = n->item;
    if (n->left && n->left->minCandidate < n->minCandidate) {
        n->minCandidate = n->left->minCandidate;
    }
    if (n->right && n->right->minCandidate < n->minCandidate) {
        n->minCandidate = n->right->minCandidate;
    }
}

ParentAVLNode* ParentAVLTree::rotateRight(ParentAVLNode* y) {
    ParentAVLNode* x = y->left;
    ParentAVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateNode(y);
    updateNode(x);
    return x;
}

ParentAVLNode* ParentAVLTree::rotateLeft(ParentAVLNode* x) {
    ParentAVLNode* y = x->right;
    ParentAVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateNode(x);
    updateNode(y);
    return y;
}

ParentAVLNode* ParentAVLTree::insertNode(ParentAVLNode* node, const Interval& iv) {
    if (!node) {
        return new ParentAVLNode(iv);
    }

    if (iv.right < node->rightKey || (iv.right == node->rightKey && iv.id < node->idKey)) {
        node->left = insertNode(node->left, iv);
    } else {
        node->right = insertNode(node->right, iv);
    }

    updateNode(node);
    int balance = getBalance(node);

    // Left-Left Case
    if (balance > 1 && (iv.right < node->left->rightKey || 
        (iv.right == node->left->rightKey && iv.id < node->left->idKey))) {
        return rotateRight(node);
    }
    // Right-Right Case
    if (balance < -1 && (iv.right > node->right->rightKey || 
        (iv.right == node->right->rightKey && iv.id > node->right->idKey))) {
        return rotateLeft(node);
    }
    // Left-Right Case
    if (balance > 1 && (iv.right > node->left->rightKey || 
        (iv.right == node->left->rightKey && iv.id > node->left->idKey))) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    // Right-Left Case
    if (balance < -1 && (iv.right < node->right->rightKey || 
        (iv.right == node->right->rightKey && iv.id < node->right->idKey))) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

void ParentAVLTree::insert(const Interval& iv) {
    root = insertNode(root, iv);
}

bool ParentAVLTree::findParent(double queryLeft, ParentCandidate& bestCand) const {
    const ParentAVLNode* curr = root;
    bool found = false;

    while (curr) {
        if (curr->rightKey < queryLeft) {
            // All nodes in left subtree have rightKey < queryLeft, so neither curr nor left subtree qualify
            curr = curr->right;
        } else {
            // curr satisfies rightKey >= queryLeft
            if (!found || curr->item < bestCand) {
                bestCand = curr->item;
                found = true;
            }
            // All nodes in right subtree have rightKey >= curr->rightKey >= queryLeft
            if (curr->right) {
                if (!found || curr->right->minCandidate < bestCand) {
                    bestCand = curr->right->minCandidate;
                    found = true;
                }
            }
            // Recurse left to find any candidate with even smaller left endpoint
            curr = curr->left;
        }
    }

    return found;
}

// ============================================================================
// LevelAVLTree Implementation
// ============================================================================

LevelAVLNode::LevelAVLNode(const CanonicalKey& k, int dp)
    : key(k), dpValue(dp), maxDP(dp), height(1), left(nullptr), right(nullptr) {}

LevelAVLTree::LevelAVLTree() : root(nullptr) {}

LevelAVLTree::~LevelAVLTree() {
    clear();
}

LevelAVLNode* LevelAVLTree::cloneTree(const LevelAVLNode* node) {
    if (!node) return nullptr;
    LevelAVLNode* copy = new LevelAVLNode(node->key, node->dpValue);
    copy->maxDP = node->maxDP;
    copy->height = node->height;
    copy->left = cloneTree(node->left);
    copy->right = cloneTree(node->right);
    return copy;
}

LevelAVLTree::LevelAVLTree(const LevelAVLTree& other) : root(cloneTree(other.root)) {}

LevelAVLTree& LevelAVLTree::operator=(const LevelAVLTree& other) {
    if (this != &other) {
        clear();
        root = cloneTree(other.root);
    }
    return *this;
}

LevelAVLTree::LevelAVLTree(LevelAVLTree&& other) noexcept : root(other.root) {
    other.root = nullptr;
}

LevelAVLTree& LevelAVLTree::operator=(LevelAVLTree&& other) noexcept {
    if (this != &other) {
        clear();
        root = other.root;
        other.root = nullptr;
    }
    return *this;
}

void LevelAVLTree::clear() {
    destroyTree(root);
    root = nullptr;
}

void LevelAVLTree::destroyTree(LevelAVLNode* node) {
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

int LevelAVLTree::getHeight(LevelAVLNode* n) {
    return n ? n->height : 0;
}

int LevelAVLTree::getBalance(LevelAVLNode* n) {
    return n ? (getHeight(n->left) - getHeight(n->right)) : 0;
}

void LevelAVLTree::updateNode(LevelAVLNode* n) {
    if (!n) return;
    n->height = 1 + std::max(getHeight(n->left), getHeight(n->right));
    n->maxDP = n->dpValue;
    if (n->left && n->left->maxDP > n->maxDP) {
        n->maxDP = n->left->maxDP;
    }
    if (n->right && n->right->maxDP > n->maxDP) {
        n->maxDP = n->right->maxDP;
    }
}

LevelAVLNode* LevelAVLTree::rotateRight(LevelAVLNode* y) {
    LevelAVLNode* x = y->left;
    LevelAVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateNode(y);
    updateNode(x);
    return x;
}

LevelAVLNode* LevelAVLTree::rotateLeft(LevelAVLNode* x) {
    LevelAVLNode* y = x->right;
    LevelAVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateNode(x);
    updateNode(y);
    return y;
}

LevelAVLNode* LevelAVLTree::insertNode(LevelAVLNode* node, const CanonicalKey& key, int dpVal) {
    if (!node) {
        return new LevelAVLNode(key, dpVal);
    }

    if (key < node->key) {
        node->left = insertNode(node->left, key, dpVal);
    } else {
        node->right = insertNode(node->right, key, dpVal);
    }

    updateNode(node);
    int balance = getBalance(node);

    if (balance > 1 && key < node->left->key) {
        return rotateRight(node);
    }
    if (balance < -1 && node->right->key < key) {
        return rotateLeft(node);
    }
    if (balance > 1 && node->left->key < key) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1 && key < node->right->key) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

void LevelAVLTree::insert(const CanonicalKey& key, int dpVal) {
    root = insertNode(root, key, dpVal);
}

int LevelAVLTree::queryPrefixMaxStrictlyLeft(const CanonicalKey& key) const {
    const LevelAVLNode* curr = root;
    int ans = 0;

    while (curr) {
        if (key < curr->key || key == curr->key) {
            // curr and right subtree are not strictly left of key
            curr = curr->left;
        } else {
            // curr is strictly left of key
            ans = std::max(ans, curr->dpValue);
            if (curr->left) {
                ans = std::max(ans, curr->left->maxDP);
            }
            curr = curr->right;
        }
    }

    return ans;
}

int LevelAVLTree::getMaxDP() const {
    return root ? root->maxDP : 0;
}

// ============================================================================
// IncrementalIndependentSet Implementation
// ============================================================================

IncrementalIndependentSet::IncrementalIndependentSet()
    : n(0), h(0), currentAlpha(0) {}

bool IncrementalIndependentSet::initialize(const IntervalGraph& graph, const DistanceTree& tree, std::string& errorMessage) {
    n = graph.getNumVertices();
    if (n == 0) {
        errorMessage = "Cannot initialize incremental MIS from an empty graph.";
        return false;
    }

    h = tree.getNumLevels();
    intervals = graph.getSortedIntervals();

    // Compute static DP as the initial baseline state
    auto staticResult = IndependentSet::computeExactDP(graph, tree);
    currentAlpha = staticResult.alpha;
    BEST = staticResult.bestValues;

    idToLevel.clear();
    idToParent.clear();
    idToDP.clear();
    parentSearchTree.clear();
    levelTrees.clear();
    levelTrees.resize(h + 1);

    for (const auto& node : tree.getAllNodes()) {
        idToLevel[node.vertexId] = node.level;
        idToParent[node.vertexId] = node.parentId;
    }

    for (int i = 1; i <= h; ++i) {
        const auto& levelNodes = tree.getLevelVerticesOneBased(i);
        for (size_t k = 0; k < levelNodes.size(); ++k) {
            int uid = levelNodes[k];
            int dpVal = staticResult.dpValues[i][k];
            idToDP[uid] = dpVal;

            const auto& iv = graph.getIntervalById(uid);
            CanonicalKey key{iv.left, iv.right, iv.id};
            levelTrees[i].insert(key, dpVal);
        }
    }

    for (const auto& iv : intervals) {
        parentSearchTree.insert(iv);
    }

    return true;
}

int IncrementalIndependentSet::getVertexDP(int vertexId) const {
    auto it = idToDP.find(vertexId);
    return (it != idToDP.end()) ? it->second : -1;
}

int IncrementalIndependentSet::getVertexLevel(int vertexId) const {
    auto it = idToLevel.find(vertexId);
    return (it != idToLevel.end()) ? it->second : -1;
}

int IncrementalIndependentSet::getVertexParent(int vertexId) const {
    auto it = idToParent.find(vertexId);
    return (it != idToParent.end()) ? it->second : -1;
}

bool IncrementalIndependentSet::findParentLogN(double queryLeft, int& parentId, double& parentLeft, double& parentRight) const {
    ParentCandidate bestCand{0.0, 0.0, -1};
    if (parentSearchTree.findParent(queryLeft, bestCand)) {
        parentId = bestCand.id;
        parentLeft = bestCand.left;
        parentRight = bestCand.right;
        return true;
    }
    return false;
}

bool IncrementalIndependentSet::validateLeafInsertion(const Interval& newIv, std::string& diagnosticMsg) const {
    // 1. Basic endpoint sanity
    if (newIv.left > newIv.right) {
        diagnosticMsg = "Rejected: Invalid interval endpoints (left > right).";
        return false;
    }

    // 2. ID uniqueness
    for (const auto& iv : intervals) {
        if (iv.id == newIv.id) {
            std::ostringstream oss;
            oss << "Rejected: Vertex ID " << newIv.id << " already exists in graph.";
            diagnosticMsg = oss.str();
            return false;
        }
    }

    // 3. Properness check with all existing intervals
    for (const auto& iv : intervals) {
        // Strict containment check
        if (newIv.left <= iv.left && iv.right <= newIv.right && (newIv.left < iv.left || iv.right < newIv.right)) {
            std::ostringstream oss;
            oss << "Rejected: Properness violation: existing vertex " << iv.id << " [" << iv.left << ", " << iv.right
                << "] is properly contained in new vertex " << newIv.id << " [" << newIv.left << ", " << newIv.right << "].";
            diagnosticMsg = oss.str();
            return false;
        }
        if (iv.left <= newIv.left && newIv.right <= iv.right && (iv.left < newIv.left || newIv.right < iv.right)) {
            std::ostringstream oss;
            oss << "Rejected: Properness violation: new vertex " << newIv.id << " [" << newIv.left << ", " << newIv.right
                << "] is properly contained in existing vertex " << iv.id << " [" << iv.left << ", " << iv.right << "].";
            diagnosticMsg = oss.str();
            return false;
        }
    }

    // 4. Connectedness check: does newIv intersect at least one existing interval?
    bool connected = false;
    for (const auto& iv : intervals) {
        if (newIv.left <= iv.right && iv.left <= newIv.right) {
            connected = true;
            break;
        }
    }
    if (!connected) {
        diagnosticMsg = "Rejected: Connectedness violation: new interval does not intersect any existing vertex in G.";
        return false;
    }

    // 5. Parent search in O(log n)
    int parentId = -1;
    double pLeft = 0.0, pRight = 0.0;
    if (!findParentLogN(newIv.left, parentId, pLeft, pRight)) {
        diagnosticMsg = "Rejected: No valid parent found (new interval does not reach any existing vertex).";
        return false;
    }

    // 6. Leaf condition: Verify that NO existing vertex adopts newIv as parent
    ParentCandidate newCand{newIv.left, newIv.right, newIv.id};
    for (const auto& w : intervals) {
        if (newIv.right >= w.left) {
            // newIv is a candidate to parent w
            int curParentId = idToParent.at(w.id);
            if (curParentId == w.id) {
                // w is root: root parent is root itself
                continue;
            }
            // Find current parent interval of w
            for (const auto& pIv : intervals) {
                if (pIv.id == curParentId) {
                    ParentCandidate curParentCand{pIv.left, pIv.right, pIv.id};
                    if (newCand < curParentCand) {
                        std::ostringstream oss;
                        oss << "Rejected: Leaf-insertion model violated: existing vertex " << w.id
                            << " would adopt newly inserted vertex " << newIv.id 
                            << " as parent instead of its current parent " << curParentId
                            << " (new vertex would not be a leaf).";
                        diagnosticMsg = oss.str();
                        return false;
                    }
                    break;
                }
            }
        }
    }

    // 7. Rightmost append-only model: The new vertex must be the rightmost vertex in its resulting level
    int pLevel = idToLevel.at(parentId);
    int uLevel = pLevel + 1;
    if (uLevel <= h) {
        for (const auto& iv : intervals) {
            if (idToLevel.at(iv.id) == uLevel) {
                // Canonical order comparison: every existing vertex in uLevel must precede newIv
                if (!(iv < newIv)) {
                    std::ostringstream oss;
                    oss << "Rejected: Rightmost append-only leaf model violated: new vertex " << newIv.id
                        << " does not occur after existing vertex " << iv.id 
                        << " in resulting Level " << uLevel << " (canonical order violation). "
                        << "Middle-of-level or leftmost leaf insertions can invalidate DP values "
                        << "and prefix maxima of existing descendants in subsequent levels.";
                    diagnosticMsg = oss.str();
                    return false;
                }
            }
        }
    }

    diagnosticMsg = "Valid rightmost leaf insertion.";
    return true;
}

IncrementalUpdateResult IncrementalIndependentSet::insertLeaf(const Interval& newIv) {
    IncrementalUpdateResult res;
    res.newVertexId = newIv.id;
    res.oldAlpha = currentAlpha;
    res.newAlpha = currentAlpha;
    res.alphaIncreased = false;

    // Validate the leaf insertion
    std::string diag;
    if (!validateLeafInsertion(newIv, diag)) {
        res.success = false;
        res.diagnostic = diag;
        return res;
    }

    // 1. Locate parent p in O(log n) time
    int pId = -1;
    double pL = 0.0, pR = 0.0;
    findParentLogN(newIv.left, pId, pL, pR);

    res.parentId = pId;
    int pLevel = idToLevel.at(pId);
    int uLevel = pLevel + 1;
    res.level = uLevel;
    res.depth = uLevel - 1;

    // 2. Query maximum DP value over vertices left of p in Level (uLevel - 1) in O(log n) time
    CanonicalKey pKey{pL, pR, pId};
    int leftMax = levelTrees[pLevel].queryPrefixMaxStrictlyLeft(pKey);
    res.prefixMaxLeftOfP = leftMax;

    // 3. Obtain BEST[uLevel - 2]
    int bestPrev = 0;
    if (uLevel - 2 >= 0 && uLevel - 2 < static_cast<int>(BEST.size())) {
        bestPrev = BEST[uLevel - 2];
    }
    res.bestPrev = bestPrev;

    // 4. Compute DP[i][u] using Theorem 3.14 recurrence
    int dpU = 1 + std::max(bestPrev, leftMax);
    res.dpValue = dpU;

    // 5. Update level trees
    if (uLevel > h) {
        h = uLevel;
        levelTrees.resize(h + 1);
        BEST.resize(h + 1, BEST.empty() ? 0 : BEST.back());
    }

    CanonicalKey uKey{newIv.left, newIv.right, newIv.id};
    levelTrees[uLevel].insert(uKey, dpU);

    // 6. Update parent search tree in O(log n) time
    parentSearchTree.insert(newIv);

    // 7. Update BEST array and global alpha in O(1) time
    BEST[uLevel] = std::max(BEST[uLevel], dpU);
    if (dpU > currentAlpha) {
        currentAlpha = dpU;
        res.alphaIncreased = true;
    }
    res.newAlpha = currentAlpha;

    // 8. Commit state
    intervals.push_back(newIv);
    idToLevel[newIv.id] = uLevel;
    idToParent[newIv.id] = pId;
    idToDP[newIv.id] = dpU;
    n++;

    res.success = true;
    std::ostringstream oss;
    oss << "Leaf vertex " << newIv.id << " successfully inserted at Level " << uLevel
        << " (Depth " << res.depth << ") with parent " << pId
        << ": DP = " << dpU << ", alpha(G) = " << currentAlpha;
    res.diagnostic = oss.str();
    return res;
}

IncrementalVerificationReport IncrementalIndependentSet::runIndependentVerification() const {
    IncrementalVerificationReport rep;
    rep.incrementalAlpha = currentAlpha;

    // Reconstruct IntervalGraph independently from intervals
    IntervalGraph g;
    std::string err;
    if (!g.initFromIntervals(intervals, err)) {
        rep.messages.push_back("Verification Error: Updated interval graph failed initialization: " + err);
        rep.isLeafInUpdatedTree = false;
        return rep;
    }

    DistanceTree t(g);
    std::string treeErr;
    if (!t.verifyTreeInvariants(treeErr)) {
        rep.messages.push_back("Verification Error: Updated distance tree failed invariants: " + treeErr);
        rep.isLeafInUpdatedTree = false;
        return rep;
    }

    // 1. Verify last inserted vertex is a leaf in t
    const auto& lastIv = intervals.back();
    const auto& lastNode = t.getNodeById(lastIv.id);
    rep.isLeafInUpdatedTree = (lastNode.childrenIds.empty());
    if (!rep.isLeafInUpdatedTree) {
        rep.messages.push_back("Error: Inserted vertex has children in updated distance tree.");
    }

    // 2. Verify all parent relationships and depths match recorded state
    rep.oldParentsPreserved = true;
    rep.oldDepthsPreserved = true;
    for (const auto& iv : intervals) {
        const auto& node = t.getNodeById(iv.id);
        if (node.parentId != idToParent.at(iv.id)) {
            rep.oldParentsPreserved = false;
            std::ostringstream oss;
            oss << "Parent mismatch for vertex " << iv.id << ": expected "
                << idToParent.at(iv.id) << ", actual in tree " << node.parentId;
            rep.messages.push_back(oss.str());
        }
        if (node.level != idToLevel.at(iv.id)) {
            rep.oldDepthsPreserved = false;
            std::ostringstream oss;
            oss << "Level mismatch for vertex " << iv.id << ": expected "
                << idToLevel.at(iv.id) << ", actual in tree " << node.level;
            rep.messages.push_back(oss.str());
        }
    }

    // 3. Parent matches brute-force
    auto bfParents = t.computeParentsBruteForce();
    rep.parentMatchesBruteForce = true;
    for (size_t i = 0; i < intervals.size(); ++i) {
        int vid = g.getIntervalBySortedIndex(static_cast<int>(i)).id;
        if (bfParents[i] != idToParent.at(vid)) {
            rep.parentMatchesBruteForce = false;
            break;
        }
    }

    // 4. Static Phase 6 DP comparison
    auto staticMIS = IndependentSet::computeExactDP(g, t);
    rep.staticAlpha = staticMIS.alpha;
    rep.alphaMatchesStatic = (rep.incrementalAlpha == rep.staticAlpha);

    rep.dpValueMatchesStatic = true;
    for (const auto& iv : intervals) {
        int incDP = idToDP.at(iv.id);
        int nodeLevel = idToLevel.at(iv.id);
        const auto& levelNodes = t.getLevelVerticesOneBased(nodeLevel);
        auto it = std::find(levelNodes.begin(), levelNodes.end(), iv.id);
        size_t posInLevel = std::distance(levelNodes.begin(), it);
        int statDP = staticMIS.dpValues[nodeLevel][posInLevel];
        if (incDP != statDP) {
            rep.dpValueMatchesStatic = false;
            std::ostringstream oss;
            oss << "DP value mismatch for vertex " << iv.id << ": incremental="
                << incDP << ", static=" << statDP;
            rep.messages.push_back(oss.str());
        }
    }

    // 5. Prefix-maximum verification
    rep.prefixMaxMatchesStatic = true;
    for (int i = 1; i <= h; ++i) {
        const auto& levelNodes = t.getLevelVerticesOneBased(i);
        for (size_t k = 0; k < levelNodes.size(); ++k) {
            int uid = levelNodes[k];
            const auto& iv = g.getIntervalById(uid);
            CanonicalKey key{iv.left, iv.right, iv.id};
            int treePref = levelTrees[i].queryPrefixMaxStrictlyLeft(key);

            int statPref = 0;
            for (size_t prev = 0; prev < k; ++prev) {
                statPref = std::max(statPref, staticMIS.dpValues[i][prev]);
            }
            if (treePref != statPref) {
                rep.prefixMaxMatchesStatic = false;
                std::ostringstream oss;
                oss << "Prefix max mismatch for vertex " << uid << " at level " << i
                    << ": tree=" << treePref << ", static=" << statPref;
                rep.messages.push_back(oss.str());
            }
        }
    }

    // 6. Brute-force MIS verification for small graphs
    rep.alphaMatchesBruteForce = true;
    rep.bruteForceAlpha = 0;
    if (g.getNumVertices() <= 25) {
        auto bfMIS = IndependentSet::computeBruteForceMIS(g);
        rep.bruteForceAlpha = static_cast<int>(bfMIS.size());
        rep.alphaMatchesBruteForce = (rep.incrementalAlpha == rep.bruteForceAlpha);
    }

    return rep;
}

std::string IncrementalIndependentSet::printStatus() const {
    std::ostringstream oss;
    oss << "Incremental MIS State (|V|=" << n << ", Levels=" << h << ", alpha(G)=" << currentAlpha << "):\n";
    for (int i = 1; i <= h; ++i) {
        oss << "  Level " << i << " (max DP=" << levelTrees[i].getMaxDP() << ", BEST[" << i << "]=" << BEST[i] << "):\n";
        for (const auto& iv : intervals) {
            if (idToLevel.at(iv.id) == i) {
                oss << "    Vertex " << iv.id << " [" << iv.left << ", " << iv.right << "]"
                    << " parent=" << idToParent.at(iv.id)
                    << " DP=" << idToDP.at(iv.id) << "\n";
            }
        }
    }
    return oss.str();
}

