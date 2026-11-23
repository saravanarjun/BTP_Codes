#ifndef CLIQUE_VERIFIER_H
#define CLIQUE_VERIFIER_H

#include "clique_types.h"
#include "IntervalGraph.h"
#include "dynamic_workload_generator.h"
#include <vector>
#include <string>

struct VerificationResult {
    bool passed = true;
    std::string testName;
    std::string details;
    int numCliquesA = 0;
    int numCliquesB = 0;
    int numCliquesC = 0;
    bool bkgRan = false;
    bool bkgPassed = false;
    bool bkgTimedOut = false;
};

class CliqueVerifier {
public:
    // Canonicalize a collection of cliques: sort vertex IDs, sort cliques lexicographically
    static std::vector<std::vector<int>> canonicalize(const std::vector<std::vector<int>>& cliques);

    // Exact equality check between two clique sets
    static bool areEqual(
        const std::vector<std::vector<int>>& a,
        const std::vector<std::vector<int>>& b,
        std::string& outDiff);

    // Verify that every pair of vertices in the clique intersects
    static bool verifyCliqueProperty(const IntervalGraph& graph, const std::vector<int>& clique);

    // Verify maximality: no vertex in V \ clique can be added
    static bool verifyMaximality(const IntervalGraph& graph, const std::vector<int>& clique, int& outViolator);

    // Verify compact descriptors against explicit membership
    static bool verifyDescriptorsEquivalence(
        const IntervalGraph& graph,
        const std::vector<CompactDescriptor>& descriptors,
        const std::vector<std::vector<int>>& explicitCliques,
        std::string& outDiff);

    // Static verification suite: cross-verifies Alg A, B (if n <= 100), and C
    static VerificationResult verifyStaticGraph(const IntervalGraph& graph, const std::string& familyName);

    // Dynamic verification suite: verifies each step of an update sequence
    static VerificationResult verifyDynamicSequence(const DynamicWorkloadSequence& sequence);
};

#endif // CLIQUE_VERIFIER_H
