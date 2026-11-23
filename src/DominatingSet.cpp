#include "DominatingSet.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include <algorithm>

DominatingSetResult DominatingSet::constructDominatingSet(const IntervalGraph& graph, const DistanceTree& tree) {
    DominatingSetResult res;
    res.h = tree.getNumLevels();
    res.n = graph.getNumVertices();
    res.rawFormulaBound = res.h / 2; // ceil((h - 1) / 2) == h / 2
    res.isBoundaryCaseH1 = (res.h == 1);
    res.theoreticalBound = res.isBoundaryCaseH1 ? 1 : res.rawFormulaBound;

    if (res.n == 0) {
        res.setSize = 0;
        res.satisfiesBound = true;
        res.constructionDetails = "Empty graph.";
        return res;
    }

    if (res.h == 1) {
        // Boundary case: Only root level (depth 0) exists
        res.selectedVertices.push_back(tree.getRootId());
        std::ostringstream oss;
        oss << "Boundary Case (h = 1): Distance tree contains only root vertex (Level 1 / Depth 0). "
            << "Thesis formula ceil((h - 1) / 2) = 0 is degenerate for non-empty graphs. "
            << "Selected root vertex (ID " << tree.getRootId() << ").";
        res.constructionDetails = oss.str();
    } else {
        std::ostringstream oss;
        oss << "Theorem 3.17 Construction (h = " << res.h << "): ";

        // Step 1: Select rightmost vertex of Level 2 (1-based Level 2 = 0-based Depth 1)
        const auto& level2 = tree.getLevelVerticesOneBased(2);
        int u2 = level2.back();
        res.selectedVertices.push_back(u2);
        oss << "Selected rightmost vertex of Level 2: ID " << u2
            << " (dominates Levels 1, 2, and 3). ";

        // Step 2: For remaining level pairs (4, 5), (6, 7), (8, 9), ...
        // Select rightmost vertex of the first level in each pair (Level i for i = 4, 6, 8, ... <= h)
        int pairCount = 0;
        for (int i = 4; i <= res.h; i += 2) {
            const auto& levelI = tree.getLevelVerticesOneBased(i);
            int ui = levelI.back();
            res.selectedVertices.push_back(ui);
            pairCount++;
            oss << "Selected rightmost vertex of Level " << i << ": ID " << ui;
            if (i + 1 <= res.h) {
                oss << " (dominates Levels " << i << " and " << (i + 1) << "). ";
            } else {
                oss << " (dominates Level " << i << "). ";
            }
        }

        if (pairCount == 0) {
            oss << "No remaining level pairs beyond Level 3.";
        }
        res.constructionDetails = oss.str();
    }

    res.setSize = static_cast<int>(res.selectedVertices.size());
    res.satisfiesBound = (res.setSize <= res.theoreticalBound);
    return res;
}

