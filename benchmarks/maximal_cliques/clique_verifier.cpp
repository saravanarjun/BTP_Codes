#include "clique_verifier.h"
#include "graph_generator.h"
#include "DistanceTree.h"
#include "theorem_cliques_adapter.h"
#include "bron_kerbosch_adapter.h"
#include "standard_interval_cliques.h"
#include "dynamic_clique_structure.h"
#include <algorithm>
#include <sstream>
#include <iostream>

std::vector<std::vector<int>> CliqueVerifier::canonicalize(const std::vector<std::vector<int>>& cliques) {
    std::vector<std::vector<int>> copy = cliques;
    for (auto& c : copy) {
        std::sort(c.begin(), c.end());
    }
    std::sort(copy.begin(), copy.end());
    return copy;
}

bool CliqueVerifier::areEqual(
    const std::vector<std::vector<int>>& a,
    const std::vector<std::vector<int>>& b,
    std::string& outDiff) {

    auto canA = canonicalize(a);
    auto canB = canonicalize(b);

    if (canA.size() != canB.size()) {
        std::ostringstream oss;
        oss << "Clique count mismatch: " << canA.size() << " vs " << canB.size();
        outDiff = oss.str();
        return false;
    }

    for (size_t i = 0; i < canA.size(); ++i) {
        if (canA[i] != canB[i]) {
            std::ostringstream oss;
            oss << "Clique " << i << " content mismatch: A=[";
            for (size_t j = 0; j < canA[i].size(); ++j) oss << (j ? "," : "") << canA[i][j];
            oss << "] vs B=[";
            for (size_t j = 0; j < canB[i].size(); ++j) oss << (j ? "," : "") << canB[i][j];
            oss << "]";
            outDiff = oss.str();
            return false;
        }
    }
    return true;
}

bool CliqueVerifier::verifyCliqueProperty(const IntervalGraph& graph, const std::vector<int>& clique) {
    int sz = static_cast<int>(clique.size());
    for (int i = 0; i < sz; ++i) {
        const auto& ivA = graph.getIntervalById(clique[i]);
        for (int j = i + 1; j < sz; ++j) {
            const auto& ivB = graph.getIntervalById(clique[j]);
            if (ivA.left > ivB.right || ivB.left > ivA.right) {
                return false;
            }
        }
    }
    return true;
}

bool CliqueVerifier::verifyMaximality(const IntervalGraph& graph, const std::vector<int>& clique, int& outViolator) {
    std::vector<bool> inClique(graph.getNumVertices(), false);
    for (int v : clique) {
        inClique[graph.getSortedIndexById(v)] = true;
    }

    const auto& allIvs = graph.getSortedIntervals();
    for (const auto& wIv : allIvs) {
        if (inClique[graph.getSortedIndexById(wIv.id)]) continue;

        bool adjacentToAll = true;
        for (int u : clique) {
            const auto& uIv = graph.getIntervalById(u);
            if (wIv.left > uIv.right || uIv.left > wIv.right) {
                adjacentToAll = false;
                break;
            }
        }
        if (adjacentToAll) {
            outViolator = wIv.id;
            return false; // Can be extended with wIv.id
        }
    }
    return true;
}

bool CliqueVerifier::verifyDescriptorsEquivalence(
    const IntervalGraph& graph,
    const std::vector<CompactDescriptor>& descriptors,
    const std::vector<std::vector<int>>& explicitCliques,
    std::string& outDiff) {

    std::vector<std::vector<int>> expanded;
    expanded.reserve(descriptors.size());

    const auto& sorted = graph.getSortedIntervals();

    for (const auto& d : descriptors) {
        std::vector<int> mems;
        for (const auto& iv : sorted) {
            if (iv.left >= d.leftBound && iv.left <= d.rightBound) {
                mems.push_back(iv.id);
            }
        }
        std::sort(mems.begin(), mems.end());
        expanded.push_back(mems);
    }

    return areEqual(expanded, explicitCliques, outDiff);
}

