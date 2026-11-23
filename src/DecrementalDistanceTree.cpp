#include "DecrementalDistanceTree.h"
#include "StructuralAnalysis.h"
#include <queue>
#include <algorithm>
#include <iostream>

DecrementalDistanceTree::DecrementalDistanceTree()
    : rootId(-1), maxDepth(0), currentAlpha(0) {}

bool DecrementalDistanceTree::initialize(const IntervalGraph& graph, const DistanceTree& tree, std::string& errorMsg) {
    if (graph.getNumVertices() == 0) {
        errorMsg = "Cannot initialize DecrementalDistanceTree with empty graph.";
        return false;
    }

    rootId = tree.getRootId();
    maxDepth = tree.getMaxDepth();
    nodes.clear();
    currentIntervals = graph.getSortedIntervals();

    int numLevels = maxDepth + 1;
    levelHead.assign(numLevels + 1, -1);
    levelTail.assign(numLevels + 1, -1);
    levelCount.assign(numLevels + 1, 0);

    // 1. Initialize DecrementalNode for each vertex
    for (const auto& iv : currentIntervals) {
        const auto& tNode = tree.getNodeById(iv.id);
        DecrementalNode dNode;
        dNode.id = iv.id;
        dNode.interval = iv;
        dNode.level = tNode.level;
        dNode.depth = tNode.depth;
        dNode.parentId = tNode.parentId;

        // Boundary links for children
        const auto& ch = tNode.childrenIds;
        if (ch.empty()) {
            dNode.childHead = -1;
            dNode.childTail = -1;
            dNode.childCount = 0;
        } else {
            dNode.childHead = ch.front();
            dNode.childTail = ch.back();
            dNode.childCount = static_cast<int>(ch.size());
        }

        dNode.prevSibling = -1;
        dNode.nextSibling = -1;
        dNode.prevLevelSibling = -1;
        dNode.nextLevelSibling = -1;

        nodes[iv.id] = dNode;
    }

    // 2. Link siblings within each parent's ordered child set
    for (const auto& kv : nodes) {
        const auto& tNode = tree.getNodeById(kv.first);
        const auto& ch = tNode.childrenIds;
        for (size_t i = 0; i < ch.size(); ++i) {
            int cId = ch[i];
            if (i > 0) {
                nodes[cId].prevSibling = ch[i - 1];
            }
            if (i + 1 < ch.size()) {
                nodes[cId].nextSibling = ch[i + 1];
            }
        }
    }

    // 3. Link vertices within each level in canonical order
    for (int l = 1; l <= numLevels; ++l) {
        const auto& lvlVerts = tree.getLevelVerticesOneBased(l);
        levelCount[l] = static_cast<int>(lvlVerts.size());
        if (!lvlVerts.empty()) {
            levelHead[l] = lvlVerts.front();
            levelTail[l] = lvlVerts.back();
            for (size_t i = 0; i < lvlVerts.size(); ++i) {
                int vId = lvlVerts[i];
                if (i > 0) {
                    nodes[vId].prevLevelSibling = lvlVerts[i - 1];
                }
                if (i + 1 < lvlVerts.size()) {
                    nodes[vId].nextLevelSibling = lvlVerts[i + 1];
                }
            }
        }
    }

    // 4. Build balanced parent search tree
    rebuildParentSearchTree();

    // 5. Compute initial static MIS
    auto misRes = IndependentSet::computeExactDP(graph, tree);
    currentAlpha = misRes.alpha;

    return true;
}

void DecrementalDistanceTree::rebuildParentSearchTree() {
    parentSearchTree.clear();
    for (const auto& iv : currentIntervals) {
        parentSearchTree.insert(iv);
    }
}

void DecrementalDistanceTree::recomputeMIS() {
    if (currentIntervals.empty()) {
        currentAlpha = 0;
        return;
    }
    IntervalGraph updatedGraph;
    std::string err;
    if (updatedGraph.initFromIntervals(currentIntervals, err)) {
        DistanceTree staticTree(updatedGraph);
        auto misRes = IndependentSet::computeExactDP(updatedGraph, staticTree);
        currentAlpha = misRes.alpha;
    }
}

