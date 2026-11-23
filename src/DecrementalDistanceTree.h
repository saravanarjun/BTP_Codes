#ifndef DECREMENTAL_DISTANCE_TREE_H
#define DECREMENTAL_DISTANCE_TREE_H

#include "IntervalGraph.h"
#include "DistanceTree.h"
#include "IndependentSet.h"
#include "IncrementalIndependentSet.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>

// Node in the linked-child distance tree representation (Chapter 3, Section 3.10.3)
struct DecrementalNode {
    int id;
    Interval interval;
    int level;          // 1-based level (depth + 1)
    int depth;          // 0-based depth (root = 0)
    int parentId;       // Parent vertex ID (-1 for root)

    // Ordered linked set of children (ordered left-to-right by canonical order)
    // In accordance with Section 3.10.3, only the boundary links (head and tail)
    // of each child set are explicitly maintained.
    int childHead;      // Leftmost child ID (-1 if empty)
    int childTail;      // Rightmost child ID (-1 if empty)
    int childCount;     // Number of children in the set

    // Doubly linked sibling links in the parent's ordered child set
    int prevSibling;    // Sibling immediately to the left (-1 if none)
    int nextSibling;    // Sibling immediately to the right (-1 if none)

    // Doubly linked same-level links
    int prevLevelSibling; // Vertex immediately to the left in the same level (-1 if none)
    int nextLevelSibling; // Vertex immediately to the right in the same level (-1 if none)

    bool operator==(const DecrementalNode& o) const {
        return id == o.id &&
               interval == o.interval &&
               level == o.level &&
               depth == o.depth &&
               parentId == o.parentId &&
               childHead == o.childHead &&
               childTail == o.childTail &&
               childCount == o.childCount &&
               prevSibling == o.prevSibling &&
               nextSibling == o.nextSibling &&
               prevLevelSibling == o.prevLevelSibling &&
               nextLevelSibling == o.nextLevelSibling;
    }
    bool operator!=(const DecrementalNode& o) const {
        return !(*this == o);
    }
};

// Full observable state snapshot for verifying atomic rejection
struct DecrementalStateSnapshot {
    int rootId;
    int maxDepth;
    int currentAlpha;
    std::vector<Interval> intervals;
    std::unordered_map<int, DecrementalNode> nodes;
    std::vector<int> levelHead;
    std::vector<int> levelTail;
    std::vector<int> levelCount;
};

// Result of a decremental vertex deletion attempt
struct DeletionResult {
    bool success;
    int deletedId;
    int parentId;
    int immediateRightId;     // Vertex v that adopted u's children (-1 if u had no children or no v)
    int numChildrenRelocated;
    int oldAlpha;
    int newAlpha;
    bool alphaChanged;
    std::string diagnostic;
};

// Independent verification report after a deletion
struct DecrementalVerificationReport {
    bool success;
    bool treeInvariantsPassed;
    bool parentsMatchStaticRebuild;
    bool depthsMatchStaticRebuild;
    bool childrenMatchStaticRebuild;
    bool rootMatchesStaticRebuild;
    bool levelsMatchStaticRebuild;
    bool structuralLemmasPassed;
    bool misMatchesStaticDP;
    bool misMatchesBruteForce;
    int decrementalAlpha;
    int staticAlpha;
    int bruteForceAlpha;
    std::vector<std::string> messages;

    bool allPassed() const {
        return success && treeInvariantsPassed && parentsMatchStaticRebuild &&
               depthsMatchStaticRebuild && childrenMatchStaticRebuild &&
               rootMatchesStaticRebuild && levelsMatchStaticRebuild &&
               structuralLemmasPassed && misMatchesStaticDP && misMatchesBruteForce;
    }
};

class DecrementalDistanceTree {
public:
    DecrementalDistanceTree();

    // Initialize from an existing valid IntervalGraph and DistanceTree
    bool initialize(const IntervalGraph& graph, const DistanceTree& tree, std::string& errorMsg);

    // Validate whether vertex deletion is admissible under Theorem 3.16 and graph invariants
    bool validateDeletion(int vertexId, std::string& diagnosticMsg) const;

    // Execute atomic vertex deletion (Theorem 3.16)
    // Splicing child sets takes O(1) boundary modifications.
    // Parent lookup takes O(log n) via balanced search tree.
    DeletionResult deleteVertex(int vertexId);

    // Run full independent verification against a fresh static rebuild and brute force
    DecrementalVerificationReport runIndependentVerification() const;

    // Accessors
    int getNumVertices() const { return static_cast<int>(nodes.size()); }
    int getRootId() const { return rootId; }
    int getMaxDepth() const { return maxDepth; }
    int getNumLevels() const { return maxDepth + 1; }
    int getAlpha() const { return currentAlpha; }
    bool hasVertex(int id) const { return nodes.find(id) != nodes.end(); }
    const DecrementalNode& getNode(int id) const { return nodes.at(id); }
    const std::vector<Interval>& getIntervals() const { return currentIntervals; }
    std::vector<int> getChildren(int id) const;
    std::vector<int> getLevelVertices(int level) const;

    // Reconstruct logical tree parent-child map by traversing the linked representation
    void getLogicalTree(std::unordered_map<int, int>& outParents,
                        std::unordered_map<int, int>& outDepths,
                        std::unordered_map<int, std::vector<int>>& outChildren) const;

    // State snapshot and atomicity check
    DecrementalStateSnapshot getSnapshot() const;
    bool matchesSnapshot(const DecrementalStateSnapshot& snap) const;

private:
    int rootId;
    int maxDepth;
    int currentAlpha;

    // Node storage
    std::unordered_map<int, DecrementalNode> nodes;
    std::vector<Interval> currentIntervals;

    // Level heads and tails (1-based level indexing)
    std::vector<int> levelHead;
    std::vector<int> levelTail;
    std::vector<int> levelCount;

    // Balanced parent search tree (Phase 8) for O(log n) parent queries
    ParentAVLTree parentSearchTree;

    // Helper routines
    void rebuildParentSearchTree();
    void recomputeMIS();
};

#endif // DECREMENTAL_DISTANCE_TREE_H