DominatingSetVerification DominatingSet::verifyDominatingSet(const IntervalGraph& graph, const std::vector<int>& dominatingSet) {
    DominatingSetVerification ver;
    ver.allVerticesValid = true;
    ver.allVerticesUnique = true;
    ver.isNonEmpty = true;
    ver.isDominating = true;

    int n = graph.getNumVertices();
    if (n == 0) {
        ver.messages.push_back("Graph is empty; trivial domination.");
        return ver;
    }

    if (dominatingSet.empty()) {
        ver.isNonEmpty = false;
        ver.isDominating = false;
        ver.messages.push_back("Error: Dominating set is empty for a non-empty graph.");
        for (const auto& v : graph.getSortedIntervals()) {
            ver.uncoveredVertices.push_back(v.id);
        }
        return ver;
    }

    std::unordered_set<int> dSet;
    for (int u : dominatingSet) {
        if (!graph.hasVertexId(u)) {
            ver.allVerticesValid = false;
            std::ostringstream oss;
            oss << "Error: Vertex ID " << u << " in dominating set does not exist in graph.";
            ver.messages.push_back(oss.str());
        }
        if (dSet.find(u) != dSet.end()) {
            ver.allVerticesUnique = false;
            std::ostringstream oss;
            oss << "Error: Duplicate vertex ID " << u << " found in dominating set.";
            ver.messages.push_back(oss.str());
        }
        dSet.insert(u);
    }

    // Check domination for every vertex v in V(G) using interval intersection directly
    for (const auto& v : graph.getSortedIntervals()) {
        bool covered = false;
        if (dSet.find(v.id) != dSet.end()) {
            covered = true;
        } else {
            for (int u : dominatingSet) {
                if (graph.isAdjacent(v.id, u)) {
                    covered = true;
                    break;
                }
            }
        }

        if (!covered) {
            ver.uncoveredVertices.push_back(v.id);
        }
    }

    if (!ver.uncoveredVertices.empty()) {
        ver.isDominating = false;
        std::ostringstream oss;
        oss << "Domination check failed: " << ver.uncoveredVertices.size()
            << " vertices are uncovered: ";
        for (size_t i = 0; i < ver.uncoveredVertices.size(); ++i) {
            int uid = ver.uncoveredVertices[i];
            const auto& iv = graph.getIntervalById(uid);
            oss << uid << " [" << iv.left << ", " << iv.right << "]"
                << (i + 1 < ver.uncoveredVertices.size() ? ", " : "");
        }
        ver.messages.push_back(oss.str());
    } else {
        ver.messages.push_back("All vertices in V(G) are dominated by D (either in D or adjacent to D).");
    }

    return ver;
}

namespace {
    bool checkCombinationDominating(const IntervalGraph& graph, const std::vector<int>& combo) {
        std::unordered_set<int> cSet(combo.begin(), combo.end());
        for (const auto& v : graph.getSortedIntervals()) {
            if (cSet.find(v.id) != cSet.end()) continue;
            bool covered = false;
            for (int u : combo) {
                if (graph.isAdjacent(v.id, u)) {
                    covered = true;
                    break;
                }
            }
            if (!covered) return false;
        }
        return true;
    }

    bool findMDSCombination(const IntervalGraph& graph, const std::vector<int>& allIds,
                            size_t k, size_t startIdx, std::vector<int>& current,
                            std::vector<int>& result) {
        if (current.size() == k) {
            if (checkCombinationDominating(graph, current)) {
                result = current;
                return true;
            }
            return false;
        }

        for (size_t i = startIdx; i < allIds.size(); ++i) {
            current.push_back(allIds[i]);
            if (findMDSCombination(graph, allIds, k, i + 1, current, result)) {
                return true;
            }
            current.pop_back();
        }
        return false;
    }
}

std::vector<int> DominatingSet::computeBruteForceMDS(const IntervalGraph& graph) {
    int n = graph.getNumVertices();
    if (n == 0) return {};
    if (n == 1) return { graph.getSortedIntervals()[0].id };

    std::vector<int> allIds;
    allIds.reserve(n);
    for (const auto& v : graph.getSortedIntervals()) {
        allIds.push_back(v.id);
    }

    // Test subset sizes k = 1, 2, ..., n
    for (size_t k = 1; k <= static_cast<size_t>(n); ++k) {
        std::vector<int> current;
        std::vector<int> result;
        if (findMDSCombination(graph, allIds, k, 0, current, result)) {
            return result;
        }
    }

    return allIds;
}

