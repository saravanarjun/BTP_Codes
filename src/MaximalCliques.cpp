#include "MaximalCliques.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include <cassert>

bool MaximalCliques::verifyCliqueProperty(const IntervalGraph& graph, const std::vector<int>& clique) {
    size_t m = clique.size();
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = i + 1; j < m; ++j) {
            if (!graph.isAdjacent(clique[i], clique[j])) {
                return false;
            }
        }
    }
    return true;
}

bool MaximalCliques::verifyMaximality(const IntervalGraph& graph, const std::vector<int>& clique, std::vector<int>& extendableWith) {
    extendableWith.clear();
    std::unordered_set<int> cliqueSet(clique.begin(), clique.end());

    int n = graph.getNumVertices();
    const auto& sortedIvs = graph.getSortedIntervals();

    for (int i = 0; i < n; ++i) {
        int vId = sortedIvs[i].id;
        if (cliqueSet.find(vId) != cliqueSet.end()) {
            continue; // Already in clique
        }

        // Check if vId is adjacent to EVERY vertex in the clique
        bool canAdd = true;
        for (int cMember : clique) {
            if (!graph.isAdjacent(vId, cMember)) {
                canAdd = false;
                break;
            }
        }

        if (canAdd) {
            extendableWith.push_back(vId);
        }
    }

    return extendableWith.empty();
}

NodeClique MaximalCliques::constructCliqueLemma3_8(const IntervalGraph& graph, const DistanceTree& tree, int uId) {
    NodeClique nc;
    nc.originatingNodeId = uId;
    const auto& uNode = tree.getNodeById(uId);
    nc.isOriginatingNodeLeaf = uNode.childrenIds.empty();

    std::vector<int> rawMembers;

    // (i) the node u itself
    rawMembers.push_back(uId);

    // (ii) all children of u
    for (int cId : uNode.childrenIds) {
        rawMembers.push_back(cId);
    }

    // (iii) all nodes at same depth as u that occur to the right of u in distance-tree ordering
    int depth = uNode.depth;
    const auto& levelVertices = tree.getLevelVertices(depth);
    for (int wId : levelVertices) {
        if (tree.getNodeById(wId).posLevel > uNode.posLevel) {
            rawMembers.push_back(wId);
        }
    }

    // (iv) all children of the nodes at the same depth as u that occur to the left of u
    for (int wId : levelVertices) {
        const auto& wNode = tree.getNodeById(wId);
        if (wNode.posLevel < uNode.posLevel) {
            for (int cwId : wNode.childrenIds) {
                rawMembers.push_back(cwId);
            }
        }
    }

    // Deterministic ordering: sort by canonical interval sortedIndex, then remove duplicates
    std::sort(rawMembers.begin(), rawMembers.end(), [&](int a, int b) {
        return graph.getSortedIndexById(a) < graph.getSortedIndexById(b);
    });
    rawMembers.erase(std::unique(rawMembers.begin(), rawMembers.end()), rawMembers.end());
    nc.members = std::move(rawMembers);

    // Verify clique and maximality properties
    nc.isClique = verifyCliqueProperty(graph, nc.members);
    nc.isMaximal = verifyMaximality(graph, nc.members, nc.extendableWith);

    return nc;
}

std::vector<NodeClique> MaximalCliques::constructAllNodeCliques(const IntervalGraph& graph, const DistanceTree& tree) {
    int n = graph.getNumVertices();
    std::vector<NodeClique> result;
    result.reserve(n);

    // Process nodes in left-to-right interval order
    for (int i = 0; i < n; ++i) {
        int uId = graph.getIntervalBySortedIndex(i).id;
        result.push_back(constructCliqueLemma3_8(graph, tree, uId));
    }

    return result;
}

std::vector<std::vector<int>> MaximalCliques::extractUniqueMaximalCliques(const std::vector<NodeClique>& allCliques) {
    std::vector<std::vector<int>> uniqueMaximal;

    for (const auto& nc : allCliques) {
        if (!nc.isMaximal) continue;

        bool duplicate = false;
        for (const auto& existing : uniqueMaximal) {
            if (existing == nc.members) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            uniqueMaximal.push_back(nc.members);
        }
    }

    // Sort cliques deterministically by their first vertex sorted index
    std::sort(uniqueMaximal.begin(), uniqueMaximal.end());
    return uniqueMaximal;
}

