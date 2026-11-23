#ifndef DYNAMIC_CLIQUE_STRUCTURE_H
#define DYNAMIC_CLIQUE_STRUCTURE_H

#include "clique_types.h"
#include "IntervalGraph.h"
#include "DistanceTree.h"
#include "standard_interval_cliques.h"
#include "bron_kerbosch_adapter.h"
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <chrono>

// Node in the linked-child distance tree representation (Section 3.10.3)
struct DynamicTreeNode {
    int id;
    Interval interval;
    int level;          // 1-based level (depth + 1)
    int depth;          // 0-based depth
    int parentId;       // Parent vertex ID (-1 for root)

    // Boundary links of the ordered child set
    int childHead;      // Leftmost child ID (-1 if empty)
    int childTail;      // Rightmost child ID (-1 if empty)
    int childCount;     // Number of children

    // Doubly linked sibling links in parent's child set
    int prevSibling;
    int nextSibling;

    // Doubly linked same-level links
    int prevLevelSibling;
    int nextLevelSibling;
};

// ============================================================================
// Algorithm A (Dynamic): Thesis-Based Theorem Maintenance
// ============================================================================
class DynamicTheoremCliques {
public:
    DynamicTheoremCliques();

    // Initialize from an existing valid IntervalGraph and DistanceTree
    bool initialize(const IntervalGraph& graph, const DistanceTree& tree);

    // Dynamic Insertion: Rightmost append-only leaf insertion (Section 3.10.2)
    DynamicUpdateStepTelemetry insertLeaf(const Interval& newIv);

    // Dynamic Deletion: Theorem 3.16 child-set relocation (Section 3.10.3)
    DynamicUpdateStepTelemetry deleteVertex(int vertexId);

    // Explicit Materialization of current maximal cliques
    std::vector<std::vector<int>> materializeCurrentCliques() const;

    // Accessors
    int getNumVertices() const { return static_cast<int>(nodes.size()); }
    int getNumCliques() const { return static_cast<int>(parentToRightmostChild.size()); }
    int64_t getTotalMemberships() const;
    std::vector<CompactDescriptor> getDescriptors() const;
    std::vector<Interval> getCurrentIntervals() const;
    IntervalGraph exportCurrentGraph() const;

    // Query parent of a given vertex
    int getParentId(int vId) const;

private:
    int rootId;
    int maxDepth;
    std::unordered_map<int, DynamicTreeNode> nodes;
    std::vector<Interval> intervals; // Sorted canonically

    // Level heads and tails (1-based level)
    std::vector<int> levelHead;
    std::vector<int> levelTail;
    std::vector<int> levelCount;

    // Compact Maximal Clique Descriptors: parentId -> rightmostChildId
    std::map<int, int> parentToRightmostChild;

    // Helper: find parent in O(log n) time
    int findParentLogN(double left) const;

    // Helper: rebuild internal intervals vector sorted canonically
    void sortIntervals();
};

// ============================================================================
// Algorithm B (Dynamic): Bron-Kerbosch Full Recomputation Baseline
// ============================================================================
class DynamicBronKerbosch {
public:
    DynamicBronKerbosch();
    void initialize(const IntervalGraph& graph);

    DynamicUpdateStepTelemetry insertLeaf(const Interval& newIv, double timeoutMs = 3000.0);
    DynamicUpdateStepTelemetry deleteVertex(int vertexId, double timeoutMs = 3000.0);

    const std::vector<std::vector<int>>& getCurrentCliques() const { return currentCliques; }
    const IntervalGraph& getCurrentGraph() const { return currentGraph; }
    int getNumCliques() const { return static_cast<int>(currentCliques.size()); }
    int64_t getTotalMemberships() const;

private:
    IntervalGraph currentGraph;
    std::vector<std::vector<int>> currentCliques;
};

// ============================================================================
// Algorithm C (Dynamic): Standard Sweep-Line Full Recomputation Baseline
// ============================================================================
class DynamicSweepLine {
public:
    DynamicSweepLine();
    void initialize(const IntervalGraph& graph);

    DynamicUpdateStepTelemetry insertLeaf(const Interval& newIv);
    DynamicUpdateStepTelemetry deleteVertex(int vertexId);

    const std::vector<std::vector<int>>& getCurrentCliques() const { return currentCliques; }
    const IntervalGraph& getCurrentGraph() const { return currentGraph; }
    int getNumCliques() const { return static_cast<int>(currentCliques.size()); }
    int64_t getTotalMemberships() const;

private:
    IntervalGraph currentGraph;
    std::vector<std::vector<int>> currentCliques;
};

#endif // DYNAMIC_CLIQUE_STRUCTURE_H