DominatingSetReport DominatingSet::runFullVerification(const IntervalGraph& graph, const DistanceTree& tree, bool runBruteForce) {
    DominatingSetReport rep;
    rep.numVertices = graph.getNumVertices();
    rep.h = tree.getNumLevels();
    rep.isBoundaryCaseH1 = (rep.h == 1);

    rep.result = constructDominatingSet(graph, tree);
    rep.constructedSize = rep.result.setSize;

    rep.verification = verifyDominatingSet(graph, rep.result.selectedVertices);
    rep.dominationPassed = rep.verification.isDominating;
    rep.boundSatisfied = rep.result.satisfiesBound;

    rep.hasBruteForceMDS = false;
    rep.bruteForceMDSSize = 0;

    if (runBruteForce && rep.numVertices <= 25) {
        std::vector<int> mds = computeBruteForceMDS(graph);
        rep.hasBruteForceMDS = true;
        rep.bruteForceMDSSize = static_cast<int>(mds.size());
    }

    std::ostringstream oss;
    oss << "Dominating Set: |D| = " << rep.constructedSize
        << ", Bound ceil((h-1)/2) = " << rep.result.theoreticalBound
        << (rep.isBoundaryCaseH1 ? " (h=1 boundary case)" : "")
        << ", Domination Valid = " << (rep.dominationPassed ? "YES" : "NO");
    if (rep.hasBruteForceMDS) {
        oss << ", Optimal gamma(G) = " << rep.bruteForceMDSSize;
    }
    rep.messages.push_back(oss.str());

    return rep;
}

bool DominatingSet::writeDominatingSetReport(const DominatingSetResult& res, const IntervalGraph& graph, const DistanceTree& tree, const std::string& filepath) {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << "================================================================================\n";
    out << " DOMINATING SET CONSTRUCTION REPORT (Theorem 3.17)\n";
    out << "================================================================================\n\n";

    out << "Graph Parameters:\n";
    out << "  Total Vertices (|V|)          : " << res.n << "\n";
    out << "  Distance Tree Levels (h)      : " << res.h << "\n";
    out << "  Distance Tree Max Depth       : " << (res.h - 1) << "\n";
    out << "  Formula Bound ceil((h - 1)/2) : " << res.rawFormulaBound << "\n";
    out << "  Applicable Theoretical Bound  : " << res.theoreticalBound;
    if (res.isBoundaryCaseH1) {
        out << " (Boundary Case: h = 1, raw formula gives 0, valid bound is 1)";
    }
    out << "\n\n";

    out << "Construction Summary:\n";
    out << "  " << res.constructionDetails << "\n";
    out << "  Number of Selected Vertices   : " << res.setSize << "\n";
    out << "  Bound Satisfied (|D| <= Bound): " << (res.satisfiesBound ? "YES" : "NO") << "\n\n";

    out << "Selected Vertices in Dominating Set D:\n";
    out << std::left << std::setw(12) << "Vertex_ID"
        << std::setw(10) << "Level"
        << std::setw(10) << "Depth"
        << std::setw(15) << "Left_Endpoint"
        << std::setw(15) << "Right_Endpoint"
        << "Role_in_Construction\n";
    out << "--------------------------------------------------------------------------------\n";

    for (size_t i = 0; i < res.selectedVertices.size(); ++i) {
        int uid = res.selectedVertices[i];
        const auto& node = tree.getNodeById(uid);
        const auto& iv = graph.getIntervalById(uid);

        std::string role;
        if (res.isBoundaryCaseH1) {
            role = "Root vertex (h = 1 boundary)";
        } else if (i == 0) {
            role = "Rightmost of Level 2 (dominates L1..L3)";
        } else {
            std::ostringstream rOss;
            int levelIdx = node.level;
            rOss << "Rightmost of Level " << levelIdx << " (dominates L" << levelIdx;
            if (levelIdx + 1 <= res.h) {
                rOss << "..L" << (levelIdx + 1);
            }
            rOss << ")";
            role = rOss.str();
        }

        out << std::left << std::setw(12) << uid
            << std::setw(10) << node.level
            << std::setw(10) << node.depth
            << std::setw(15) << iv.left
            << std::setw(15) << iv.right
            << role << "\n";
    }

    return true;
}

