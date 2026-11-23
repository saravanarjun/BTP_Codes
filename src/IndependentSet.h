#ifndef INDEPENDENT_SET_H
#define INDEPENDENT_SET_H

#include "IntervalGraph.h"
#include "DistanceTree.h"
#include <vector>
#include <string>

// Lemma 3.11 Bounds Result
struct MISBoundsResult {
    int h;                          // Number of levels (1-based: L_1 ... L_h)
    int lowerBound;                 // floor(h / 2)
    int upperBound;                 // h - 1 (valid for h >= 2)
    int actualAlpha;                // alpha(G)
    bool isBoundaryCaseH1;          // True if h == 1
    bool boundsSatisfied;           // True if actualAlpha in [lowerBound, upperBound] (or h==1 handled)
    std::string details;
};

// Theorem 3.12 2-Approximation Result
struct MISApproximationResult {
    int h;
    std::vector<int> selectedVertices; // Rightmost vertex from levels 1, 3, 5, ...
    int approxSize;                 // ceil(h / 2)
    bool isIndependent;
    double approxRatio;             // alpha(G) / approxSize
    bool guaranteeSatisfied;        // ratio < 2.0 (or 1.0 for h=1)
    std::string details;
};

// Section 3.10.1 & Theorem 3.13 Exact Linear-Time Dynamic Programming Result
struct MISExactResult {
    int alpha;                      // BEST[h]
    std::vector<int> selectedVertices; // Reconstructed maximum independent set
    std::vector<std::vector<int>> dpValues; // dpValues[i][k]: DP for k-th vertex in L_i (1-based level)
    std::vector<int> bestValues;    // bestValues[i]: BEST[i] for i = 0 ... h
    bool isIndependent;
};

// Comprehensive MIS Verification Report
struct MISVerificationReport {
    int numVertices;
    int h;
    int dpAlpha;
    int bruteForceAlpha;
    int approxSize;
    bool dpAgreesWithBruteForce;
    bool dpSetIsIndependent;
    bool approxSetIsIndependent;
    bool boundsValid;
    std::vector<std::string> messages;

    bool allPassed() const {
        return dpAgreesWithBruteForce && dpSetIsIndependent && approxSetIsIndependent && boundsValid;
    }
};

class IndependentSet {
public:
    // Lemma 3.11: Check bounds floor(h/2) <= alpha(G) <= h - 1
    // Handles h = 1 boundary case explicitly
    static MISBoundsResult verifyBounds(const DistanceTree& tree, int actualAlpha);

    // Theorem 3.12: 2-Approximation algorithm in O(h) time
    static MISApproximationResult computeApproximation(const IntervalGraph& graph, const DistanceTree& tree, int actualAlpha);

    // Section 3.10.1 & Theorem 3.13: Exact linear-time DP algorithm in O(n) time and O(n) space
    static MISExactResult computeExactDP(const IntervalGraph& graph, const DistanceTree& tree);

    // Independent Ground Truth: Brute-force MIS solver for small graphs (using interval intersections directly)
    static std::vector<int> computeBruteForceMIS(const IntervalGraph& graph);

    // Verification helper: Checks if a set of vertices is independent using interval intersection
    static bool isIndependentSet(const IntervalGraph& graph, const std::vector<int>& vertexIds);

    // Full verification audit
    static MISVerificationReport runFullVerification(const IntervalGraph& graph, const DistanceTree& tree);

    // File exporters
    static bool writeBoundsReport(const MISBoundsResult& res, const std::string& filepath);
    static bool writeApproximationReport(const MISApproximationResult& res, const std::string& filepath);
    static bool writeExactDPReport(const MISExactResult& res, const DistanceTree& tree, const std::string& filepath);
    static bool writeVerificationReport(const MISVerificationReport& rep, const std::string& filepath);

private:
    static void bruteForceBacktrack(const IntervalGraph& graph, const std::vector<int>& allVertices,
                                   size_t idx, std::vector<int>& currentSet, std::vector<int>& bestSet);
};

#endif // INDEPENDENT_SET_H

