#include "DistanceTree.h"
#include <queue>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cassert>

DistanceTree::DistanceTree(const IntervalGraph& g)
    : graph(g), rootId(-1), maxDepth(0) {
    build();
}

void DistanceTree::build() {
    int n = graph.getNumVertices();
    if (n == 0) {
        return;
    }

    nodes.clear();
    nodes.resize(n);
    idToNodeIndex.clear();

    const auto& sortedIvs = graph.getSortedIntervals();

    // Initialize basic node properties
    for (int i = 0; i < n; ++i) {
        nodes[i].vertexId = sortedIvs[i].id;
        nodes[i].sortedIndex = i;
        nodes[i].parentId = -1;
        nodes[i].parentSortedIndex = -1;
        nodes[i].depth = 0;
        nodes[i].level = 1;
        nodes[i].posLevel = -1;
        nodes[i].posPre = -1;
        nodes[i].posPost = -1;
        idToNodeIndex[sortedIvs[i].id] = i;
    }

    // Root is the first vertex in sorted interval order (Section 2.6, 3.2)
    rootId = sortedIvs[0].id;
    nodes[0].parentId = rootId;
    nodes[0].parentSortedIndex = 0;

    // Optimized parent determination using monotonic sweep in O(n) time
    // parent(v_i) = argmin_{u \in V} { l_u : r_u >= l_{v_i} }
    // Since intervals are sorted by left endpoints and the graph is proper,
    // right endpoints are also non-decreasing, making the parent index monotonically non-decreasing.
    int p = 0;
    for (int i = 1; i < n; ++i) {
        while (p < i && sortedIvs[p].right < sortedIvs[i].left) {
            p++;
        }
        nodes[i].parentId = sortedIvs[p].id;
        nodes[i].parentSortedIndex = p;

        // Maintain children in left-to-right interval order (Section 2.7, 3.2)
        nodes[p].childrenSortedIndices.push_back(i);
        nodes[p].childrenIds.push_back(nodes[i].vertexId);
    }

    // Compute depths and levels via BFS from root
    maxDepth = 0;
    std::queue<int> q;
    q.push(0);
    nodes[0].depth = 0;
    nodes[0].level = 1;

    while (!q.empty()) {
        int uIdx = q.front();
        q.pop();

        int d = nodes[uIdx].depth;
        if (d > maxDepth) {
            maxDepth = d;
        }

        for (int cIdx : nodes[uIdx].childrenSortedIndices) {
            nodes[cIdx].depth = d + 1;
            nodes[cIdx].level = d + 2;
            q.push(cIdx);
        }
    }

    // Populate level sets: levels[d] contains vertices at depth d in left-to-right order
    levels.clear();
    levels.resize(maxDepth + 1);
    for (int i = 0; i < n; ++i) {
        levels[nodes[i].depth].push_back(nodes[i].vertexId);
    }

    // Compute traversal orders (Section 2.8, 3.4)
    computeLevelOrder();

    int prePos = 0;
    preorderSeq.clear();
    preorderSeq.reserve(n);
    computePreorder(0, prePos);

    int postPos = 0;
    postorderSeq.clear();
    postorderSeq.reserve(n);
    computePostorder(0, postPos);
}

std::vector<int> DistanceTree::computeParentsBruteForce() const {
    int n = graph.getNumVertices();
    std::vector<int> bruteParents(n, -1);
    if (n == 0) return bruteParents;

    const auto& sortedIvs = graph.getSortedIntervals();
    bruteParents[0] = sortedIvs[0].id; // root

    for (int i = 1; i < n; ++i) {
        int bestIdx = -1;
        double minLeft = 1e300;
        for (int j = 0; j < n; ++j) {
            if (sortedIvs[j].right >= sortedIvs[i].left) {
                if (bestIdx == -1 || sortedIvs[j].left < minLeft ||
                   (sortedIvs[j].left == minLeft && sortedIvs[j].right < sortedIvs[bestIdx].right) ||
                   (sortedIvs[j].left == minLeft && sortedIvs[j].right == sortedIvs[bestIdx].right && sortedIvs[j].id < sortedIvs[bestIdx].id)) {
                    bestIdx = j;
                    minLeft = sortedIvs[j].left;
                }
            }
        }
        assert(bestIdx != -1);
        bruteParents[i] = sortedIvs[bestIdx].id;
    }

    return bruteParents;
}

