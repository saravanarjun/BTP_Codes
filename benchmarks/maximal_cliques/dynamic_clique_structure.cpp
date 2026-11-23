#include "dynamic_clique_structure.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>

// ============================================================================
// DynamicTheoremCliques (Algorithm A Dynamic Maintenance)
// ============================================================================

DynamicTheoremCliques::DynamicTheoremCliques()
    : rootId(-1), maxDepth(0) {}

bool DynamicTheoremCliques::initialize(const IntervalGraph& graph, const DistanceTree& tree) {
    nodes.clear();
    parentToRightmostChild.clear();
    intervals = graph.getSortedIntervals();
    rootId = tree.getRootId();
    maxDepth = tree.getMaxDepth();

    int h = maxDepth + 1;
    levelHead.assign(h + 2, -1);
    levelTail.assign(h + 2, -1);
    levelCount.assign(h + 2, 0);

    // 1. Initialize all nodes
    for (const auto& iv : intervals) {
        DynamicTreeNode node;
        node.id = iv.id;
        node.interval = iv;
        const auto& tNode = tree.getNodeById(iv.id);
        node.level = tNode.level;
        node.depth = tNode.depth;
        node.parentId = tNode.parentId;
        node.childHead = -1;
        node.childTail = -1;
        node.childCount = 0;
        node.prevSibling = -1;
        node.nextSibling = -1;
        node.prevLevelSibling = -1;
        node.nextLevelSibling = -1;
        nodes[iv.id] = node;
    }

    // 2. Build level lists
    for (int lvl = 1; lvl <= h; ++lvl) {
        const auto& lvlVerts = tree.getLevelVerticesOneBased(lvl);
        levelCount[lvl] = static_cast<int>(lvlVerts.size());
        if (!lvlVerts.empty()) {
            levelHead[lvl] = lvlVerts.front();
            levelTail[lvl] = lvlVerts.back();
            for (size_t i = 0; i < lvlVerts.size(); ++i) {
                int cur = lvlVerts[i];
                if (i > 0) nodes[cur].prevLevelSibling = lvlVerts[i - 1];
                if (i + 1 < lvlVerts.size()) nodes[cur].nextLevelSibling = lvlVerts[i + 1];
            }
        }
    }

    // 3. Build ordered child lists and populate compact descriptors (Lemma 3.15)
    for (const auto& iv : intervals) {
        int uId = iv.id;
        const auto& tNode = tree.getNodeById(uId);
        const auto& children = tNode.childrenIds;
        if (!children.empty()) {
            nodes[uId].childHead = children.front();
            nodes[uId].childTail = children.back();
            nodes[uId].childCount = static_cast<int>(children.size());

            for (size_t c = 0; c < children.size(); ++c) {
                int curChild = children[c];
                if (c > 0) nodes[curChild].prevSibling = children[c - 1];
                if (c + 1 < children.size()) nodes[curChild].nextSibling = children[c + 1];
            }

            // Lemma 3.15: non-leaf parent uId defines compact maximal clique [uId, children.back()]
            parentToRightmostChild[uId] = children.back();
        }
    }

    return true;
}

int DynamicTheoremCliques::findParentLogN(double left) const {
    // In proper interval graphs ordered canonically by left endpoint,
    // right endpoints are monotonically non-decreasing.
    // The parent is the first vertex in sorted order whose right endpoint >= left.
    auto it = std::lower_bound(intervals.begin(), intervals.end(), left,
                               [](const Interval& iv, double val) {
                                   return iv.right < val;
                               });
    return (it != intervals.end()) ? it->id : -1;
}

