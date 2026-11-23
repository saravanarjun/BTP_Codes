#include "StructuralAnalysis.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cassert>

StructuralCheckResult StructuralAnalysis::verifyLemma3_1(const IntervalGraph& graph, const DistanceTree& tree) {
    StructuralCheckResult res;
    res.ruleName = "Lemma 3.1: Parent-Children Clique";
    res.thesisReference = "Chapter 3, Section 3.10, Lemma 3.1 (Page 24)";
    res.passed = true;
    res.casesChecked = 0;

    for (const auto& node : tree.getAllNodes()) {
        std::vector<int> cliqueMembers;
        cliqueMembers.push_back(node.vertexId);
        for (int cId : node.childrenIds) {
            cliqueMembers.push_back(cId);
        }

        // Verify pairwise adjacency for all pairs in {u} U Children(u)
        size_t m = cliqueMembers.size();
        for (size_t i = 0; i < m; ++i) {
            for (size_t j = i + 1; j < m; ++j) {
                res.casesChecked++;
                int u = cliqueMembers[i];
                int v = cliqueMembers[j];
                if (!graph.isAdjacent(u, v)) {
                    res.passed = false;
                    std::ostringstream oss;
                    oss << "Pair (" << u << ", " << v << ") in parent-children set of node " 
                        << node.vertexId << " is not adjacent in G.";
                    res.failureDetails.push_back(oss.str());
                }
            }
        }
    }

    return res;
}

StructuralCheckResult StructuralAnalysis::verifyDepthLocality(const IntervalGraph& graph, const DistanceTree& tree) {
    StructuralCheckResult res;
    res.ruleName = "Depth Locality of Non-Level Edges";
    res.thesisReference = "Chapter 3, Section 3.10, Observation (Page 26)";
    res.passed = true;
    res.casesChecked = 0;

    int n = graph.getNumVertices();
    const auto& sortedIvs = graph.getSortedIntervals();

    for (int i = 0; i < n; ++i) {
        int uId = sortedIvs[i].id;
        int uDepth = tree.getNodeById(uId).depth;

        for (int j = i + 1; j < n; ++j) {
            int vId = sortedIvs[j].id;
            if (graph.isAdjacent(uId, vId)) {
                res.casesChecked++;
                int vDepth = tree.getNodeById(vId).depth;
                int diff = std::abs(uDepth - vDepth);
                if (diff > 1) {
                    res.passed = false;
                    std::ostringstream oss;
                    oss << "Edge (" << uId << ", " << vId << ") violates depth locality: depth("
                        << uId << ") = " << uDepth << ", depth(" << vId << ") = " << vDepth 
                        << ", difference = " << diff << " > 1.";
                    res.failureDetails.push_back(oss.str());
                }
            }
        }
    }

    return res;
}

StructuralCheckResult StructuralAnalysis::verifyLemma3_2(const IntervalGraph& graph, const DistanceTree& tree) {
    StructuralCheckResult res;
    res.ruleName = "Lemma 3.2: Child Cannot Be Adjacent to Parent's Left Sibling";
    res.thesisReference = "Chapter 3, Section 3.10, Lemma 3.2 (Page 26)";
    res.passed = true;
    res.casesChecked = 0;

    // For every non-root vertex v:
    // Let p = parent(v).
    // Let q be any left sibling of p (both true sibling sharing parent(p),
    // and any peer at same depth to the left of p in level ordering).
    for (const auto& vNode : tree.getAllNodes()) {
        if (vNode.depth == 0) continue; // Skip root

        int pId = vNode.parentId;
        const auto& pNode = tree.getNodeById(pId);
        int pDepth = pNode.depth;

        // Examine all vertices in level pDepth occurring to the left of p
        const auto& levelVertices = tree.getLevelVertices(pDepth);
        for (int qId : levelVertices) {
            const auto& qNode = tree.getNodeById(qId);
            if (qNode.posLevel >= pNode.posLevel) {
                break; // q is not to the left of p
            }

            res.casesChecked++;
            // Check non-adjacency: qv must NOT be in E
            if (graph.isAdjacent(qId, vNode.vertexId)) {
                res.passed = false;
                std::ostringstream oss;
                oss << "Child " << vNode.vertexId << " of parent " << pId 
                    << " is adjacent to left sibling/peer " << qId << " at depth " << pDepth 
                    << " (contradicts parent optimality).";
                res.failureDetails.push_back(oss.str());
            }
        }
    }

    return res;
}