bool DecrementalDistanceTree::validateDeletion(int vertexId, std::string& diagnosticMsg) const {
    auto it = nodes.find(vertexId);
    if (it == nodes.end()) {
        diagnosticMsg = "Rejected: Vertex ID " + std::to_string(vertexId) + " does not exist in graph.";
        return false;
    }

    // Root deletion check (root is the unique vertex at level 1 / depth 0)
    if (vertexId == rootId || it->second.level == 1 || it->second.depth == 0) {
        diagnosticMsg = "Rejected: Root deletion (vertex " + std::to_string(vertexId) + 
                        ") is not supported by Theorem 3.16 decremental child relocation. "
                        "The root has no same-level peers to adopt children.";
        return false;
    }

    // Non-empty graph check
    if (currentIntervals.size() <= 1) {
        diagnosticMsg = "Rejected: Deletion of vertex " + std::to_string(vertexId) + " would leave graph empty.";
        return false;
    }

    // Build candidate interval set for connectivity and properness verification
    std::vector<Interval> candidateIntervals;
    candidateIntervals.reserve(currentIntervals.size() - 1);
    for (const auto& iv : currentIntervals) {
        if (iv.id != vertexId) {
            candidateIntervals.push_back(iv);
        }
    }

    // Connectedness check: deletion must not disconnect the graph
    auto connRes = IntervalGraph::validateConnectedness(candidateIntervals);
    if (!connRes.valid) {
        diagnosticMsg = "Rejected: Deletion disconnects the graph: " + connRes.message;
        return false;
    }

    // Properness check: deletion must preserve properness
    auto propRes = IntervalGraph::validateProperness(candidateIntervals);
    if (!propRes.valid) {
        diagnosticMsg = "Rejected: Deletion violates properness: " + propRes.message;
        return false;
    }

    // Children relocation condition (Observation 3.10.3):
    // If u has children, an immediate right vertex v in the same level must exist to adopt them
    const auto& uNode = it->second;
    if (uNode.childHead != -1 && uNode.nextLevelSibling == -1) {
        diagnosticMsg = "Rejected: Vertex " + std::to_string(vertexId) + 
                        " has children but no immediate right vertex in the same level (Observation 3.10.3 requirement).";
        return false;
    }

    diagnosticMsg = "Valid deletion";
    return true;
}

DeletionResult DecrementalDistanceTree::deleteVertex(int vertexId) {
    DeletionResult res;
    res.deletedId = vertexId;
    res.parentId = -1;
    res.immediateRightId = -1;
    res.numChildrenRelocated = 0;
    res.oldAlpha = currentAlpha;
    res.newAlpha = currentAlpha;
    res.alphaChanged = false;

    std::string diag;
    if (!validateDeletion(vertexId, diag)) {
        res.success = false;
        res.diagnostic = diag;
        return res;
    }

    const auto& uNode = nodes.at(vertexId);
    int p = uNode.parentId;
    int v = uNode.nextLevelSibling;
    int u_left = uNode.childHead;
    int u_right = uNode.childTail;
    int numChildren = uNode.childCount;

    res.parentId = p;
    res.immediateRightId = v;
    res.numChildrenRelocated = numChildren;

    // 1. Relocate children of u under v (Observation 3.10.3 & Theorem 3.16)
    // Using O(1) boundary link modifications
    if (u_left != -1) {
        DecrementalNode& vNode = nodes.at(v);
        int v_left = vNode.childHead;

        if (v_left == -1) {
            // v has no existing children: C(v) becomes C(u)
            vNode.childHead = u_left;
            vNode.childTail = u_right;
            vNode.childCount = numChildren;
        } else {
            // Both u and v have children: splice C(u) strictly to the left of C(v)
            nodes.at(u_right).nextSibling = v_left;
            nodes.at(v_left).prevSibling = u_right;
            vNode.childHead = u_left;
            // vNode.childTail remains unchanged
            vNode.childCount += numChildren;
        }
    }

    // 2. Remove u from parent p's child set (O(1) boundary link modifications)
    DecrementalNode& pNode = nodes.at(p);
    int prevSib = uNode.prevSibling;
    int nextSib = uNode.nextSibling;

    if (prevSib != -1) {
        nodes.at(prevSib).nextSibling = nextSib;
    } else {
        // u was leftmost child of p
        pNode.childHead = nextSib;
    }

    if (nextSib != -1) {
        nodes.at(nextSib).prevSibling = prevSib;
    } else {
        // u was rightmost child of p (boundary case in thesis)
        pNode.childTail = prevSib;
    }
    pNode.childCount--;

    // 3. Remove u from its level (O(1) list splice)
    int lvl = uNode.level;
    int prevLvl = uNode.prevLevelSibling;
    int nextLvl = uNode.nextLevelSibling;

    if (prevLvl != -1) {
        nodes.at(prevLvl).nextLevelSibling = nextLvl;
    } else {
        levelHead[lvl] = nextLvl;
    }

    if (nextLvl != -1) {
        nodes.at(nextLvl).prevLevelSibling = prevLvl;
    } else {
        levelTail[lvl] = prevLvl;
    }
    levelCount[lvl]--;

    // 4. Remove u from node map and current intervals
    nodes.erase(vertexId);
    currentIntervals.erase(
        std::remove_if(currentIntervals.begin(), currentIntervals.end(),
                       [vertexId](const Interval& iv) { return iv.id == vertexId; }),
        currentIntervals.end());

    // 5. Update maxDepth if highest level became empty
    while (maxDepth >= 0 && (maxDepth + 1 >= static_cast<int>(levelCount.size()) || levelCount[maxDepth + 1] == 0)) {
        maxDepth--;
    }

    // 6. Update balanced parent search tree
    rebuildParentSearchTree();

    // 7. Recompute static MIS
    recomputeMIS();
    res.newAlpha = currentAlpha;
    res.alphaChanged = (res.oldAlpha != res.newAlpha);

    std::ostringstream oss;
    oss << "Success: Vertex " << vertexId << " deleted.";
    if (numChildren > 0) {
        oss << " " << numChildren << " children relocated under immediate right vertex " << v << ".";
    } else {
        oss << " Vertex had 0 children (leaf).";
    }
    res.success = true;
    res.diagnostic = oss.str();
    return res;
}