std::vector<CompactClique> MaximalCliques::constructCompactCliques(const IntervalGraph& graph, const DistanceTree& tree) {
    std::vector<CompactClique> compacts;
    int n = graph.getNumVertices();

    // Boundary case: single-vertex graph
    if (n == 1) {
        int rootId = tree.getRootId();
        const auto& iv = graph.getIntervalById(rootId);
        CompactClique cc;
        cc.parentId = rootId;
        cc.rightmostChildId = rootId;
        cc.leftBound = iv.left;
        cc.rightBound = iv.left;
        cc.expandedMembers = {rootId};
        cc.isClique = true;
        cc.isMaximal = true;
        compacts.push_back(cc);
        return compacts;
    }

    // Lemma 3.15: Every maximal clique corresponds to a parent node p and its rightmost child c.
    // Interval [l_p, l_c] represents all vertices whose left endpoints lie in [l_p, l_c].
    for (int i = 0; i < n; ++i) {
        int pId = graph.getIntervalBySortedIndex(i).id;
        const auto& pNode = tree.getNodeById(pId);

        // Only nodes that have children define a compact representation
        if (pNode.childrenIds.empty()) {
            continue;
        }

        int cId = pNode.childrenIds.back(); // Rightmost child
        double l_p = graph.getIntervalById(pId).left;
        double l_c = graph.getIntervalById(cId).left;

        CompactClique cc;
        cc.parentId = pId;
        cc.rightmostChildId = cId;
        cc.leftBound = l_p;
        cc.rightBound = l_c;

        // Expand all vertices whose left endpoint lies in [l_p, l_c] inclusive
        for (int j = 0; j < n; ++j) {
            const auto& candidate = graph.getIntervalBySortedIndex(j);
            if (candidate.left >= l_p && candidate.left <= l_c) {
                cc.expandedMembers.push_back(candidate.id);
            }
        }

        // Verify expanded set
        std::vector<int> dummy;
        cc.isClique = verifyCliqueProperty(graph, cc.expandedMembers);
        cc.isMaximal = verifyMaximality(graph, cc.expandedMembers, dummy);

        compacts.push_back(cc);
    }

    return compacts;
}

void MaximalCliques::bronKerboschRecursive(std::vector<int>& R, std::vector<int>& P, std::vector<int>& X,
                                           const IntervalGraph& graph, std::vector<std::vector<int>>& result) {
    if (P.empty() && X.empty()) {
        std::vector<int> clique = R;
        std::sort(clique.begin(), clique.end(), [&](int a, int b) {
            return graph.getSortedIndexById(a) < graph.getSortedIndexById(b);
        });
        result.push_back(clique);
        return;
    }

    // Choose pivot u in P U X maximizing |P \cap N(u)|
    int pivot = -1;
    int maxIntersect = -1;

    std::vector<int> P_union_X = P;
    P_union_X.insert(P_union_X.end(), X.begin(), X.end());

    for (int u : P_union_X) {
        int count = 0;
        for (int v : P) {
            if (graph.isAdjacent(u, v)) count++;
        }
        if (count > maxIntersect) {
            maxIntersect = count;
            pivot = u;
        }
    }

    // Candidates in P \ N(pivot) (note: in simple graphs, pivot is not in N(pivot))
    std::vector<int> candidates;
    for (int v : P) {
        if (v == pivot || !graph.isAdjacent(pivot, v)) {
            candidates.push_back(v);
        }
    }

    for (int v : candidates) {
        R.push_back(v);

        std::vector<int> newP, newX;
        for (int p : P) {
            if (p != v && graph.isAdjacent(v, p)) newP.push_back(p);
        }
        for (int x : X) {
            if (x != v && graph.isAdjacent(v, x)) newX.push_back(x);
        }

        bronKerboschRecursive(R, newP, newX, graph, result);

        R.pop_back();

        // Move v from P to X
        P.erase(std::remove(P.begin(), P.end(), v), P.end());
        X.push_back(v);
    }
}