DynamicUpdateStepTelemetry DynamicTheoremCliques::insertLeaf(const Interval& newIv) {
    DynamicUpdateStepTelemetry tel;
    tel.opType = DynamicOpType::INSERTION;
    tel.targetVertexId = newIv.id;
    tel.targetInterval = newIv;
    tel.nBefore = getNumVertices();
    tel.qBefore = getNumCliques();

    auto t0 = std::chrono::steady_clock::now();

    // 1. Locate parent p in O(log n)
    int p = findParentLogN(newIv.left);
    if (p == -1) {
        tel.status = BenchmarkStatus::UNSUPPORTED_UPDATE;
        tel.diagnostic = "No intersecting parent candidate found.";
        return tel;
    }

    auto t1 = std::chrono::steady_clock::now();

    // 2. Perform distance-tree update (Section 3.10.2)
    DynamicTreeNode& pNode = nodes.at(p);
    DynamicTreeNode uNode;
    uNode.id = newIv.id;
    uNode.interval = newIv;
    uNode.level = pNode.level + 1;
    uNode.depth = pNode.depth + 1;
    uNode.parentId = p;
    uNode.childHead = -1;
    uNode.childTail = -1;
    uNode.childCount = 0;
    uNode.nextSibling = -1;
    uNode.nextLevelSibling = -1;

    // Splice into p's ordered child set
    if (pNode.childHead == -1) {
        pNode.childHead = newIv.id;
        pNode.childTail = newIv.id;
        pNode.childCount = 1;
        uNode.prevSibling = -1;
    } else {
        int oldTail = pNode.childTail;
        nodes.at(oldTail).nextSibling = newIv.id;
        uNode.prevSibling = oldTail;
        pNode.childTail = newIv.id;
        pNode.childCount++;
    }

    // Splice into level list
    int lvl = uNode.level;
    if (lvl >= static_cast<int>(levelHead.size())) {
        levelHead.resize(lvl + 2, -1);
        levelTail.resize(lvl + 2, -1);
        levelCount.resize(lvl + 2, 0);
    }
    if (levelHead[lvl] == -1) {
        levelHead[lvl] = newIv.id;
        levelTail[lvl] = newIv.id;
        levelCount[lvl] = 1;
        uNode.prevLevelSibling = -1;
    } else {
        int oldLevelTail = levelTail[lvl];
        nodes.at(oldLevelTail).nextLevelSibling = newIv.id;
        uNode.prevLevelSibling = oldLevelTail;
        levelTail[lvl] = newIv.id;
        levelCount[lvl]++;
    }
    if (uNode.depth > maxDepth) {
        maxDepth = uNode.depth;
    }

    nodes[newIv.id] = uNode;
    intervals.push_back(newIv);

    // 3. Update maximal clique descriptors (Lemma 3.15 & Section 3.10.2)
    // O(1) map operations replacing O(q log q) map copy and linear scan
    int descsAdded = 0, descsRemoved = 0, descsModified = 0, descsPreserved = 0;
    auto itP = parentToRightmostChild.find(p);
    if (itP == parentToRightmostChild.end()) {
        descsAdded = 1;
        parentToRightmostChild[p] = newIv.id;
    } else {
        descsModified = 1;
        itP->second = newIv.id;
    }
    descsPreserved = static_cast<int>(parentToRightmostChild.size()) - descsAdded - descsModified;

    auto t2 = std::chrono::steady_clock::now();

    tel.updateApplicationMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    tel.algorithmUpdateMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    tel.totalLatencyMs = std::chrono::duration<double, std::milli>(t2 - t0).count();
    tel.descriptorsAdded = descsAdded;
    tel.descriptorsRemoved = descsRemoved;
    tel.descriptorsModified = descsModified;
    tel.descriptorsPreserved = descsPreserved;
    tel.nAfter = getNumVertices();
    tel.qAfter = getNumCliques();
    tel.status = BenchmarkStatus::SUCCESS;

    return tel;
}

