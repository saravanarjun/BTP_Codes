#ifndef GRAPH_GENERATOR_H
#define GRAPH_GENERATOR_H

#include "IntervalGraph.h"
#include <vector>
#include <string>
#include <cstdint>

enum class OverlapProfile {
    SPARSE_OVERLAP,   // Path-like chain, k = 1, avg degree approx 2.0
    MEDIUM_OVERLAP,   // Moderate overlap, k = 8, avg degree approx 16.0
    DENSE_OVERLAP     // Broad clique overlap, k = 25, avg degree approx 50.0
};

inline std::string overlapProfileToString(OverlapProfile p) {
    switch (p) {
        case OverlapProfile::SPARSE_OVERLAP: return "sparse-overlap";
        case OverlapProfile::MEDIUM_OVERLAP: return "medium-overlap";
        case OverlapProfile::DENSE_OVERLAP:  return "dense-overlap";
        default: return "unknown";
    }
}

class GraphGenerator {
public:
    // 1. Equal-length intervals with regularly spaced endpoints
    static IntervalGraph generateEqualLengthsGraph(int n, OverlapProfile profile);
    static IntervalGraph generateEqualLengthsGraph(int n, double step, double width);

    // 2. Unequal-length proper intervals with strictly increasing endpoints
    static IntervalGraph generateUnequalLengthsGraph(int n, double baseStep = 1.0, double minW = 2.0, double maxW = 8.0);

    // 3. Randomized proper interval graphs (with deterministic seed)
    static IntervalGraph generateRandomProperGraph(int n, uint32_t seed, double minW = 2.0, double maxW = 10.0, double stepMean = 1.0);

    // 4. Identical interval clusters (multiple intervals sharing identical endpoints)
    static IntervalGraph generateIdenticalIntervalsGraph(int numUnique, int clusterSize, double step = 1.0, double width = 3.5);

    // 5. Endpoint-touching intervals under closed-interval semantics (r_i = l_{i+1})
    static IntervalGraph generateEndpointTouchingGraph(int n, double width = 2.0);

    // 6. Long chains of small-overlap intervals (path-like chain, k = 1)
    static IntervalGraph generateLongChainGraph(int n);

    // 7. High-overlap interval families (large bandwidth, broad maximal cliques)
    static IntervalGraph generateHighOverlapGraph(int n, int bandwidth = 40);

    // 8. Alternating short and long interval lengths while preserving properness
    static IntervalGraph generateAlternatingLengthsGraph(int n, double shortW = 3.0, double longW = 6.0, double step = 2.0);

    // 9. Irregularly spaced endpoints
    static IntervalGraph generateIrregularEndpointsGraph(int n, uint32_t seed);

    // 10. Graphs with many small maximal cliques (k = 2)
    static IntervalGraph generateManySmallCliquesGraph(int n);

    // 11. Graphs with fewer but large maximal cliques
    static IntervalGraph generateFewLargeCliquesGraph(int n);

    // 12. Graphs with varying maximal-clique sizes and overlap patterns
    static IntervalGraph generateVaryingOverlapGraph(int n);

    // 13. Adversarial cases for parent selection and tie-breaking
    static IntervalGraph generateAdversarialTieBreakingGraph(int n);

    // 14. Graphs with shuffled vertex IDs (verifying independence from ID == sorted index)
    static IntervalGraph generateShuffledIdsGraph(int n, uint32_t seed);

    // Graph validation helper
    static bool validateProperConnectedGraph(const IntervalGraph& graph, std::string& outDiagnostic);
};

#endif // GRAPH_GENERATOR_H
