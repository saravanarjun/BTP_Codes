#ifndef THEOREM_CLIQUES_ADAPTER_H
#define THEOREM_CLIQUES_ADAPTER_H

#include "IntervalGraph.h"
#include "DistanceTree.h"
#include "MaximalCliques.h"
#include <vector>

// Compact descriptor produced by Algorithm A (Lemma 3.15)
struct TheoremCliqueDescriptor {
    int parentId;
    int rightmostChildId;
    int parentSortedIndex;
    int rightmostChildSortedIndex;
    double leftBound;   // l_p
    double rightBound;  // l_c
    int size;
};

class TheoremCliquesAdapter {
public:
    // Algorithm A Core: Lemma 3.15 compact clique identification from distance tree
    // Complexity: O(n) scan over non-leaf parent nodes
    static std::vector<TheoremCliqueDescriptor> computeCompactDescriptors(
        const IntervalGraph& graph,
        const DistanceTree& tree);

    // Algorithm A Materialization: Expand descriptors into explicit vector<vector<int>>
    // Complexity: O(S) where S = sum |C_j|
    static std::vector<std::vector<int>> materializeCliques(
        const std::vector<TheoremCliqueDescriptor>& descriptors,
        const IntervalGraph& graph);

    // Direct call to approved Phase 5 function (MaximalCliques::constructCompactCliques)
    static std::vector<CompactClique> runApprovedPhase5Compact(
        const IntervalGraph& graph,
        const DistanceTree& tree);

    // Direct call to approved Phase 5 Lemma 3.8 all-node clique extraction
    static std::vector<std::vector<int>> runApprovedPhase5Lemma3_8(
        const IntervalGraph& graph,
        const DistanceTree& tree);
};

#endif // THEOREM_CLIQUES_ADAPTER_H