bool DistanceTree::verifyTreeInvariants(std::string& errorMsg) const {
    int n = graph.getNumVertices();
    if (n == 0) {
        errorMsg = "Empty graph has no distance tree.";
        return false;
    }

    // 1. Root check
    if (nodes[0].vertexId != rootId) {
        errorMsg = "Root ID mismatch at index 0.";
        return false;
    }
    if (nodes[0].parentId != rootId) {
        errorMsg = "Root parent must be itself (Equation 2.1).";
        return false;
    }
    if (nodes[0].depth != 0 || nodes[0].level != 1) {
        errorMsg = "Root must have depth 0 and level 1.";
        return false;
    }

    // 2. Independent brute-force parent check
    std::vector<int> bruteParents = computeParentsBruteForce();
    for (int i = 0; i < n; ++i) {
        if (nodes[i].parentId != bruteParents[i]) {
            std::ostringstream oss;
            oss << "Parent mismatch for vertex " << nodes[i].vertexId 
                << ": fast sweep got " << nodes[i].parentId 
                << " but brute-force got " << bruteParents[i];
            errorMsg = oss.str();
            return false;
        }
    }

    // 3. Tree edge validity in G
    for (int i = 1; i < n; ++i) {
        int uId = nodes[i].vertexId;
        int pId = nodes[i].parentId;
        if (!graph.isAdjacent(uId, pId)) {
            std::ostringstream oss;
            oss << "Tree edge (" << uId << ", " << pId << ") is not an edge in G.";
            errorMsg = oss.str();
            return false;
        }
    }

    // 4. Reachability from root and tree topology (no cycles, n-1 edges)
    std::vector<bool> visited(n, false);
    std::queue<int> q;
    q.push(0);
    visited[0] = true;
    int visitCount = 0;

    while (!q.empty()) {
        int uIdx = q.front();
        q.pop();
        visitCount++;

        for (int cIdx : nodes[uIdx].childrenSortedIndices) {
            if (visited[cIdx]) {
                errorMsg = "Cycle or multiple parents detected at vertex " + std::to_string(nodes[cIdx].vertexId);
                return false;
            }
            visited[cIdx] = true;
            q.push(cIdx);
        }
    }

    if (visitCount != n) {
        std::ostringstream oss;
        oss << "Not all vertices reachable from root: visited " << visitCount << " of " << n << " vertices.";
        errorMsg = oss.str();
        return false;
    }

    // 5. Children ordering verification
    for (int i = 0; i < n; ++i) {
        const auto& cIndices = nodes[i].childrenSortedIndices;
        for (size_t k = 0; k + 1 < cIndices.size(); ++k) {
            if (cIndices[k] >= cIndices[k + 1]) {
                errorMsg = "Children of vertex " + std::to_string(nodes[i].vertexId) + " are not in left-to-right order.";
                return false;
            }
        }
    }

    // 6. Level consistency verification
    for (int d = 0; d <= maxDepth; ++d) {
        for (int vId : levels[d]) {
            const auto& node = getNodeById(vId);
            if (node.depth != d) {
                errorMsg = "Vertex " + std::to_string(vId) + " listed in level " + std::to_string(d) + " but has depth " + std::to_string(node.depth);
                return false;
            }
        }
    }

    errorMsg.clear();
    return true;
}

void DistanceTree::computeLevelOrder() {
    int n = graph.getNumVertices();
    levelOrderSeq.clear();
    levelOrderSeq.reserve(n);

    std::queue<int> q;
    q.push(0);
    int pos = 0;

    while (!q.empty()) {
        int uIdx = q.front();
        q.pop();

        nodes[uIdx].posLevel = pos++;
        levelOrderSeq.push_back(nodes[uIdx].vertexId);

        for (int cIdx : nodes[uIdx].childrenSortedIndices) {
            q.push(cIdx);
        }
    }
}

void DistanceTree::computePreorder(int nodeIdx, int& currentPos) {
    nodes[nodeIdx].posPre = currentPos++;
    preorderSeq.push_back(nodes[nodeIdx].vertexId);

    for (int cIdx : nodes[nodeIdx].childrenSortedIndices) {
        computePreorder(cIdx, currentPos);
    }
}

void DistanceTree::computePostorder(int nodeIdx, int& currentPos) {
    for (int cIdx : nodes[nodeIdx].childrenSortedIndices) {
        computePostorder(cIdx, currentPos);
    }

    nodes[nodeIdx].posPost = currentPos++;
    postorderSeq.push_back(nodes[nodeIdx].vertexId);
}