std::vector<std::vector<int>> MaximalCliques::enumerateBronKerbosch(const IntervalGraph& graph) {
    std::vector<std::vector<int>> result;
    int n = graph.getNumVertices();
    if (n == 0) return result;

    std::vector<int> R;
    std::vector<int> P;
    P.reserve(n);
    for (int i = 0; i < n; ++i) {
        P.push_back(graph.getIntervalBySortedIndex(i).id);
    }
    std::vector<int> X;

    bronKerboschRecursive(R, P, X, graph, result);

    std::sort(result.begin(), result.end());
    return result;
}

CliqueComparisonReport MaximalCliques::compareAllMethods(const IntervalGraph& graph, const DistanceTree& tree) {
    CliqueComparisonReport report;
    report.exactMatch = true;

    // 1. Bron-Kerbosch ground truth
    auto bkCliques = enumerateBronKerbosch(graph);
    report.numBronKerboschCliques = bkCliques.size();

    // 2. Lemma 3.8 all node cliques
    auto allNodeCliques = constructAllNodeCliques(graph, tree);
    report.numLemma3_8TotalConstructed = allNodeCliques.size();

    report.numLeafSubsetsDetected = 0;
    for (const auto& nc : allNodeCliques) {
        if (nc.isOriginatingNodeLeaf && !nc.isMaximal) {
            report.numLeafSubsetsDetected++;
        }
    }

    auto l38Maximal = extractUniqueMaximalCliques(allNodeCliques);
    report.numLemma3_8UniqueMaximal = l38Maximal.size();

    // 3. Lemma 3.15 compact cliques
    auto compactCliques = constructCompactCliques(graph, tree);
    report.numCompactCliques = compactCliques.size();

    std::vector<std::vector<int>> compactExpanded;
    compactExpanded.reserve(compactCliques.size());
    for (const auto& cc : compactCliques) {
        compactExpanded.push_back(cc.expandedMembers);
    }
    std::sort(compactExpanded.begin(), compactExpanded.end());

    // Compare collections
    if (bkCliques.size() != l38Maximal.size()) {
        report.exactMatch = false;
        std::ostringstream oss;
        oss << "Count mismatch: Bron-Kerbosch found " << bkCliques.size() 
            << " maximal cliques, but Lemma 3.8 unique maximal gave " << l38Maximal.size();
        report.discrepancies.push_back(oss.str());
    }

    if (bkCliques.size() != compactExpanded.size()) {
        report.exactMatch = false;
        std::ostringstream oss;
        oss << "Count mismatch: Bron-Kerbosch found " << bkCliques.size() 
            << " maximal cliques, but Lemma 3.15 compact gave " << compactExpanded.size();
        report.discrepancies.push_back(oss.str());
    }

    if (bkCliques != l38Maximal) {
        report.exactMatch = false;
        report.discrepancies.push_back("Content mismatch between Bron-Kerbosch and Lemma 3.8 maximal cliques.");
    }

    if (bkCliques != compactExpanded) {
        report.exactMatch = false;
        report.discrepancies.push_back("Content mismatch between Bron-Kerbosch and Lemma 3.15 compact expanded cliques.");
    }

    return report;
}