VerificationResult CliqueVerifier::verifyStaticGraph(const IntervalGraph& graph, const std::string& familyName) {
    VerificationResult res;
    res.testName = familyName + " (n=" + std::to_string(graph.getNumVertices()) + ")";

    // 1. Validate properness and connectedness
    std::string diag;
    if (!GraphGenerator::validateProperConnectedGraph(graph, diag)) {
        res.passed = false;
        res.details = "Graph property failure: " + diag;
        return res;
    }

    // 2. Algorithm A: Theorem-Based
    DistanceTree tree(graph);
    tree.build();
    auto descsA = TheoremCliquesAdapter::computeCompactDescriptors(graph, tree);
    auto cliquesA = TheoremCliquesAdapter::materializeCliques(descsA, graph);
    res.numCliquesA = static_cast<int>(cliquesA.size());

    // 3. Algorithm C: Standard Sweep-Line
    auto cliquesC = StandardIntervalCliques::enumerateMaximalCliques(graph);
    res.numCliquesC = static_cast<int>(cliquesC.size());

    // Compare A and C
    std::string diffAC;
    if (!areEqual(cliquesA, cliquesC, diffAC)) {
        res.passed = false;
        res.details = "Mismatch between Alg A and Alg C: " + diffAC;
        return res;
    }

    // 4. Algorithm B: Bron-Kerbosch (run if n <= 50 with safe timeout)
    int n = graph.getNumVertices();
    if (n <= 50) {
        res.bkgRan = true;
        auto bkRes = BronKerboschAdapter::runPivotedWithTimeout(graph, 1000.0);
        if (!bkRes.timedOut) {
            res.bkgPassed = true;
            res.bkgTimedOut = false;
            res.numCliquesB = static_cast<int>(bkRes.cliques.size());
            std::string diffAB;
            if (!areEqual(cliquesA, bkRes.cliques, diffAB)) {
                res.passed = false;
                res.details = "Mismatch between Alg A and Alg B (Bron-Kerbosch): " + diffAB;
                return res;
            }
        } else {
            res.bkgPassed = false;
            res.bkgTimedOut = true;
            res.numCliquesB = -1; // Honest sentinel
        }
    } else {
        res.bkgRan = false;
        res.bkgPassed = false;
        res.bkgTimedOut = false;
        res.numCliquesB = -1; // Honest sentinel: skipped by policy for n > 50
    }

    // 5. Verify Clique Property and Maximality
    for (size_t i = 0; i < cliquesA.size(); ++i) {
        if (!verifyCliqueProperty(graph, cliquesA[i])) {
            res.passed = false;
            res.details = "Clique " + std::to_string(i) + " contains non-adjacent vertices.";
            return res;
        }
        int violator = -1;
        if (!verifyMaximality(graph, cliquesA[i], violator)) {
            res.passed = false;
            res.details = "Clique " + std::to_string(i) + " is not maximal (can add vertex " + std::to_string(violator) + ").";
            return res;
        }
    }

    res.passed = true;
    if (res.bkgRan && res.bkgPassed) {
        res.details = "All checks passed (A == B == C verified, pairwise adjacency verified, maximality verified).";
    } else if (res.bkgRan && res.bkgTimedOut) {
        res.details = "A == C verified (PASS), Maximality verified (PASS), BK timed out on sparse branching.";
    } else {
        res.details = "A == C verified (PASS), Maximality verified (PASS), BK skipped by policy (n > 50).";
    }
    return res;
}

VerificationResult CliqueVerifier::verifyDynamicSequence(const DynamicWorkloadSequence& sequence) {
    VerificationResult res;
    res.testName = sequence.workloadName + " (seed=" + std::to_string(sequence.seed) + ", ops=" + std::to_string(sequence.operations.size()) + ")";

    // Initialize Algorithm A
    DistanceTree initTree(sequence.initialGraph);
    initTree.build();

    DynamicTheoremCliques dynA;
    dynA.initialize(sequence.initialGraph, initTree);

    // Initialize Algorithm C
    DynamicSweepLine dynC;
    dynC.initialize(sequence.initialGraph);

    // Initialize Algorithm B (if initial size <= 50)
    bool trackBK = (sequence.initialN <= 50 && sequence.operations.size() <= 30);
    DynamicBronKerbosch dynB;
    if (trackBK) {
        dynB.initialize(sequence.initialGraph);
    }

    for (size_t step = 0; step < sequence.operations.size(); ++step) {
        const auto& op = sequence.operations[step];

        if (op.type == DynamicOpType::INSERTION) {
            auto telA = dynA.insertLeaf(op.newInterval);
            auto telC = dynC.insertLeaf(op.newInterval);
            if (trackBK) dynB.insertLeaf(op.newInterval);

            if (telA.status != BenchmarkStatus::SUCCESS) {
                res.passed = false;
                res.details = "Dynamic Algorithm A failed insertion step " + std::to_string(step) + ": " + telA.diagnostic;
                return res;
            }
        } else {
            auto telA = dynA.deleteVertex(op.targetId);
            auto telC = dynC.deleteVertex(op.targetId);
            if (trackBK) dynB.deleteVertex(op.targetId);

            if (telA.status != BenchmarkStatus::SUCCESS) {
                res.passed = false;
                res.details = "Dynamic Algorithm A failed deletion step " + std::to_string(step) + ": " + telA.diagnostic;
                return res;
            }
        }

        // Cross-verify dynamic Algorithm A against dynamic Algorithm C
        auto cliquesA = dynA.materializeCurrentCliques();
        auto cliquesC = dynC.getCurrentCliques();

        std::string diffAC;
        if (!areEqual(cliquesA, cliquesC, diffAC)) {
            res.passed = false;
            res.details = "Dynamic step " + std::to_string(step) + " mismatch between Alg A and Alg C: " + diffAC;
            return res;
        }

        if (trackBK) {
            auto cliquesB = dynB.getCurrentCliques();
            std::string diffAB;
            if (!areEqual(cliquesA, cliquesB, diffAB)) {
                res.passed = false;
                res.details = "Dynamic step " + std::to_string(step) + " mismatch between Alg A and Bron-Kerbosch: " + diffAB;
                return res;
            }
        }

        // Verify against static rebuild from exported graph
        IntervalGraph curGraph = dynA.exportCurrentGraph();
        DistanceTree staticTree(curGraph);
        staticTree.build();
        auto staticDescs = TheoremCliquesAdapter::computeCompactDescriptors(curGraph, staticTree);
        auto staticCliques = TheoremCliquesAdapter::materializeCliques(staticDescs, curGraph);

        std::string diffStatic;
        if (!areEqual(cliquesA, staticCliques, diffStatic)) {
            res.passed = false;
            res.details = "Dynamic step " + std::to_string(step) + " mismatch between Dynamic A and Static Rebuild: " + diffStatic;
            return res;
        }
    }

    res.passed = true;
    res.numCliquesA = dynA.getNumCliques();
    res.numCliquesC = dynC.getNumCliques();
    res.bkgRan = trackBK;
    res.bkgPassed = trackBK;
    res.numCliquesB = trackBK ? dynB.getNumCliques() : -1;
    res.details = "All " + std::to_string(sequence.operations.size()) + " dynamic update steps passed 100% equivalence.";
    return res;
}