const TreeNode& DistanceTree::getNodeById(int id) const {
    auto it = idToNodeIndex.find(id);
    if (it == idToNodeIndex.end()) {
        throw std::out_of_range("Vertex ID not found in tree: " + std::to_string(id));
    }
    return nodes[it->second];
}

const TreeNode& DistanceTree::getNodeBySortedIndex(int idx) const {
    if (idx < 0 || idx >= static_cast<int>(nodes.size())) {
        throw std::out_of_range("Node index out of range: " + std::to_string(idx));
    }
    return nodes[idx];
}

const std::vector<int>& DistanceTree::getLevelVertices(int depth) const {
    if (depth < 0 || depth > maxDepth) {
        throw std::out_of_range("Depth out of range: " + std::to_string(depth));
    }
    return levels[depth];
}

const std::vector<int>& DistanceTree::getLevelVerticesOneBased(int level) const {
    return getLevelVertices(level - 1);
}

bool DistanceTree::writeDistanceTree(const std::string& filepath) const {
    std::ofstream fout(filepath);
    if (!fout.is_open()) return false;

    fout << "# Distance Tree Representation T(G)\n";
    fout << "# Root: " << rootId << "\n";
    fout << "# Total Vertices: " << nodes.size() << "\n";
    fout << "# Max Depth: " << maxDepth << ", Total Levels: " << getNumLevels() << "\n\n";
    fout << "Vertex_ID\tParent_ID\tDepth\tLevel\tNum_Children\tChildren_IDs\n";

    for (const auto& node : nodes) {
        fout << node.vertexId << "\t\t"
             << node.parentId << "\t\t"
             << node.depth << "\t"
             << node.level << "\t"
             << node.childrenIds.size() << "\t\t";
        if (node.childrenIds.empty()) {
            fout << "none";
        } else {
            for (size_t k = 0; k < node.childrenIds.size(); ++k) {
                if (k > 0) fout << ",";
                fout << node.childrenIds[k];
            }
        }
        fout << "\n";
    }
    return true;
}

bool DistanceTree::writeTreeParameters(const std::string& filepath) const {
    std::ofstream fout(filepath);
    if (!fout.is_open()) return false;

    fout << "========================================================\n";
    fout << " Distance Tree Parameters (Sections 2.7, 2.8, 3.3, 3.4)\n";
    fout << "========================================================\n\n";
    fout << "Root Vertex ID: " << rootId << "\n";
    fout << "Maximum Depth h_depth: " << maxDepth << "\n";
    fout << "Total Number of Levels h: " << getNumLevels() << "\n\n";

    fout << "--- Level Sets (Left-to-Right Ordered) ---\n";
    for (int d = 0; d <= maxDepth; ++d) {
        fout << "Level " << (d + 1) << " (Depth " << d << ", Size " << levels[d].size() << "): [";
        for (size_t k = 0; k < levels[d].size(); ++k) {
            if (k > 0) fout << ", ";
            fout << levels[d][k];
        }
        fout << "]\n";
    }
    fout << "\n";

    fout << "--- Positional Parameters ---\n";
    fout << "Vertex_ID\tSorted_Index\tDepth\tLevel\tpos_level\tpos_pre\tpos_post\n";
    for (const auto& node : nodes) {
        fout << node.vertexId << "\t\t"
             << node.sortedIndex << "\t\t"
             << node.depth << "\t"
             << node.level << "\t"
             << node.posLevel << "\t\t"
             << node.posPre << "\t\t"
             << node.posPost << "\n";
    }
    fout << "\n";

    fout << "--- Traversal Sequences ---\n";
    fout << "Level-Order Traversal: ";
    for (size_t k = 0; k < levelOrderSeq.size(); ++k) {
        if (k > 0) fout << " -> ";
        fout << levelOrderSeq[k];
    }
    fout << "\n\nPreorder Traversal:    ";
    for (size_t k = 0; k < preorderSeq.size(); ++k) {
        if (k > 0) fout << " -> ";
        fout << preorderSeq[k];
    }
    fout << "\n\nPostorder Traversal:   ";
    for (size_t k = 0; k < postorderSeq.size(); ++k) {
        if (k > 0) fout << " -> ";
        fout << postorderSeq[k];
    }
    fout << "\n";

    return true;
}