std::vector<int> DecrementalDistanceTree::getChildren(int id) const {
    std::vector<int> children;
    auto it = nodes.find(id);
    if (it == nodes.end()) return children;

    int curr = it->second.childHead;
    while (curr != -1) {
        children.push_back(curr);
        auto cit = nodes.find(curr);
        if (cit != nodes.end()) {
            curr = cit->second.nextSibling;
        } else {
            break;
        }
    }
    return children;
}

std::vector<int> DecrementalDistanceTree::getLevelVertices(int level) const {
    std::vector<int> lvl;
    if (level < 1 || level >= static_cast<int>(levelHead.size())) return lvl;

    int curr = levelHead[level];
    while (curr != -1) {
        lvl.push_back(curr);
        auto it = nodes.find(curr);
        if (it != nodes.end()) {
            curr = it->second.nextLevelSibling;
        } else {
            break;
        }
    }
    return lvl;
}

void DecrementalDistanceTree::getLogicalTree(std::unordered_map<int, int>& outParents,
                                            std::unordered_map<int, int>& outDepths,
                                            std::unordered_map<int, std::vector<int>>& outChildren) const {
    outParents.clear();
    outDepths.clear();
    outChildren.clear();

    if (nodes.empty() || rootId == -1) return;

    // Traverse starting from rootId (root is its own parent in DistanceTree convention)
    outParents[rootId] = rootId;
    outDepths[rootId] = 0;

    std::queue<int> q;
    q.push(rootId);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        int currDepth = outDepths[u];
        auto chList = getChildren(u);
        outChildren[u] = chList;

        for (int c : chList) {
            outParents[c] = u;
            outDepths[c] = currDepth + 1;
            q.push(c);
        }
    }
}

