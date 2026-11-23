#include "theorem_cliques_adapter.h"

std::vector<TheoremCliqueDescriptor> TheoremCliquesAdapter::computeCompactDescriptors(
    const IntervalGraph& graph,
    const DistanceTree& tree) {

    int n = graph.getNumVertices();
    std::vector<TheoremCliqueDescriptor> descriptors;
    if (n == 0) return descriptors;

    const auto& sortedIvs = graph.getSortedIntervals();

    if (n == 1) {
        int rootId = tree.getRootId();
        TheoremCliqueDescriptor desc;
        desc.parentId = rootId;
        desc.rightmostChildId = rootId;
        desc.parentSortedIndex = 0;
        desc.rightmostChildSortedIndex = 0;
        desc.leftBound = sortedIvs[0].left;
        desc.rightBound = sortedIvs[0].left;
        desc.size = 1;
        descriptors.push_back(desc);
        return descriptors;
    }

    // Lemma 3.15: Every maximal clique corresponds to a non-leaf parent node p
    // and its rightmost child c in the distance tree, represented as [l_p, l_c].
    for (int i = 0; i < n; ++i) {
        int pId = sortedIvs[i].id;
        const auto& pNode = tree.getNodeById(pId);

        if (pNode.childrenIds.empty()) {
            continue; // Leaves do not originate maximal cliques
        }

        int cId = pNode.childrenIds.back(); // Rightmost child
        int cIdx = tree.getNodeById(cId).sortedIndex;

        TheoremCliqueDescriptor desc;
        desc.parentId = pId;
        desc.rightmostChildId = cId;
        desc.parentSortedIndex = i;
        desc.rightmostChildSortedIndex = cIdx;
        desc.leftBound = sortedIvs[i].left;
        desc.rightBound = sortedIvs[cIdx].left;
        desc.size = cIdx - i + 1;

        descriptors.push_back(desc);
    }

    return descriptors;
}

std::vector<std::vector<int>> TheoremCliquesAdapter::materializeCliques(
    const std::vector<TheoremCliqueDescriptor>& descriptors,
    const IntervalGraph& graph) {

    std::vector<std::vector<int>> cliques;
    cliques.reserve(descriptors.size());

    const auto& sortedIvs = graph.getSortedIntervals();

    for (const auto& desc : descriptors) {
        std::vector<int> clique;
        clique.reserve(desc.size);

        for (int idx = desc.parentSortedIndex; idx <= desc.rightmostChildSortedIndex; ++idx) {
            clique.push_back(sortedIvs[idx].id);
        }

        cliques.push_back(std::move(clique));
    }

    return cliques;
}

std::vector<CompactClique> TheoremCliquesAdapter::runApprovedPhase5Compact(
    const IntervalGraph& graph,
    const DistanceTree& tree) {
    return MaximalCliques::constructCompactCliques(graph, tree);
}

std::vector<std::vector<int>> TheoremCliquesAdapter::runApprovedPhase5Lemma3_8(
    const IntervalGraph& graph,
    const DistanceTree& tree) {
    auto allNodeCliques = MaximalCliques::constructAllNodeCliques(graph, tree);
    return MaximalCliques::extractUniqueMaximalCliques(allNodeCliques);
}