bool MaximalCliques::writeMaximalCliques(const std::vector<NodeClique>& allNodeCliques, 
                                        const std::vector<std::vector<int>>& uniqueMaximal, 
                                        const std::string& filepath) {
    std::ofstream fout(filepath);
    if (!fout.is_open()) return false;

    fout << "========================================================================\n";
    fout << " Maximal Cliques Report (Lemma 3.8)\n";
    fout << " Total Distance Tree Nodes Evaluated: " << allNodeCliques.size() << "\n";
    fout << " Total Unique Maximal Cliques:        " << uniqueMaximal.size() << "\n";
    fout << "========================================================================\n\n";

    fout << "--- Unique Maximal Cliques (Canonical Order) ---\n";
    for (size_t i = 0; i < uniqueMaximal.size(); ++i) {
        fout << "Clique " << (i + 1) << " (Size " << uniqueMaximal[i].size() << "): {";
        for (size_t k = 0; k < uniqueMaximal[i].size(); ++k) {
            if (k > 0) fout << ", ";
            fout << uniqueMaximal[i][k];
        }
        fout << "}\n";
    }
    fout << "\n";

    fout << "--- Evaluation of Lemma 3.8 for Every Node u in T(G) ---\n";
    fout << "Node_u\tIs_Leaf\tIs_Clique\tIs_Maximal\tSize\tMembers\tStatus / Non-Maximal Details\n";
    for (const auto& nc : allNodeCliques) {
        fout << nc.originatingNodeId << "\t"
             << (nc.isOriginatingNodeLeaf ? "YES" : "NO") << "\t"
             << (nc.isClique ? "PASS" : "FAIL") << "\t\t"
             << (nc.isMaximal ? "PASS" : "SUBSET") << "\t\t"
             << nc.members.size() << "\t{";
        for (size_t k = 0; k < nc.members.size(); ++k) {
            if (k > 0) fout << ",";
            fout << nc.members[k];
        }
        fout << "}\t";
        if (nc.isMaximal) {
            fout << "MAXIMAL CLIQUE\n";
        } else {
            fout << "Non-maximal: extendable with {";
            for (size_t k = 0; k < nc.extendableWith.size(); ++k) {
                if (k > 0) fout << ",";
                fout << nc.extendableWith[k];
            }
            fout << "}\n";
        }
    }

    return true;
}

bool MaximalCliques::writeCompactCliques(const std::vector<CompactClique>& compactCliques, const std::string& filepath) {
    std::ofstream fout(filepath);
    if (!fout.is_open()) return false;

    fout << "========================================================================\n";
    fout << " Compact Representation of Maximal Cliques (Lemma 3.15: [l_p, l_c])\n";
    fout << " Total Compact Descriptors: " << compactCliques.size() << "\n";
    fout << "========================================================================\n\n";

    fout << "Idx\tParent_p\tRightmost_c\tl_p\t\tl_c\t\tSize\tExpanded Members\n";
    for (size_t i = 0; i < compactCliques.size(); ++i) {
        const auto& cc = compactCliques[i];
        fout << (i + 1) << "\t"
             << cc.parentId << "\t\t"
             << cc.rightmostChildId << "\t\t"
             << std::fixed << std::setprecision(4) << cc.leftBound << "\t"
             << cc.rightBound << "\t"
             << cc.expandedMembers.size() << "\t{";
        for (size_t k = 0; k < cc.expandedMembers.size(); ++k) {
            if (k > 0) fout << ", ";
            fout << cc.expandedMembers[k];
        }
        fout << "}\n";
    }

    return true;
}

bool MaximalCliques::writeCliqueVerification(const CliqueComparisonReport& report, const std::string& filepath) {
    std::ofstream fout(filepath);
    if (!fout.is_open()) return false;

    fout << "========================================================================\n";
    fout << " Maximal Clique Cross-Verification Report\n";
    fout << " Comparing: Bron-Kerbosch (Ground Truth) vs Lemma 3.8 vs Lemma 3.15\n";
    fout << "========================================================================\n\n";

    fout << "Bron-Kerbosch Ground Truth Cliques:  " << report.numBronKerboschCliques << "\n";
    fout << "Lemma 3.8 Total Evaluated Nodes:     " << report.numLemma3_8TotalConstructed << "\n";
    fout << "Lemma 3.8 Leaf Subsets Detected:     " << report.numLeafSubsetsDetected << "\n";
    fout << "Lemma 3.8 Unique Maximal Cliques:    " << report.numLemma3_8UniqueMaximal << "\n";
    fout << "Lemma 3.15 Compact Cliques:          " << report.numCompactCliques << "\n";
    fout << "Exact Equivalence Between Methods:   " << (report.exactMatch ? "PASS (100% IDENTICAL)" : "FAIL") << "\n\n";

    if (!report.discrepancies.empty()) {
        fout << "Discrepancies Encountered:\n";
        for (const auto& disc : report.discrepancies) {
            fout << "  [-] " << disc << "\n";
        }
    } else {
        fout << "[+] All maximal cliques from Lemma 3.8, Lemma 3.15, and Bron-Kerbosch match identically!\n";
    }

    return true;
}