StructuralCheckResult StructuralAnalysis::verifyLemma3_3(const IntervalGraph& graph, const DistanceTree& tree) {
    StructuralCheckResult res;
    res.ruleName = "Lemma 3.3: Adjacency with Parent's Right Siblings";
    res.thesisReference = "Chapter 3, Section 3.10, Lemma 3.3 (Page 27)";
    res.passed = true;
    res.casesChecked = 0;

    // For every node u in T(G):
    // For every child v of u:
    // For every right sibling w of u:
    // v must be adjacent to w in G.
    for (const auto& uNode : tree.getAllNodes()) {
        if (uNode.childrenIds.empty()) continue;

        int uDepth = uNode.depth;
        const auto& levelVertices = tree.getLevelVertices(uDepth);

        // Find right peers at the same depth
        std::vector<int> rightSiblings;
        for (int wId : levelVertices) {
            const auto& wNode = tree.getNodeById(wId);
            if (wNode.posLevel > uNode.posLevel) {
                // If u is not root, also verify true sibling condition (sharing parent)
                // Both true siblings and right peers are checked.
                rightSiblings.push_back(wId);
            }
        }

        for (int vId : uNode.childrenIds) {
            for (int wId : rightSiblings) {
                res.casesChecked++;
                if (!graph.isAdjacent(vId, wId)) {
                    res.passed = false;
                    std::ostringstream oss;
                    oss << "Child " << vId << " of node " << uNode.vertexId 
                        << " is not adjacent to right sibling/peer " << wId 
                        << " at depth " << uDepth << ".";
                    res.failureDetails.push_back(oss.str());
                }
            }
        }
    }

    return res;
}

StructuralCheckResult StructuralAnalysis::verifyLemma3_4(const IntervalGraph& graph, const DistanceTree& tree) {
    StructuralCheckResult res;
    res.ruleName = "Lemma 3.4: Node, Children, and Right Siblings Form a Clique";
    res.thesisReference = "Chapter 3, Section 3.10, Lemma 3.4 (Page 28)";
    res.passed = true;
    res.casesChecked = 0;

    // S = {u} U Children(u) U RightSiblings(u)
    for (const auto& uNode : tree.getAllNodes()) {
        std::vector<int> S;
        S.push_back(uNode.vertexId);
        for (int cId : uNode.childrenIds) {
            S.push_back(cId);
        }

        // True right siblings of u (children of parent(u) occurring to the right of u)
        if (uNode.depth > 0) {
            const auto& pNode = tree.getNodeById(uNode.parentId);
            for (int sibId : pNode.childrenIds) {
                if (tree.getNodeById(sibId).posLevel > uNode.posLevel) {
                    S.push_back(sibId);
                }
            }
        }

        // Verify pairwise adjacency for all elements in S
        size_t m = S.size();
        for (size_t i = 0; i < m; ++i) {
            for (size_t j = i + 1; j < m; ++j) {
                res.casesChecked++;
                int x = S[i];
                int y = S[j];
                if (!graph.isAdjacent(x, y)) {
                    res.passed = false;
                    std::ostringstream oss;
                    oss << "Pair (" << x << ", " << y << ") in {u} U Children(u) U RightSiblings(u) "
                        << "for node " << uNode.vertexId << " is not adjacent in G.";
                    res.failureDetails.push_back(oss.str());
                }
            }
        }
    }

    return res;
}

StructuralCheckResult StructuralAnalysis::verifyLemma3_5(const IntervalGraph& graph, const DistanceTree& tree) {
    StructuralCheckResult res;
    res.ruleName = "Lemma 3.5: Mutual Adjacency of Siblings' Children";
    res.thesisReference = "Chapter 3, Section 3.10, Lemma 3.5 (Page 28)";
    res.passed = true;
    res.casesChecked = 0;

    // For each node that has at least two children:
    for (const auto& pNode : tree.getAllNodes()) {
        const auto& sibs = pNode.childrenIds;
        size_t numSibs = sibs.size();
        if (numSibs < 2) continue;

        for (size_t i = 0; i < numSibs; ++i) {
            int vId = sibs[i]; // Left sibling
            const auto& vNode = tree.getNodeById(vId);

            for (size_t j = i + 1; j < numSibs; ++j) {
                int uId = sibs[j]; // Right sibling
                const auto& uNode = tree.getNodeById(uId);

                // For every child x of v and child w of u:
                for (int xId : vNode.childrenIds) {
                    for (int wId : uNode.childrenIds) {
                        res.casesChecked++;
                        if (!graph.isAdjacent(xId, wId)) {
                            res.passed = false;
                            std::ostringstream oss;
                            oss << "Child " << xId << " of left sibling " << vId 
                                << " is not adjacent to child " << wId << " of right sibling " << uId 
                                << " (parent = " << pNode.vertexId << ").";
                            res.failureDetails.push_back(oss.str());
                        }
                    }
                }
            }
        }
    }

    return res;
}

