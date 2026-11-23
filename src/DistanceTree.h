#ifndef DISTANCE_TREE_H
#define DISTANCE_TREE_H

#include "IntervalGraph.h"
#include <vector>
#include <unordered_map>
#include <string>

struct TreeNode {
    int vertexId;
    int sortedIndex;
    int parentId;
    int parentSortedIndex;
    std::vector<int> childrenIds;            // Ordered left-to-right
    std::vector<int> childrenSortedIndices;  // Ordered left-to-right
    int depth;                               // 0-based depth (root = 0)
    int level;                               // 1-based level (depth + 1)
    int posLevel;                            // Level-order position (0 to n-1)
    int posPre;                              // Preorder position (0 to n-1)
    int posPost;                             // Postorder position (0 to n-1)
};

class DistanceTree {
public:
    explicit DistanceTree(const IntervalGraph& graph);

    // Build the distance tree using Equation (2.1) / (3.1)
    // Runs in O(n) using monotonic sweep or O(n log n) using binary search
    void build();

    // Independent O(n^2) verification method that directly evaluates Eq (2.1)
    // parent(v) = argmin_{u \in V} { l_u : r_u >= l_v }
    std::vector<int> computeParentsBruteForce() const;

    // Verify all mathematical and structural invariants of the distance tree
    bool verifyTreeInvariants(std::string& errorMsg) const;

    // Accessors
    int getRootId() const { return rootId; }
    int getMaxDepth() const { return maxDepth; }
    int getNumLevels() const { return maxDepth + 1; }
    const std::vector<TreeNode>& getAllNodes() const { return nodes; }
    const TreeNode& getNodeById(int id) const;
    const TreeNode& getNodeBySortedIndex(int idx) const;

    // Levels: 0-based depth in [0, maxDepth]
    const std::vector<int>& getLevelVertices(int depth) const;
    // 1-based level in [1, maxDepth + 1]
    const std::vector<int>& getLevelVerticesOneBased(int level) const;

    // Traversal sequences (ordered list of vertex IDs)
    const std::vector<int>& getLevelOrderSequence() const { return levelOrderSeq; }
    const std::vector<int>& getPreorderSequence() const { return preorderSeq; }
    const std::vector<int>& getPostorderSequence() const { return postorderSeq; }

    // Export output files
    bool writeDistanceTree(const std::string& filepath) const;
    bool writeTreeParameters(const std::string& filepath) const;

private:
    const IntervalGraph& graph;
    int rootId;
    int maxDepth;
    std::vector<TreeNode> nodes; // Indexed by sortedIndex (0 to n-1)
    std::unordered_map<int, int> idToNodeIndex;

    // Level sets: levels[d] contains vertex IDs at depth d, ordered left-to-right
    std::vector<std::vector<int>> levels;

    // Traversal vertex sequences
    std::vector<int> levelOrderSeq;
    std::vector<int> preorderSeq;
    std::vector<int> postorderSeq;

    // Helper traversal routines
    void computeLevelOrder();
    void computePreorder(int nodeIdx, int& currentPos);
    void computePostorder(int nodeIdx, int& currentPos);
};

#endif // DISTANCE_TREE_H