DynamicUpdateStepTelemetry DynamicTheoremCliques::deleteVertex(int vertexId) {
    DynamicUpdateStepTelemetry tel;
    tel.opType = DynamicOpType::DELETION;
    tel.targetVertexId = vertexId;
    tel.nBefore = getNumVertices();
    tel.qBefore = getNumCliques();

    auto it = nodes.find(vertexId);
    if (it == nodes.end()) {
        tel.status = BenchmarkStatus::INVALID_INPUT;
        tel.diagnostic = "Vertex ID not found in distance tree.";
        return tel;
    }

    const DynamicTreeNode& uNode = it->second;
    tel.targetInterval = uNode.interval;

    if (vertexId == rootId) {
        tel.status = BenchmarkStatus::UNSUPPORTED_UPDATE;
        tel.diagnostic = "Root deletion is unsupported under Section 3.10.3.";
        return tel;
    }

    int v = uNode.nextLevelSibling;
    if (uNode.childHead != -1 && v == -1) {
        tel.status = BenchmarkStatus::UNSUPPORTED_UPDATE;
        tel.diagnostic = "Vertex has children but no immediate right sibling in level (Observation 3.10.3 requirement).";
        return tel;
    }

    auto t0 = std::chrono::steady_clock::now();

    int p = uNode.parentId;
    int u_left = uNode.childHead;
    int u_right = uNode.childTail;
    int numChildren = uNode.childCount;

    // 1. Relocate children under v (Theorem 3.16)
    if (u_left != -1) {
        DynamicTreeNode& vNode = nodes.at(v);
        int v_left = vNode.childHead;
        if (v_left == -1) {
            vNode.childHead = u_left;
            vNode.childTail = u_right;
            vNode.childCount = numChildren;
        } else {
            nodes.at(u_right).nextSibling = v_left;
            nodes.at(v_left).prevSibling = u_right;
            vNode.childHead = u_left;
            vNode.childCount += numChildren;
        }

        // Update parentId of all relocated children from u to v
        int currChild = u_left;
        while (currChild != -1) {
            nodes.at(currChild).parentId = v;
            if (currChild == u_right) break;
            currChild = nodes.at(currChild).nextSibling;
        }
    }

    // 2. Remove u from parent p's child set
    DynamicTreeNode& pNode = nodes.at(p);
    int prevSib = uNode.prevSibling;
    int nextSib = uNode.nextSibling;

    if (prevSib != -1) nodes.at(prevSib).nextSibling = nextSib;
    else pNode.childHead = nextSib;

    if (nextSib != -1) nodes.at(nextSib).prevSibling = prevSib;
    else pNode.childTail = prevSib;

    pNode.childCount--;

    // 3. Remove u from its level list
    int lvl = uNode.level;
    int prevLvl = uNode.prevLevelSibling;
    int nextLvl = uNode.nextLevelSibling;

    if (prevLvl != -1) nodes.at(prevLvl).nextLevelSibling = nextLvl;
    else levelHead[lvl] = nextLvl;

    if (nextLvl != -1) nodes.at(nextLvl).prevLevelSibling = prevLvl;
    else levelTail[lvl] = prevLvl;

    levelCount[lvl]--;

    // 4. Update maximal clique descriptors (Lemma 3.15 & Section 3.10.3)
    // O(1) map operations replacing O(q log q) map copy and linear scan
    int descsAdded = 0, descsRemoved = 0, descsModified = 0, descsPreserved = 0;

    bool u_was_parent = (parentToRightmostChild.find(vertexId) != parentToRightmostChild.end());
    int c_u = u_was_parent ? parentToRightmostChild[vertexId] : -1;

    bool p_was_parent = (p != -1 && parentToRightmostChild.find(p) != parentToRightmostChild.end());
    bool u_was_p_rmc = (p_was_parent && parentToRightmostChild[p] == vertexId);

    // (A) If u was in descriptors:
    if (u_was_parent) {
        parentToRightmostChild.erase(vertexId);
        descsRemoved++;

        if (u_left != -1 && v != -1) {
            if (parentToRightmostChild.find(v) == parentToRightmostChild.end()) {
                parentToRightmostChild[v] = c_u;
                descsAdded++;
            }
        }
    }

    // (B) If u was the rightmost child of its parent p:
    if (p_was_parent && u_was_p_rmc) {
        if (pNode.childTail != -1) {
            parentToRightmostChild[p] = pNode.childTail;
            descsModified++;
        } else {
            parentToRightmostChild.erase(p);
            descsRemoved++;
        }
    }

    descsPreserved = static_cast<int>(parentToRightmostChild.size()) - descsAdded - descsModified;

    nodes.erase(vertexId);
    intervals.erase(
        std::remove_if(intervals.begin(), intervals.end(),
                       [vertexId](const Interval& iv) { return iv.id == vertexId; }),
        intervals.end());

    auto t1 = std::chrono::steady_clock::now();

    tel.updateApplicationMs = 0.0;
    tel.algorithmUpdateMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    tel.totalLatencyMs = tel.algorithmUpdateMs;
    tel.descriptorsAdded = descsAdded;
    tel.descriptorsRemoved = descsRemoved;
    tel.descriptorsModified = descsModified;
    tel.descriptorsPreserved = descsPreserved;
    tel.nAfter = getNumVertices();
    tel.qAfter = getNumCliques();
    tel.status = BenchmarkStatus::SUCCESS;

    return tel;
}

