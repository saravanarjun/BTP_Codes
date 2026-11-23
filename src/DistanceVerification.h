#ifndef DISTANCE_VERIFICATION_H
#define DISTANCE_VERIFICATION_H

#include "IntervalGraph.h"
#include "DistanceTree.h"
#include <vector>
#include <string>

struct DistanceMismatch {
    int uId;
    int vId;
    int uDepth;
    int vDepth;
    int uPosLevel;
    int vPosLevel;
    int uPosPost;
    int vPosPost;
    int formulaDist;
    int bfsDist;
    std::string details;
};

struct DistanceVerificationReport {
    int numVertices;
    long long totalPairsTested;
    long long matchedPairs;
    std::vector<DistanceMismatch> mismatches;

    bool passed() const {
        return mismatches.empty() && (totalPairsTested == matchedPairs);
    }
};

class DistanceVerification {
public:
    // Raw evaluation of Eq (2.2): requires pos_level(u) > pos_level(v)
    // dist_G(u, v) = depth(u) - depth(v) + [pos_post(u) > pos_post(v)]
    static int evaluateEquation2_2Raw(const DistanceTree& tree, int uId, int vId);

    // Symmetric query wrapper: handles any pair (x, y) by assigning u to the vertex
    // with larger pos_level and v to the other. Returns 0 if x == y.
    static int getDistanceFormula(const DistanceTree& tree, int xId, int yId);

    // Independent BFS ground truth on G (uses interval intersection, NOT tree edges)
    static int computeBfsDistance(const IntervalGraph& graph, int startId, int targetId);

    // Independent BFS distance from startId to all vertices
    static std::vector<int> computeSingleSourceBfs(const IntervalGraph& graph, int startId);

    // Verify all unordered and ordered pairs in graph against Eq (2.2)
    static DistanceVerificationReport verifyAllPairs(const IntervalGraph& graph, const DistanceTree& tree);

    // Verify Phase 2 positional claim: pos_level(u) < pos_level(v) <=> u < v in interval ordering
    static bool verifyPositionalOrderingProperty(const IntervalGraph& graph, const DistanceTree& tree, std::string& errorMsg);

    // Verify preorder and postorder traversal structural invariants
    static bool verifyTraversalProperties(const DistanceTree& tree, std::string& errorMsg);

    // Deterministic generator of connected proper interval graphs for stress testing
    static std::vector<Interval> generateRandomProperIntervals(int n, unsigned int seed);
};

#endif // DISTANCE_VERIFICATION_H

