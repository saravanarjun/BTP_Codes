#ifndef DOMINATING_SET_H
#define DOMINATING_SET_H

#include "IntervalGraph.h"
#include "DistanceTree.h"
#include <vector>
#include <string>

// Theorem 3.17 Dominating Set Result
struct DominatingSetResult {
    int h;                              // Total number of levels (1-based: L_1 ... L_h)
    int n;                              // Total number of vertices
    int theoreticalBound;               // ceil((h - 1) / 2) for h >= 2; 1 for h == 1 (boundary case)
    int rawFormulaBound;                // ceil((h - 1) / 2) strictly evaluated (0 for h == 1)
    bool isBoundaryCaseH1;              // True if h == 1 (single-vertex graph)
    std::vector<int> selectedVertices;  // Selected vertex IDs in dominating set D
    int setSize;                        // |D|
    bool satisfiesBound;                // True if |D| <= theoreticalBound
    std::string constructionDetails;    // Detailed explanation of levels and vertices picked
};

// Independent Dominating Set Verification Result
struct DominatingSetVerification {
    bool allVerticesValid;              // Every vertex ID belongs to the graph
    bool allVerticesUnique;             // No duplicates in D
    bool isNonEmpty;                    // D is non-empty for non-empty graph
    bool isDominating;                  // Every v in V is in D or adjacent to some u in D
    std::vector<int> uncoveredVertices; // IDs of vertices NOT covered by D (if any)
    std::vector<std::string> messages;  // Verification log messages

    bool allPassed() const {
        return allVerticesValid && allVerticesUnique && isNonEmpty && isDominating && uncoveredVertices.empty();
    }
};

// Full Verification Report across Construction, Independent Verification, and Brute-Force Oracle
struct DominatingSetReport {
    int numVertices;
    int h;
    int constructedSize;
    int bruteForceMDSSize;              // True gamma(G), computed for small graphs (<= 25)
    bool hasBruteForceMDS;              // True if brute-force was run
    bool dominationPassed;
    bool boundSatisfied;
    bool isBoundaryCaseH1;
    DominatingSetResult result;
    DominatingSetVerification verification;
    std::vector<std::string> messages;

    bool allPassed() const {
        return dominationPassed && boundSatisfied && verification.allPassed();
    }
};

class DominatingSet {
public:
    // Theorem 3.17: Construct dominating set in O(h) time using distance tree level structure
    // Handles boundary case h = 1 explicitly (selects root v_1)
    static DominatingSetResult constructDominatingSet(const IntervalGraph& graph, const DistanceTree& tree);

    // Independent Validator: Verifies domination using interval intersections directly
    // Does NOT use distance tree or construction logic
    static DominatingSetVerification verifyDominatingSet(const IntervalGraph& graph, const std::vector<int>& dominatingSet);

    // Independent Ground Truth: Brute-force Minimum Dominating Set (MDS) solver for small graphs (n <= 25)
    static std::vector<int> computeBruteForceMDS(const IntervalGraph& graph);

    // Full verification audit
    static DominatingSetReport runFullVerification(const IntervalGraph& graph, const DistanceTree& tree, bool runBruteForce = true);

    // File exporters
    static bool writeDominatingSetReport(const DominatingSetResult& res, const IntervalGraph& graph, const DistanceTree& tree, const std::string& filepath);
    static bool writeVerificationReport(const DominatingSetReport& rep, const IntervalGraph& graph, const DistanceTree& tree, const std::string& filepath);
};

#endif // DOMINATING_SET_H

