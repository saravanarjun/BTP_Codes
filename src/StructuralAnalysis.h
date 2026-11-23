#ifndef STRUCTURAL_ANALYSIS_H
#define STRUCTURAL_ANALYSIS_H

#include "IntervalGraph.h"
#include "DistanceTree.h"
#include <string>
#include <vector>

struct StructuralCheckResult {
    std::string ruleName;
    std::string thesisReference;
    bool passed;
    long long casesChecked;
    std::vector<std::string> failureDetails;
};

struct StructuralReport {
    StructuralCheckResult lemma3_1;       // Parent-children clique
    StructuralCheckResult depthLocality;  // Depth locality of edges (|depth(u)-depth(v)| <= 1)
    StructuralCheckResult lemma3_2;       // Child not adjacent to parent's left sibling
    StructuralCheckResult lemma3_3;       // Child adjacent to parent's right siblings
    StructuralCheckResult lemma3_4;       // Node, children, and right siblings form a clique
    StructuralCheckResult lemma3_5;       // Mutual adjacency of siblings' children
    StructuralCheckResult lemma3_6;       // Propagation of adjacency to children
    StructuralCheckResult theorem3_7;     // Each depth level induces a clique

    bool allPassed() const {
        return lemma3_1.passed && depthLocality.passed && lemma3_2.passed &&
               lemma3_3.passed && lemma3_4.passed && lemma3_5.passed &&
               lemma3_6.passed && theorem3_7.passed;
    }
};

class StructuralAnalysis {
public:
    // Individual verifiers for each lemma and theorem
    static StructuralCheckResult verifyLemma3_1(const IntervalGraph& graph, const DistanceTree& tree);
    static StructuralCheckResult verifyDepthLocality(const IntervalGraph& graph, const DistanceTree& tree);
    static StructuralCheckResult verifyLemma3_2(const IntervalGraph& graph, const DistanceTree& tree);
    static StructuralCheckResult verifyLemma3_3(const IntervalGraph& graph, const DistanceTree& tree);
    static StructuralCheckResult verifyLemma3_4(const IntervalGraph& graph, const DistanceTree& tree);
    static StructuralCheckResult verifyLemma3_5(const IntervalGraph& graph, const DistanceTree& tree);
    static StructuralCheckResult verifyLemma3_6(const IntervalGraph& graph, const DistanceTree& tree);
    static StructuralCheckResult verifyTheorem3_7(const IntervalGraph& graph, const DistanceTree& tree);

    // Run complete structural audit suite
    static StructuralReport runFullAudit(const IntervalGraph& graph, const DistanceTree& tree);

    // Export structural checks report to output/structural_checks.txt
    static bool writeStructuralChecksReport(const StructuralReport& report, const std::string& filepath);
};

#endif // STRUCTURAL_ANALYSIS_H

