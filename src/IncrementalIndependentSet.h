#ifndef INCREMENTAL_INDEPENDENT_SET_H
#define INCREMENTAL_INDEPENDENT_SET_H

#include "IntervalGraph.h"
#include "DistanceTree.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

// Result of an incremental leaf insertion attempt
struct IncrementalUpdateResult {
    bool success;
    int newVertexId;
    int parentId;
    int level;
    int depth;
    int dpValue;
    int prefixMaxLeftOfP;
    int bestPrev;
    int oldAlpha;
    int newAlpha;
    bool alphaIncreased;
    std::string diagnostic;
};

// Independent verification report after an incremental update
struct IncrementalVerificationReport {
    bool isLeafInUpdatedTree;
    bool oldParentsPreserved;
    bool oldDepthsPreserved;
    bool parentMatchesBruteForce;
    bool dpValueMatchesStatic;
    bool prefixMaxMatchesStatic;
    bool alphaMatchesStatic;
    bool alphaMatchesBruteForce;
    int incrementalAlpha;
    int staticAlpha;
    int bruteForceAlpha;
    std::vector<std::string> messages;

    bool allPassed() const {
        return isLeafInUpdatedTree && oldParentsPreserved && oldDepthsPreserved &&
               parentMatchesBruteForce && dpValueMatchesStatic && prefixMaxMatchesStatic &&
               alphaMatchesStatic && alphaMatchesBruteForce;
    }
};

// ============================================================================
// Internal Data Structures for O(log n) Parent Search and Prefix Maxima
// ============================================================================

// Candidate tuple for parent selection: (left, right, id)
struct ParentCandidate {
    double left;
    double right;
    int id;

    bool operator<(const ParentCandidate& other) const {
        if (left != other.left) return left < other.left;
        if (right != other.right) return right < other.right;
        return id < other.id;
    }
};

// Augmented AVL Tree Node for Parent Search (Keyed by right endpoint)
struct ParentAVLNode {
    double rightKey;
    int idKey;
    ParentCandidate item;
    ParentCandidate minCandidate; // Augmented: min (left, right, id) in subtree
    int height;
    ParentAVLNode* left;
    ParentAVLNode* right;

    ParentAVLNode(const Interval& iv);
};

class ParentAVLTree {
public:
    ParentAVLTree();
    ~ParentAVLTree();

    // Rule of Five
    ParentAVLTree(const ParentAVLTree& other);
    ParentAVLTree& operator=(const ParentAVLTree& other);
    ParentAVLTree(ParentAVLTree&& other) noexcept;
    ParentAVLTree& operator=(ParentAVLTree&& other) noexcept;

    void clear();
    void insert(const Interval& iv);
    bool findParent(double queryLeft, ParentCandidate& bestCand) const;

private:
    ParentAVLNode* root;

    static int getHeight(ParentAVLNode* n);
    static int getBalance(ParentAVLNode* n);
    static void updateNode(ParentAVLNode* n);
    static ParentAVLNode* rotateRight(ParentAVLNode* y);
    static ParentAVLNode* rotateLeft(ParentAVLNode* x);
    static ParentAVLNode* insertNode(ParentAVLNode* node, const Interval& iv);
    static void destroyTree(ParentAVLNode* node);
    static ParentAVLNode* cloneTree(const ParentAVLNode* node);
};

// Canonical ordering key for level members: (left, right, id)
struct CanonicalKey {
    double left;
    double right;
    int id;

    bool operator<(const CanonicalKey& other) const {
        if (left != other.left) return left < other.left;
        if (right != other.right) return right < other.right;
        return id < other.id;
    }
    bool operator<=(const CanonicalKey& other) const {
        return !(other < *this);
    }
    bool operator==(const CanonicalKey& other) const {
        return left == other.left && right == other.right && id == other.id;
    }
};

// Augmented AVL Tree Node for Level Prefix Maxima (Keyed by canonical order)
struct LevelAVLNode {
    CanonicalKey key;
    int dpValue;
    int maxDP; // Augmented: max DP in subtree
    int height;
    LevelAVLNode* left;
    LevelAVLNode* right;

    LevelAVLNode(const CanonicalKey& k, int dp);
};

class LevelAVLTree {
public:
    LevelAVLTree();
    ~LevelAVLTree();

    // Rule of Five
    LevelAVLTree(const LevelAVLTree& other);
    LevelAVLTree& operator=(const LevelAVLTree& other);
    LevelAVLTree(LevelAVLTree&& other) noexcept;
    LevelAVLTree& operator=(LevelAVLTree&& other) noexcept;

    void clear();
    void insert(const CanonicalKey& key, int dpVal);
    int queryPrefixMaxStrictlyLeft(const CanonicalKey& key) const;
    int getMaxDP() const;

private:
    LevelAVLNode* root;

    static int getHeight(LevelAVLNode* n);
    static int getBalance(LevelAVLNode* n);
    static void updateNode(LevelAVLNode* n);
    static LevelAVLNode* rotateRight(LevelAVLNode* y);
    static LevelAVLNode* rotateLeft(LevelAVLNode* x);
    static LevelAVLNode* insertNode(LevelAVLNode* node, const CanonicalKey& key, int dpVal);
    static void destroyTree(LevelAVLNode* node);
    static LevelAVLNode* cloneTree(const LevelAVLNode* node);
};

// ============================================================================
// Main Incremental MIS Manager
// ============================================================================

class IncrementalIndependentSet {
public:
    IncrementalIndependentSet();

    // Initialize from an existing valid IntervalGraph and DistanceTree
    bool initialize(const IntervalGraph& graph, const DistanceTree& tree, std::string& errorMessage);

    // Validate if inserting newIv satisfies properness, connectedness, and the rightmost append-only leaf model.
    // Restricting to rightmost-append insertions ensures no existing vertex's prefix maximum increases,
    // which guarantees that all existing DP values remain invariant and prevents cascading DP invalidations.
    bool validateLeafInsertion(const Interval& newIv, std::string& diagnosticMsg) const;

    // Perform the incremental update (Theorem 3.14) in O(log n) internal tree-maintenance time
    // (End-to-end insertion including O(n) pairwise validation is O(n)).
    IncrementalUpdateResult insertLeaf(const Interval& newIv);

    // Run independent correctness verification of the current incremental state
    IncrementalVerificationReport runIndependentVerification() const;

    // Accessors
    int getNumVertices() const { return static_cast<int>(intervals.size()); }
    int getNumLevels() const { return h; }
    int getAlpha() const { return currentAlpha; }
    int getVertexDP(int vertexId) const;
    int getVertexLevel(int vertexId) const;
    int getVertexParent(int vertexId) const;
    const std::vector<Interval>& getIntervals() const { return intervals; }

    // Diagnostic printer
    std::string printStatus() const;

private:
    int n;
    int h;
    int currentAlpha;
    std::vector<int> BEST; // BEST[i]: max independent set size among levels 1..i (BEST[0]=0)

    std::vector<Interval> intervals;
    std::unordered_map<int, int> idToLevel;  // 1-based level
    std::unordered_map<int, int> idToParent; // parent vertex ID
    std::unordered_map<int, int> idToDP;     // DP value

    ParentAVLTree parentSearchTree;
    std::vector<LevelAVLTree> levelTrees; // 1-based indexing (levelTrees[i] for level i)

    // Helper: Find parent of an interval using the O(log n) parentSearchTree
    bool findParentLogN(double queryLeft, int& parentId, double& parentLeft, double& parentRight) const;
};

#endif // INCREMENTAL_INDEPENDENT_SET_H