DecrementalVerificationReport DecrementalDistanceTree::runIndependentVerification() const {
    DecrementalVerificationReport rep;
    rep.success = true;
    rep.treeInvariantsPassed = true;
    rep.parentsMatchStaticRebuild = true;
    rep.depthsMatchStaticRebuild = true;
    rep.childrenMatchStaticRebuild = true;
    rep.rootMatchesStaticRebuild = true;
    rep.levelsMatchStaticRebuild = true;
    rep.structuralLemmasPassed = true;
    rep.misMatchesStaticDP = true;
    rep.misMatchesBruteForce = true;
    rep.decrementalAlpha = currentAlpha;
    rep.staticAlpha = 0;
    rep.bruteForceAlpha = 0;

    if (currentIntervals.empty()) {
        rep.messages.push_back("Empty interval graph");
        return rep;
    }

    // 1. Independent static rebuild from scratch
    IntervalGraph staticGraph;
    std::string err;
    if (!staticGraph.initFromIntervals(currentIntervals, err)) {
        rep.success = false;
        rep.treeInvariantsPassed = false;
        rep.messages.push_back("Failed to build IntervalGraph from remaining intervals: " + err);
        return rep;
    }

    DistanceTree staticTree(staticGraph);
    if (!staticTree.verifyTreeInvariants(err)) {
        rep.success = false;
        rep.treeInvariantsPassed = false;
        rep.messages.push_back("Static tree invariants failed: " + err);
        return rep;
    }

    // 2. Check root identity
    if (rootId != staticTree.getRootId()) {
        rep.rootMatchesStaticRebuild = false;
        rep.messages.push_back("Root mismatch: decremental " + std::to_string(rootId) + 
                               " vs static " + std::to_string(staticTree.getRootId()));
    }

    // 3. Reconstruct logical tree from linked child sets
    std::unordered_map<int, int> decParents;
    std::unordered_map<int, int> decDepths;
    std::unordered_map<int, std::vector<int>> decChildren;
    getLogicalTree(decParents, decDepths, decChildren);

    // 4. Compare parent, depth, and children for every vertex
    for (const auto& iv : currentIntervals) {
        int vId = iv.id;
        const auto& sNode = staticTree.getNodeById(vId);

        // Parent check
        int expectedParent = sNode.parentId;
        int actualParent = decParents.count(vId) ? decParents.at(vId) : -999;
        if (actualParent != expectedParent) {
            rep.parentsMatchStaticRebuild = false;
            rep.messages.push_back("Parent mismatch for vertex " + std::to_string(vId) + 
                                   ": decremental " + std::to_string(actualParent) + 
                                   " vs static " + std::to_string(expectedParent));
        }

        // Depth check
        int expectedDepth = sNode.depth;
        int actualDepth = decDepths.count(vId) ? decDepths.at(vId) : -999;
        if (actualDepth != expectedDepth) {
            rep.depthsMatchStaticRebuild = false;
            rep.messages.push_back("Depth mismatch for vertex " + std::to_string(vId) + 
                                   ": decremental " + std::to_string(actualDepth) + 
                                   " vs static " + std::to_string(expectedDepth));
        }

        // Children check
        const auto& expectedChildren = sNode.childrenIds;
        auto actualChildren = decChildren.count(vId) ? decChildren.at(vId) : std::vector<int>{};
        if (actualChildren != expectedChildren) {
            rep.childrenMatchStaticRebuild = false;
            rep.messages.push_back("Children mismatch for vertex " + std::to_string(vId));
        }
    }

    // 5. Compare level sets
    int staticNumLevels = staticTree.getNumLevels();
    if (getNumLevels() != staticNumLevels) {
        rep.levelsMatchStaticRebuild = false;
        rep.messages.push_back("Level count mismatch: decremental " + std::to_string(getNumLevels()) + 
                               " vs static " + std::to_string(staticNumLevels));
    }
    for (int l = 1; l <= staticNumLevels; ++l) {
        auto decLvl = getLevelVertices(l);
        const auto& sLvl = staticTree.getLevelVerticesOneBased(l);
        if (decLvl != sLvl) {
            rep.levelsMatchStaticRebuild = false;
            rep.messages.push_back("Level " + std::to_string(l) + " vertices mismatch");
        }
    }

    // 6. Check structural lemmas on rebuilt graph
    auto sRep = StructuralAnalysis::runFullAudit(staticGraph, staticTree);
    if (!sRep.allPassed()) {
        rep.structuralLemmasPassed = false;
        rep.messages.push_back("Structural lemmas failed on updated graph");
    }

    // 7. Check MIS against static DP
    auto misExact = IndependentSet::computeExactDP(staticGraph, staticTree);
    rep.staticAlpha = misExact.alpha;
    if (currentAlpha != misExact.alpha) {
        rep.misMatchesStaticDP = false;
        rep.messages.push_back("MIS mismatch with static DP: decremental " + std::to_string(currentAlpha) + 
                               " vs static " + std::to_string(misExact.alpha));
    }

    // 8. Check MIS against brute-force on small graphs
    if (staticGraph.getNumVertices() <= 20) {
        auto bfSet = IndependentSet::computeBruteForceMIS(staticGraph);
        rep.bruteForceAlpha = static_cast<int>(bfSet.size());
        if (currentAlpha != rep.bruteForceAlpha) {
            rep.misMatchesBruteForce = false;
            rep.messages.push_back("MIS mismatch with brute force: decremental " + std::to_string(currentAlpha) + 
                                   " vs brute-force " + std::to_string(rep.bruteForceAlpha));
        }
    } else {
        rep.bruteForceAlpha = rep.staticAlpha;
    }

    rep.success = rep.allPassed();
    return rep;
}

DecrementalStateSnapshot DecrementalDistanceTree::getSnapshot() const {
    DecrementalStateSnapshot snap;
    snap.rootId = rootId;
    snap.maxDepth = maxDepth;
    snap.currentAlpha = currentAlpha;
    snap.intervals = currentIntervals;
    snap.nodes = nodes;
    snap.levelHead = levelHead;
    snap.levelTail = levelTail;
    snap.levelCount = levelCount;
    return snap;
}

bool DecrementalDistanceTree::matchesSnapshot(const DecrementalStateSnapshot& snap) const {
    if (rootId != snap.rootId || maxDepth != snap.maxDepth || currentAlpha != snap.currentAlpha) {
        return false;
    }
    if (currentIntervals.size() != snap.intervals.size()) {
        return false;
    }
    for (size_t i = 0; i < currentIntervals.size(); ++i) {
        if (currentIntervals[i] != snap.intervals[i]) {
            return false;
        }
    }
    if (nodes.size() != snap.nodes.size()) {
        return false;
    }
    for (const auto& kv : nodes) {
        auto it = snap.nodes.find(kv.first);
        if (it == snap.nodes.end()) return false;
        if (kv.second != it->second) return false;
    }
    if (levelHead != snap.levelHead || levelTail != snap.levelTail || levelCount != snap.levelCount) {
        return false;
    }
    return true;
}