StructuralCheckResult StructuralAnalysis::verifyLemma3_6(const IntervalGraph& graph, const DistanceTree& tree) {
    StructuralCheckResult res;
    res.ruleName = "Lemma 3.6: Propagation of Adjacency to Children";
    res.thesisReference = "Chapter 3, Section 3.10, Lemma 3.6 (Page 29)";
    res.passed = true;
    res.casesChecked = 0;

    // For every depth level d:
    for (int d = 0; d <= tree.getMaxDepth(); ++d) {
        const auto& levelVertices = tree.getLevelVertices(d);
        size_t m = levelVertices.size();

        for (size_t i = 0; i < m; ++i) {
            int uId = levelVertices[i];
            const auto& uNode = tree.getNodeById(uId);
            if (uNode.childrenIds.empty()) continue;

            for (size_t j = i + 1; j < m; ++j) {
                int vId = levelVertices[j];
                const auto& vNode = tree.getNodeById(vId);
                if (vNode.childrenIds.empty()) continue;

                // If u and v are adjacent:
                if (graph.isAdjacent(uId, vId)) {
                    for (int wId : uNode.childrenIds) {
                        for (int xId : vNode.childrenIds) {
                            res.casesChecked++;
                            if (!graph.isAdjacent(wId, xId)) {
                                res.passed = false;
                                std::ostringstream oss;
                                oss << "Child " << wId << " of " << uId << " and child " << xId 
                                    << " of " << vId << " (both at depth " << d 
                                    << " and adjacent) fail to be adjacent.";
                                res.failureDetails.push_back(oss.str());
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}

StructuralCheckResult StructuralAnalysis::verifyTheorem3_7(const IntervalGraph& graph, const DistanceTree& tree) {
    StructuralCheckResult res;
    res.ruleName = "Theorem 3.7: Each Depth Level Induces a Clique";
    res.thesisReference = "Chapter 3, Section 3.10, Theorem 3.7 (Page 30)";
    res.passed = true;
    res.casesChecked = 0;

    for (int d = 0; d <= tree.getMaxDepth(); ++d) {
        const auto& levelVertices = tree.getLevelVertices(d);
        size_t m = levelVertices.size();

        // Enumerate all pairs in L_d and verify adjacency directly by interval intersection
        for (size_t i = 0; i < m; ++i) {
            for (size_t j = i + 1; j < m; ++j) {
                res.casesChecked++;
                int uId = levelVertices[i];
                int vId = levelVertices[j];

                if (!graph.isAdjacent(uId, vId)) {
                    res.passed = false;
                    std::ostringstream oss;
                    oss << "Level clique violation at depth " << d << ": vertices " 
                        << uId << " and " << vId << " are at the same depth but not adjacent in G.";
                    res.failureDetails.push_back(oss.str());
                }
            }
        }
    }

    return res;
}

StructuralReport StructuralAnalysis::runFullAudit(const IntervalGraph& graph, const DistanceTree& tree) {
    StructuralReport report;
    report.lemma3_1 = verifyLemma3_1(graph, tree);
    report.depthLocality = verifyDepthLocality(graph, tree);
    report.lemma3_2 = verifyLemma3_2(graph, tree);
    report.lemma3_3 = verifyLemma3_3(graph, tree);
    report.lemma3_4 = verifyLemma3_4(graph, tree);
    report.lemma3_5 = verifyLemma3_5(graph, tree);
    report.lemma3_6 = verifyLemma3_6(graph, tree);
    report.theorem3_7 = verifyTheorem3_7(graph, tree);
    return report;
}

bool StructuralAnalysis::writeStructuralChecksReport(const StructuralReport& report, const std::string& filepath) {
    std::ofstream fout(filepath);
    if (!fout.is_open()) return false;

    fout << "========================================================================\n";
    fout << " Structural Analysis & Lemma Verification Report (Chapter 3)\n";
    fout << " Ground Truth Adjacency: Interval Intersections in G\n";
    fout << "========================================================================\n\n";

    auto printResult = [&](const StructuralCheckResult& r) {
        fout << "Rule:       " << r.ruleName << "\n";
        fout << "Reference:  " << r.thesisReference << "\n";
        fout << "Status:     " << (r.passed ? "PASS" : "FAIL") << "\n";
        fout << "Evaluated:  " << r.casesChecked << " cases/pairs\n";
        if (!r.passed) {
            fout << "Violations (" << r.failureDetails.size() << "):\n";
            for (const auto& msg : r.failureDetails) {
                fout << "  [-] " << msg << "\n";
            }
        }
        fout << "------------------------------------------------------------------------\n";
    };

    printResult(report.lemma3_1);
    printResult(report.depthLocality);
    printResult(report.lemma3_2);
    printResult(report.lemma3_3);
    printResult(report.lemma3_4);
    printResult(report.lemma3_5);
    printResult(report.lemma3_6);
    printResult(report.theorem3_7);

    fout << "\nGlobal Structural Verification: " 
         << (report.allPassed() ? "ALL PROPERTIES PASSED SUCCESSFULLY" : "FAILURES DETECTED") 
         << "\n";

    return true;
}

