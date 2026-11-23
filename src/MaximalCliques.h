#ifndef MAXIMAL_CLIQUES_H
#define MAXIMAL_CLIQUES_H

#include "IntervalGraph.h"
#include "DistanceTree.h"
#include <vector>
#include <string>

// Explicit clique constructed for each distance-tree node using Lemma 3.8
struct NodeClique {
    int originatingNodeId;
    bool isOriginatingNodeLeaf;
    std::vector<int> members;           // Sorted deterministically by canonical interval ordering
    bool isClique;                      // Pairwise adjacency check
    bool isMaximal;                     // Maximality check (cannot be extended)
    std::vector<int> extendableWith;    // Vertices outside C(u) that can be added if not maximal
};

// Compact representation of maximal cliques using Lemma 3.15: [l_p, l_c]
struct CompactClique {
    int parentId;
    int rightmostChildId;
    double leftBound;                   // l_p
    double rightBound;                  // l_c
    std::vector<int> expandedMembers;   // All vertices with l_v in [l_p, l_c]
    bool isClique;
    bool isMaximal;
};

// Cross-validation comparison report
struct CliqueComparisonReport {
    bool exactMatch;
    size_t numBronKerboschCliques;
    size_t numLemma3_8TotalConstructed;
    size_t numLemma3_8UniqueMaximal;
    size_t numCompactCliques;
    size_t numLeafSubsetsDetected;
    std::vector<std::string> discrepancies;
};

class MaximalCliques {
public:
    // Lemma 3.8: Construct C(u) for a given distance-tree node u
    // 1. u itself
    // 2. all children of u
    // 3. all nodes at same depth as u to the right of u
    // 4. all children of nodes at same depth as u to the left of u
    static NodeClique constructCliqueLemma3_8(const IntervalGraph& graph, const DistanceTree& tree, int uId);

    // Construct C(u) for every node in T(G)
    static std::vector<NodeClique> constructAllNodeCliques(const IntervalGraph& graph, const DistanceTree& tree);

    // Filter and deduplicate to obtain unique maximal cliques from Lemma 3.8
    static std::vector<std::vector<int>> extractUniqueMaximalCliques(const std::vector<NodeClique>& allCliques);

    // Lemma 3.15: Construct compact representation [l_p, l_c] for each parent node
    static std::vector<CompactClique> constructCompactCliques(const IntervalGraph& graph, const DistanceTree& tree);

    // Independent Ground Truth: Bron-Kerbosch maximal clique enumeration on interval graph G
    static std::vector<std::vector<int>> enumerateBronKerbosch(const IntervalGraph& graph);

    // Verification predicates using interval adjacency directly
    static bool verifyCliqueProperty(const IntervalGraph& graph, const std::vector<int>& clique);
    static bool verifyMaximality(const IntervalGraph& graph, const std::vector<int>& clique, std::vector<int>& extendableWith);

    // Comprehensive comparison across Lemma 3.8, Lemma 3.15, and Bron-Kerbosch
    static CliqueComparisonReport compareAllMethods(const IntervalGraph& graph, const DistanceTree& tree);

    // Output exporters
    static bool writeMaximalCliques(const std::vector<NodeClique>& allNodeCliques, 
                                   const std::vector<std::vector<int>>& uniqueMaximal, 
                                   const std::string& filepath);
    static bool writeCompactCliques(const std::vector<CompactClique>& compactCliques, const std::string& filepath);
    static bool writeCliqueVerification(const CliqueComparisonReport& report, const std::string& filepath);

private:
    static void bronKerboschRecursive(std::vector<int>& R, std::vector<int>& P, std::vector<int>& X,
                                      const IntervalGraph& graph, std::vector<std::vector<int>>& result);
};

#endif // MAXIMAL_CLIQUES_H