std::vector<std::vector<int>> DynamicTheoremCliques::materializeCurrentCliques() const {
    std::vector<std::vector<int>> cliques;
    cliques.reserve(parentToRightmostChild.size());

    std::unordered_map<int, double> idToLeft;
    for (const auto& iv : intervals) {
        idToLeft[iv.id] = iv.left;
    }

    for (const auto& kv : parentToRightmostChild) {
        int pId = kv.first;
        int cId = kv.second;
        double lp = idToLeft.at(pId);
        double lc = idToLeft.at(cId);

        std::vector<int> clique;
        for (const auto& iv : intervals) {
            if (iv.left >= lp && iv.left <= lc) {
                clique.push_back(iv.id);
            }
        }
        std::sort(clique.begin(), clique.end());
        cliques.push_back(clique);
    }
    std::sort(cliques.begin(), cliques.end());
    return cliques;
}

int64_t DynamicTheoremCliques::getTotalMemberships() const {
    int64_t total = 0;
    std::unordered_map<int, double> idToLeft;
    for (const auto& iv : intervals) {
        idToLeft[iv.id] = iv.left;
    }
    for (const auto& kv : parentToRightmostChild) {
        double lp = idToLeft.at(kv.first);
        double lc = idToLeft.at(kv.second);
        for (const auto& iv : intervals) {
            if (iv.left >= lp && iv.left <= lc) {
                total++;
            }
        }
    }
    return total;
}

std::vector<CompactDescriptor> DynamicTheoremCliques::getDescriptors() const {
    std::vector<CompactDescriptor> descs;
    descs.reserve(parentToRightmostChild.size());
    std::unordered_map<int, double> idToLeft;
    for (const auto& iv : intervals) {
        idToLeft[iv.id] = iv.left;
    }
    for (const auto& kv : parentToRightmostChild) {
        descs.push_back(CompactDescriptor{
            kv.first, kv.second, idToLeft.at(kv.first), idToLeft.at(kv.second)
        });
    }
    return descs;
}

std::vector<Interval> DynamicTheoremCliques::getCurrentIntervals() const {
    return intervals;
}

IntervalGraph DynamicTheoremCliques::exportCurrentGraph() const {
    IntervalGraph g;
    std::string err;
    g.initFromIntervals(intervals, err);
    return g;
}

int DynamicTheoremCliques::getParentId(int vId) const {
    auto it = nodes.find(vId);
    return (it != nodes.end()) ? it->second.parentId : -1;
}

// ============================================================================
// DynamicBronKerbosch (Algorithm B: BK-Full-Recomputation)
// ============================================================================

DynamicBronKerbosch::DynamicBronKerbosch() {}

void DynamicBronKerbosch::initialize(const IntervalGraph& graph) {
    currentGraph = graph;
    currentCliques = BronKerboschAdapter::runApprovedPhase5(currentGraph);
}

DynamicUpdateStepTelemetry DynamicBronKerbosch::insertLeaf(const Interval& newIv, double timeoutMs) {
    DynamicUpdateStepTelemetry tel;
    tel.opType = DynamicOpType::INSERTION;
    tel.targetVertexId = newIv.id;
    tel.targetInterval = newIv;
    tel.nBefore = currentGraph.getNumVertices();
    tel.qBefore = static_cast<int>(currentCliques.size());

    auto t0 = std::chrono::steady_clock::now();

    auto ivs = currentGraph.getSortedIntervals();
    ivs.push_back(newIv);
    std::string err;
    currentGraph.initFromIntervals(ivs, err);

    auto t1 = std::chrono::steady_clock::now();

    auto bkRes = BronKerboschAdapter::runPivotedWithTimeout(currentGraph, timeoutMs);

    auto t2 = std::chrono::steady_clock::now();

    tel.updateApplicationMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    tel.algorithmUpdateMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    tel.totalLatencyMs = std::chrono::duration<double, std::milli>(t2 - t0).count();
    tel.nAfter = currentGraph.getNumVertices();

    if (bkRes.timedOut) {
        tel.status = BenchmarkStatus::TIMEOUT;
        tel.qAfter = 0;
    } else {
        currentCliques = bkRes.cliques;
        tel.qAfter = static_cast<int>(currentCliques.size());
        tel.status = BenchmarkStatus::SUCCESS;
    }

    return tel;
}