bool DominatingSet::writeVerificationReport(const DominatingSetReport& rep, const IntervalGraph& graph, const DistanceTree& tree, const std::string& filepath) {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << "================================================================================\n";
    out << " DOMINATING SET INDEPENDENT VERIFICATION REPORT\n";
    out << "================================================================================\n\n";

    out << "Overview:\n";
    out << "  Total Vertices (|V|)          : " << rep.numVertices << "\n";
    out << "  Distance Tree Height (h)      : " << rep.h << "\n";
    out << "  Constructed Dominating Set |D|: " << rep.constructedSize << "\n";
    out << "  Theoretical Bound             : " << rep.result.theoreticalBound << "\n";
    out << "  Bound Satisfied               : " << (rep.boundSatisfied ? "PASS" : "FAIL") << "\n";
    out << "  Independent Domination Check  : " << (rep.dominationPassed ? "PASS" : "FAIL") << "\n";
    if (rep.hasBruteForceMDS) {
        out << "  Brute-Force Optimal gamma(G)  : " << rep.bruteForceMDSSize << "\n";
        out << "  Approximation Ratio (|D|/gamma): "
            << std::fixed << std::setprecision(3)
            << (static_cast<double>(rep.constructedSize) / rep.bruteForceMDSSize) << "\n";
    }
    out << "\n";

    out << "Independent Validation Checks:\n";
    out << "  1. All Vertices in D Exist in Graph: " << (rep.verification.allVerticesValid ? "PASS" : "FAIL") << "\n";
    out << "  2. All Vertices in D are Unique    : " << (rep.verification.allVerticesUnique ? "PASS" : "FAIL") << "\n";
    out << "  3. Dominating Set is Non-Empty     : " << (rep.verification.isNonEmpty ? "PASS" : "FAIL") << "\n";
    out << "  4. Complete Graph Domination       : " << (rep.verification.isDominating ? "PASS" : "FAIL") << "\n\n";

    if (!rep.verification.uncoveredVertices.empty()) {
        out << "UNCOVERED VERTICES DETAILS:\n";
        for (int uid : rep.verification.uncoveredVertices) {
            const auto& iv = graph.getIntervalById(uid);
            const auto& node = tree.getNodeById(uid);
            out << "  Vertex " << uid << " [level " << node.level << ", depth " << node.depth
                << "]: interval [" << iv.left << ", " << iv.right << "]\n";
        }
        out << "\n";
    }

    out << "Vertex-by-Vertex Domination Witness Table:\n";
    out << std::left << std::setw(12) << "Vertex_ID"
        << std::setw(10) << "Level"
        << std::setw(12) << "In_D"
        << std::setw(15) << "Dominated_By"
        << "Interval\n";
    out << "--------------------------------------------------------------------------------\n";

    std::unordered_set<int> dSet(rep.result.selectedVertices.begin(), rep.result.selectedVertices.end());
    for (const auto& v : graph.getSortedIntervals()) {
        const auto& node = tree.getNodeById(v.id);
        bool inD = (dSet.find(v.id) != dSet.end());
        std::string witness;
        if (inD) {
            witness = "Self (in D)";
        } else {
            std::ostringstream wOss;
            bool first = true;
            for (int u : rep.result.selectedVertices) {
                if (graph.isAdjacent(v.id, u)) {
                    if (!first) wOss << ", ";
                    wOss << u;
                    first = false;
                }
            }
            witness = first ? "NONE (UNCOVERED)" : wOss.str();
        }

        std::ostringstream ivOss;
        ivOss << "[" << v.left << ", " << v.right << "]";

        out << std::left << std::setw(12) << v.id
            << std::setw(10) << node.level
            << std::setw(12) << (inD ? "YES" : "NO")
            << std::setw(15) << witness
            << ivOss.str() << "\n";
    }

    out << "\nFinal Verdict: " << (rep.allPassed() ? "ALL CHECKS PASSED (VERIFIED)" : "VERIFICATION FAILED") << "\n";
    return true;
}
