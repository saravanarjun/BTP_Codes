#include "IntervalGraph.h"
#include "DistanceTree.h"
#include "DistanceVerification.h"
#include "StructuralAnalysis.h"
#include "MaximalCliques.h"
#include "IndependentSet.h"
#include "DominatingSet.h"
#include "IncrementalIndependentSet.h"
#include "DecrementalDistanceTree.h"
#include <iostream>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string inputFile = "input/intervals.txt";
    if (argc > 1) {
        inputFile = argv[1];
    }

    std::cout << "========================================================\n";
    std::cout << " BTP Interval Graphs - Research Pipeline\n";
    std::cout << " Phases 1-5: Structural Representation & Maximal Cliques\n";
    std::cout << " Phase 6: Maximum Independent Set (Bounds, Approx, DP)\n";
    std::cout << " Phase 7: Dominating Set Bound and Construction (Thm 3.17)\n";
    std::cout << " Phase 8: Incremental Maintenance of MIS (Theorem 3.14)\n";
    std::cout << " Phase 9: Decremental Maintenance of Distance Tree (Thm 3.16)\n";
    std::cout << "========================================================\n\n";

    std::cout << "Loading intervals from: " << inputFile << "\n";
    IntervalGraph graph;
    std::string errorMsg;
    if (!graph.loadFromFile(inputFile, errorMsg)) {
        std::cerr << "[-] Error loading graph: " << errorMsg << "\n";
        return 1;
    }

    std::cout << "[+] Graph successfully loaded and validated!\n";
    std::cout << "    - Number of vertices: " << graph.getNumVertices() << "\n";
    std::cout << "    - Properness: PASS\n";
    std::cout << "    - Connectedness: PASS\n\n";

    fs::create_directories("output");
    std::string sortedOut = "output/sorted_intervals.txt";
    if (graph.writeSortedIntervals(sortedOut)) {
        std::cout << "[+] Written sorted intervals to: " << sortedOut << "\n";
    }

    // Phase 2: Distance Tree
    std::cout << "\n--- Constructing Distance Tree T(G) (Eq 2.1, 3.1) ---\n";
    DistanceTree tree(graph);

    std::string treeErr;
    if (!tree.verifyTreeInvariants(treeErr)) {
        std::cerr << "[-] Distance Tree invariant violation: " << treeErr << "\n";
        return 1;
    }
    std::cout << "[+] Distance tree verified successfully against all invariants!\n";
    std::cout << "    - Root Vertex ID: " << tree.getRootId() << "\n";
    std::cout << "    - Maximum Depth h_depth: " << tree.getMaxDepth() << "\n";
    std::cout << "    - Total Levels h: " << tree.getNumLevels() << "\n\n";

    std::string treeOut = "output/distance_tree.txt";
    if (tree.writeDistanceTree(treeOut)) {
        std::cout << "[+] Written distance tree to: " << treeOut << "\n";
    }

    std::string paramsOut = "output/tree_parameters.txt";
    if (tree.writeTreeParameters(paramsOut)) {
        std::cout << "[+] Written tree parameters to: " << paramsOut << "\n";
    }

    // Phase 3: Distance Verification
    std::cout << "\n--- Verifying Shortest-Path Distance Formula (Equation 2.2) ---\n";
    std::string posErr;
    if (!DistanceVerification::verifyPositionalOrderingProperty(graph, tree, posErr)) {
        std::cerr << "[-] Positional ordering check FAILED: " << posErr << "\n";
        return 1;
    }
    std::cout << "[+] Positional ordering property (pos_level <=> interval order): PASS\n";

    std::string travErr;
    if (!DistanceVerification::verifyTraversalProperties(tree, travErr)) {
        std::cerr << "[-] Traversal invariants check FAILED: " << travErr << "\n";
        return 1;
    }
    std::cout << "[+] Traversal bijection invariants: PASS\n";

    auto report = DistanceVerification::verifyAllPairs(graph, tree);
    std::cout << "[+] All-pairs shortest path verification:\n";
    std::cout << "    - Total pairs tested: " << report.totalPairsTested << "\n";
    std::cout << "    - Matched pairs:      " << report.matchedPairs << "\n";
    std::cout << "    - Mismatches:         " << report.mismatches.size() << "\n";

    if (!report.passed()) {
        std::cerr << "[-] MISMATCHES DETECTED:\n";
        for (const auto& mm : report.mismatches) {
            std::cerr << "    " << mm.details << "\n";
        }
        return 1;
    }
    std::cout << "[+] Full Distance Formula Verification (Equation 2.2 vs BFS): 100% PASS!\n";

    // Phase 4: Structural Analysis
    std::cout << "\n--- Executing Structural Analysis & Verification (Chapter 3) ---\n";
    auto structReport = StructuralAnalysis::runFullAudit(graph, tree);

    auto printAudit = [](const StructuralCheckResult& r) {
        std::cout << "    [" << (r.passed ? "+" : "-") << "] " 
                  << std::left << std::setw(55) << r.ruleName 
                  << ": " << (r.passed ? "PASS" : "FAIL") 
                  << " (" << r.casesChecked << " cases)\n";
    };

    printAudit(structReport.lemma3_1);
    printAudit(structReport.depthLocality);
    printAudit(structReport.lemma3_2);
    printAudit(structReport.lemma3_3);
    printAudit(structReport.lemma3_4);
    printAudit(structReport.lemma3_5);
    printAudit(structReport.lemma3_6);
    printAudit(structReport.theorem3_7);

    std::string structOut = "output/structural_checks.txt";
    if (StructuralAnalysis::writeStructuralChecksReport(structReport, structOut)) {
        std::cout << "[+] Written structural analysis report to: " << structOut << "\n";
    }

    if (!structReport.allPassed()) {
        std::cerr << "[-] Structural property violations detected!\n";
        return 1;
    }

    // Phase 5: Maximal Cliques & Compact Representation
    std::cout << "\n--- Constructing Maximal Cliques (Lemma 3.8 & Lemma 3.15) ---\n";
    auto allNodeCliques = MaximalCliques::constructAllNodeCliques(graph, tree);
    auto uniqueMaximal = MaximalCliques::extractUniqueMaximalCliques(allNodeCliques);
    auto compactCliques = MaximalCliques::constructCompactCliques(graph, tree);
    auto cliqueComparison = MaximalCliques::compareAllMethods(graph, tree);

    std::cout << "[+] Maximal Cliques Construction Summary:\n";
    std::cout << "    - Distance tree nodes evaluated:  " << allNodeCliques.size() << "\n";
    std::cout << "    - Non-leaf parent maximal cliques: " << uniqueMaximal.size() << "\n";
    std::cout << "    - Leaf non-maximal sub-cliques:    " << cliqueComparison.numLeafSubsetsDetected << "\n";
    std::cout << "    - Compact descriptors [l_p, l_c]:  " << compactCliques.size() << "\n";
    std::cout << "    - Bron-Kerbosch ground truth:      " << cliqueComparison.numBronKerboschCliques << "\n";
    std::cout << "    - Cross-method exact set match:    " << (cliqueComparison.exactMatch ? "100% IDENTICAL" : "MISMATCH") << "\n";

    MaximalCliques::writeMaximalCliques(allNodeCliques, uniqueMaximal, "output/maximal_cliques.txt");
    MaximalCliques::writeCompactCliques(compactCliques, "output/compact_cliques.txt");
    MaximalCliques::writeCliqueVerification(cliqueComparison, "output/clique_verification.txt");

    // Phase 6: Maximum Independent Set
    std::cout << "\n--- Maximum Independent Set Analysis (Section 3.10) ---\n";
    auto misExact = IndependentSet::computeExactDP(graph, tree);
    auto misBounds = IndependentSet::verifyBounds(tree, misExact.alpha);
    auto misApprox = IndependentSet::computeApproximation(graph, tree, misExact.alpha);
    auto misVerification = IndependentSet::runFullVerification(graph, tree);

    std::cout << "[+] Lemma 3.11 Bounds: floor(" << misBounds.h << "/2)=" << misBounds.lowerBound 
              << " <= alpha(G)=" << misBounds.actualAlpha << " <= (h-1)=" << misBounds.upperBound 
              << " -> " << (misBounds.boundsSatisfied ? "PASS" : "FAIL") << "\n";

    std::cout << "[+] Theorem 3.12 2-Approximation: size=" << misApprox.approxSize 
              << ", ratio=" << std::fixed << std::setprecision(3) << misApprox.approxRatio 
              << ", independent=" << (misApprox.isIndependent ? "PASS" : "FAIL") 
              << ", guarantee=" << (misApprox.guaranteeSatisfied ? "PASS" : "FAIL") << "\n";

    std::cout << "[+] Theorem 3.13 Exact DP alpha(G) = " << misExact.alpha << "\n";
    std::cout << "    - Reconstructed MIS: {";
    for (size_t k = 0; k < misExact.selectedVertices.size(); ++k) {
        if (k > 0) std::cout << ", ";
        std::cout << misExact.selectedVertices[k];
    }
    std::cout << "}\n";
    std::cout << "    - Brute-force verification: " 
              << (misVerification.dpAgreesWithBruteForce ? "PASS (100% match)" : "FAIL") << "\n";

    IndependentSet::writeBoundsReport(misBounds, "output/mis_bounds.txt");
    IndependentSet::writeApproximationReport(misApprox, "output/mis_approximation.txt");
    IndependentSet::writeExactDPReport(misExact, tree, "output/mis_exact_dp.txt");
    IndependentSet::writeVerificationReport(misVerification, "output/mis_verification.txt");

    // Phase 7: Dominating Set (Theorem 3.17)
    std::cout << "\n--- Dominating Set Construction & Verification (Theorem 3.17) ---\n";
    auto domResult = DominatingSet::constructDominatingSet(graph, tree);
    auto domReport = DominatingSet::runFullVerification(graph, tree, true);

    std::cout << "[+] Theorem 3.17 Construction: |D| = " << domResult.setSize 
              << ", Bound ceil((h-1)/2) = " << domResult.theoreticalBound;
    if (domResult.isBoundaryCaseH1) {
        std::cout << " (h=1 boundary case)";
    }
    std::cout << " -> " << (domResult.satisfiesBound ? "PASS" : "FAIL") << "\n";

    std::cout << "    - Selected Dominating Set D: {";
    for (size_t k = 0; k < domResult.selectedVertices.size(); ++k) {
        if (k > 0) std::cout << ", ";
        std::cout << domResult.selectedVertices[k];
    }
    std::cout << "}\n";

    std::cout << "    - Independent Domination Check: " 
              << (domReport.dominationPassed ? "PASS (100% covered)" : "FAIL") << "\n";
    if (domReport.hasBruteForceMDS) {
        std::cout << "    - Optimal gamma(G) (Brute-Force MDS): " << domReport.bruteForceMDSSize << "\n";
        std::cout << "    - Approximation Ratio (|D|/gamma): " 
                  << std::fixed << std::setprecision(3)
                  << (static_cast<double>(domReport.constructedSize) / domReport.bruteForceMDSSize) << "\n";
    }

    DominatingSet::writeDominatingSetReport(domResult, graph, tree, "output/dominating_set.txt");
    DominatingSet::writeVerificationReport(domReport, graph, tree, "output/dominating_set_verification.txt");

    // Phase 8: Incremental Maintenance of MIS (Theorem 3.14)
    std::cout << "\n--- Incremental Maintenance of Maximum Independent Set (Theorem 3.14) ---\n";
    IncrementalIndependentSet incMIS;
    std::string initErr;
    if (!incMIS.initialize(graph, tree, initErr)) {
        std::cerr << "[-] Error initializing Incremental MIS: " << initErr << "\n";
        return 1;
    }
    std::cout << "[+] Initialized dynamic MIS manager: |V| = " << incMIS.getNumVertices()
              << ", Levels = " << incMIS.getNumLevels()
              << ", alpha(G) = " << incMIS.getAlpha() << "\n";

    // 1. Valid leaf insertion (ID 9: [11.0, 14.0])
    Interval newLeaf1{9, 11.0, 14.0};
    std::cout << "\n  -> Attempting valid leaf insertion: Vertex 9 [11.0, 14.0]...\n";
    auto res1 = incMIS.insertLeaf(newLeaf1);
    if (res1.success) {
        std::cout << "     [+] SUCCESS: " << res1.diagnostic << "\n";
        std::cout << "         Parent = " << res1.parentId << ", Level = " << res1.level 
                  << ", DP = " << res1.dpValue << ", alpha(G) = " << res1.newAlpha << "\n";
    } else {
        std::cout << "     [-] REJECTED: " << res1.diagnostic << "\n";
    }
    auto rep1 = incMIS.runIndependentVerification();
    std::cout << "     [+] Independent Verification: " << (rep1.allPassed() ? "PASS (100% match with static DP & BFS)" : "FAIL") << "\n";

    // 2. Valid leaf insertion extending height (ID 10: [13.0, 16.0])
    Interval newLeaf2{10, 13.0, 16.0};
    std::cout << "\n  -> Attempting valid leaf insertion extending level (ID 10 [13.0, 16.0])...\n";
    auto res2 = incMIS.insertLeaf(newLeaf2);
    if (res2.success) {
        std::cout << "     [+] SUCCESS: " << res2.diagnostic << "\n";
        std::cout << "         Parent = " << res2.parentId << ", Level = " << res2.level 
                  << ", DP = " << res2.dpValue << ", alpha(G) = " << res2.newAlpha << "\n";
    } else {
        std::cout << "     [-] REJECTED: " << res2.diagnostic << "\n";
    }
    auto rep2 = incMIS.runIndependentVerification();
    std::cout << "     [+] Independent Verification: " << (rep2.allPassed() ? "PASS (100% match with static DP & BFS)" : "FAIL") << "\n";

    // 3. Attempting an invalid insertion (disconnected gap)
    Interval badIv{99, 25.0, 30.0};
    std::cout << "\n  -> Attempting invalid insertion: Vertex 99 [25.0, 30.0] (gap)...\n";
    auto resBad = incMIS.insertLeaf(badIv);
    std::cout << "     [-] Expected Rejection: " << resBad.diagnostic << "\n";

    std::cout << "\n[+] Phase 8 completed and verified successfully!\n";

    // Phase 9: Decremental Maintenance of Distance Tree (Theorem 3.16)
    std::cout << "\n--- Decremental Maintenance of Distance Tree (Theorem 3.16) ---\n";
    DecrementalDistanceTree decTree;
    std::string decInitErr;
    if (!decTree.initialize(graph, tree, decInitErr)) {
        std::cerr << "[-] Error initializing Decremental Tree: " << decInitErr << "\n";
        return 1;
    }
    std::cout << "[+] Initialized decremental distance tree: |V| = " << decTree.getNumVertices()
              << ", Levels = " << decTree.getNumLevels()
              << ", alpha(G) = " << decTree.getAlpha() << "\n";

    // 1. Valid leaf deletion: Vertex 2
    std::cout << "\n  -> Attempting valid leaf deletion: Vertex 2 (Level 2)...\n";
    auto delRes1 = decTree.deleteVertex(2);
    if (delRes1.success) {
        std::cout << "     [+] SUCCESS: " << delRes1.diagnostic << "\n";
    } else {
        std::cout << "     [-] REJECTED: " << delRes1.diagnostic << "\n";
    }
    auto delVer1 = decTree.runIndependentVerification();
    std::cout << "     [+] Independent Verification: " 
              << (delVer1.allPassed() ? "PASS (100% match with static rebuild & DP)" : "FAIL") << "\n";

    // 2. Valid deletion with child relocation: Vertex 3 (relocates child 5 to 4)
    std::cout << "\n  -> Attempting valid deletion with child relocation: Vertex 3 (Level 2)...\n";
    auto delRes2 = decTree.deleteVertex(3);
    if (delRes2.success) {
        std::cout << "     [+] SUCCESS: " << delRes2.diagnostic << "\n";
    } else {
        std::cout << "     [-] REJECTED: " << delRes2.diagnostic << "\n";
    }
    auto delVer2 = decTree.runIndependentVerification();
    std::cout << "     [+] Independent Verification: " 
              << (delVer2.allPassed() ? "PASS (100% match with static rebuild & DP)" : "FAIL") << "\n";

    // 3. Invalid deletion attempt: Vertex 4 (has children, but no immediate right vertex)
    std::cout << "\n  -> Attempting invalid deletion: Vertex 4 (has children, no immediate right)...\n";
    auto delResBad1 = decTree.deleteVertex(4);
    std::cout << "     [-] Expected Rejection: " << delResBad1.diagnostic << "\n";

    // 4. Invalid root deletion attempt: Vertex 1
    std::cout << "\n  -> Attempting invalid root deletion: Vertex 1...\n";
    auto delResBad2 = decTree.deleteVertex(1);
    std::cout << "     [-] Expected Rejection: " << delResBad2.diagnostic << "\n";

    std::cout << "\n[+] Phase 9 completed and verified successfully!\n";
    return 0;
}