DynamicUpdateStepTelemetry DynamicBronKerbosch::deleteVertex(int vertexId, double timeoutMs) {
    DynamicUpdateStepTelemetry tel;
    tel.opType = DynamicOpType::DELETION;
    tel.targetVertexId = vertexId;
    tel.nBefore = currentGraph.getNumVertices();
    tel.qBefore = static_cast<int>(currentCliques.size());

    auto t0 = std::chrono::steady_clock::now();

    auto ivs = currentGraph.getSortedIntervals();
    ivs.erase(
        std::remove_if(ivs.begin(), ivs.end(),
                       [vertexId](const Interval& iv) { return iv.id == vertexId; }),
        ivs.end());
    std::string err;
    currentGraph.initFromIntervals(ivs, err);

    auto t1 = std::chrono::steady_clock::now();

    auto bkRes = BronKerboschAdapter::runPivotedWithTimeout(currentGraph, timeoutMs);

    auto t2 = std::chrono::steady_clock::now();

    tel.updateApplicationMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    tel.algorithmUpdateMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    tel.totalLatencyMs = std::chrono::duration<double, std::milli>(t2 - t0).count();
    tel.nAfter = currentGraph.getNumVertices();

    if (bkRes.timedOut) {
        tel.status = BenchmarkStatus::TIMEOUT;
        tel.qAfter = 0;
    } else {
        currentCliques = bkRes.cliques;
        tel.qAfter = static_cast<int>(currentCliques.size());
        tel.status = BenchmarkStatus::SUCCESS;
    }

    return tel;
}

int64_t DynamicBronKerbosch::getTotalMemberships() const {
    int64_t total = 0;
    for (const auto& c : currentCliques) {
        total += c.size();
    }
    return total;
}

// ============================================================================
// DynamicSweepLine (Algorithm C: Sweep-Full-Recomputation)
// ============================================================================

DynamicSweepLine::DynamicSweepLine() {}

void DynamicSweepLine::initialize(const IntervalGraph& graph) {
    currentGraph = graph;
    currentCliques = StandardIntervalCliques::enumerateMaximalCliques(currentGraph);
}

DynamicUpdateStepTelemetry DynamicSweepLine::insertLeaf(const Interval& newIv) {
    DynamicUpdateStepTelemetry tel;
    tel.opType = DynamicOpType::INSERTION;
    tel.targetVertexId = newIv.id;
    tel.targetInterval = newIv;
    tel.nBefore = currentGraph.getNumVertices();
    tel.qBefore = static_cast<int>(currentCliques.size());

    auto t0 = std::chrono::steady_clock::now();

    auto ivs = currentGraph.getSortedIntervals();
    ivs.push_back(newIv);
    std::string err;
    currentGraph.initFromIntervals(ivs, err);

    auto t1 = std::chrono::steady_clock::now();

    currentCliques = StandardIntervalCliques::enumerateMaximalCliques(currentGraph);

    auto t2 = std::chrono::steady_clock::now();

    tel.updateApplicationMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    tel.algorithmUpdateMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    tel.totalLatencyMs = std::chrono::duration<double, std::milli>(t2 - t0).count();
    tel.nAfter = currentGraph.getNumVertices();
    tel.qAfter = static_cast<int>(currentCliques.size());
    tel.status = BenchmarkStatus::SUCCESS;

    return tel;
}

DynamicUpdateStepTelemetry DynamicSweepLine::deleteVertex(int vertexId) {
    DynamicUpdateStepTelemetry tel;
    tel.opType = DynamicOpType::DELETION;
    tel.targetVertexId = vertexId;
    tel.nBefore = currentGraph.getNumVertices();
    tel.qBefore = static_cast<int>(currentCliques.size());

    auto t0 = std::chrono::steady_clock::now();

    auto ivs = currentGraph.getSortedIntervals();
    ivs.erase(
        std::remove_if(ivs.begin(), ivs.end(),
                       [vertexId](const Interval& iv) { return iv.id == vertexId; }),
        ivs.end());
    std::string err;
    currentGraph.initFromIntervals(ivs, err);

    auto t1 = std::chrono::steady_clock::now();

    currentCliques = StandardIntervalCliques::enumerateMaximalCliques(currentGraph);

    auto t2 = std::chrono::steady_clock::now();

    tel.updateApplicationMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    tel.algorithmUpdateMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    tel.totalLatencyMs = std::chrono::duration<double, std::milli>(t2 - t0).count();
    tel.nAfter = currentGraph.getNumVertices();
    tel.qAfter = static_cast<int>(currentCliques.size());
    tel.status = BenchmarkStatus::SUCCESS;

    return tel;
}

int64_t DynamicSweepLine::getTotalMemberships() const {
    int64_t total = 0;
    for (const auto& c : currentCliques) {
        total += c.size();
    }
    return total;
}
