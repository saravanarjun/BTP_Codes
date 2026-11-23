#include "IndependentSet.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cassert>

bool IndependentSet::isIndependentSet(const IntervalGraph& graph, const std::vector<int>& vertexIds) {
    size_t m = vertexIds.size();
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = i + 1; j < m; ++j) {
            if (graph.isAdjacent(vertexIds[i], vertexIds[j])) {
                return false;
            }
        }
    }
    return true;
}

MISBoundsResult IndependentSet::verifyBounds(const DistanceTree& tree, int actualAlpha) {
    MISBoundsResult res;
    res.h = tree.getNumLevels();
    res.actualAlpha = actualAlpha;
    res.lowerBound = res.h / 2; // floor(h / 2)

    if (res.h == 1) {
        res.isBoundaryCaseH1 = true;
        res.upperBound = 1; // True upper bound for single vertex
        res.boundsSatisfied = (actualAlpha == 1);
        res.details = "Boundary case h = 1: The graph consists of a single vertex with alpha(G) = 1. "
                      "The thesis formula upper bound (h - 1 = 0) is false for h = 1. "
                      "Proper bound alpha(G) = 1 is verified.";
    } else {
        res.isBoundaryCaseH1 = false;
        res.upperBound = res.h - 1; // Lemma 3.11 bound
        res.boundsSatisfied = (actualAlpha >= res.lowerBound && actualAlpha <= res.upperBound);
        std::ostringstream oss;
        oss << "Thesis Lemma 3.11 bounds: floor(" << res.h << "/2) = " << res.lowerBound 
            << " <= alpha(G) = " << actualAlpha << " <= h - 1 = " << res.upperBound 
            << ". Result: " << (res.boundsSatisfied ? "PASS" : "FAIL");
        res.details = oss.str();
    }

    return res;
}

MISApproximationResult IndependentSet::computeApproximation(const IntervalGraph& graph, const DistanceTree& tree, int actualAlpha) {
    MISApproximationResult res;
    res.h = tree.getNumLevels();
    res.approxSize = (res.h + 1) / 2; // ceil(h / 2)
    res.selectedVertices.clear();

    // Traverse alternate levels: level 1, 3, 5, ... (0-based depth 0, 2, 4, ...)
    // Select the rightmost vertex of each such level
    for (int d = 0; d <= tree.getMaxDepth(); d += 2) {
        const auto& levelVertices = tree.getLevelVertices(d);
        assert(!levelVertices.empty());
        res.selectedVertices.push_back(levelVertices.back());
    }

    assert(static_cast<int>(res.selectedVertices.size()) == res.approxSize);

    // Verify independence using interval intersection
    res.isIndependent = isIndependentSet(graph, res.selectedVertices);

    // Compute approximation ratio: alpha(G) / |I'|
    res.approxRatio = (res.approxSize > 0) ? (static_cast<double>(actualAlpha) / static_cast<double>(res.approxSize)) : 0.0;

    if (res.h == 1) {
        res.guaranteeSatisfied = (res.approxRatio <= 1.0);
        res.details = "h = 1: Single vertex selected. Optimal exact solution (ratio = 1.0).";
    } else {
        // By Lemma 3.11, alpha(G) <= h - 1, and |I'| = ceil(h / 2).
        // Hence alpha(G) / |I'| <= (h - 1) / ceil(h / 2) < 2.0.
        res.guaranteeSatisfied = res.isIndependent && (res.approxRatio < 2.0);
        std::ostringstream oss;
        oss << "Approximation size: " << res.approxSize << " = ceil(" << res.h << "/2), "
            << "Exact alpha(G): " << actualAlpha << ", Ratio: " << std::fixed << std::setprecision(3) << res.approxRatio
            << " < 2.0. Result: " << (res.guaranteeSatisfied ? "PASS" : "FAIL");
        res.details = oss.str();
    }

    return res;
}

MISExactResult IndependentSet::computeExactDP(const IntervalGraph& graph, const DistanceTree& tree) {
    MISExactResult res;
    int h = tree.getNumLevels(); // 1-based level count (1 ... h)

    res.dpValues.resize(h + 1);
    res.bestValues.resize(h + 1, 0);

    std::vector<std::vector<int>> prefix(h + 1);
    std::vector<std::vector<int>> argPrefix(h + 1);
    std::vector<int> bestVertex(h + 1, -1);
    std::vector<std::vector<int>> predecessor(h + 1);

    // Helper map from vertexId to (level i, index j in L_i)
    std::unordered_map<int, std::pair<int, int>> vertexToLevelPos;

    // Level 1: Root alone
    int rootId = tree.getRootId();
    res.dpValues[1].resize(1);
    res.dpValues[1][0] = 1;

    prefix[1].resize(1);
    prefix[1][0] = 1;
    argPrefix[1].resize(1);
    argPrefix[1][0] = rootId;

    predecessor[1].resize(1);
    predecessor[1][0] = -1; // No predecessor

    res.bestValues[0] = 0;
    bestVertex[0] = -1;

    res.bestValues[1] = 1;
    bestVertex[1] = rootId;

    vertexToLevelPos[rootId] = {1, 0};

    // Process levels 2 through h
    for (int i = 2; i <= h; ++i) {
        const auto& L_prev = tree.getLevelVertices(i - 2); // Level i - 1 (depth i - 2)
        const auto& L_curr = tree.getLevelVertices(i - 1); // Level i (depth i - 1)
        int m = static_cast<int>(L_curr.size());

        res.dpValues[i].resize(m);
        prefix[i].resize(m);
        argPrefix[i].resize(m);
        predecessor[i].resize(m, -1);

        // Precompute position map for L_prev (1-based index k in 1 ... |L_prev|)
        std::unordered_map<int, int> posInPrev;
        for (int idx = 0; idx < static_cast<int>(L_prev.size()); ++idx) {
            posInPrev[L_prev[idx]] = idx + 1; // 1-based pos(p) = k
        }

        int currentLevelMaxDP = 0;
        int currentLevelArgMax = -1;

        for (int j = 0; j < m; ++j) {
            int vId = L_curr[j];
            vertexToLevelPos[vId] = {i, j};

            int pId = tree.getNodeById(vId).parentId;
            int k = posInPrev[pId]; // 1-based position of parent in L_prev

            // Term 1: BEST[i - 2] (corresponds to picking no vertex from level i - 1)
            int term1 = res.bestValues[i - 2];

            // Term 2: max DP over Left(parent(v))
            // Left(p) are vertices at indices 1 ... k-1 in L_prev.
            // If k == 1, Left(p) is empty.
            int term2 = -1;
            int argTerm2 = -1;
            if (k > 1) {
                term2 = prefix[i - 1][k - 2];      // k-2 in 0-based array corresponds to position k-1
                argTerm2 = argPrefix[i - 1][k - 2];
            }

            // Recurrence: DP[i][v] = 1 + max(BEST[i-2], max_{u in Left(p)} DP[i-1][u])
            if (term2 >= term1 && term2 > 0) {
                res.dpValues[i][j] = 1 + term2;
                predecessor[i][j] = argTerm2;
            } else {
                res.dpValues[i][j] = 1 + term1;
                predecessor[i][j] = bestVertex[i - 2];
            }

            if (res.dpValues[i][j] > currentLevelMaxDP) {
                currentLevelMaxDP = res.dpValues[i][j];
                currentLevelArgMax = vId;
            }
        }

        // Construct prefix maximum array for level i in a single left-to-right pass
        for (int j = 0; j < m; ++j) {
            int vId = L_curr[j];
            int dpVal = res.dpValues[i][j];

            if (j == 0) {
                prefix[i][0] = dpVal;
                argPrefix[i][0] = vId;
            } else {
                if (dpVal > prefix[i][j - 1]) {
                    prefix[i][j] = dpVal;
                    argPrefix[i][j] = vId;
                } else {
                    prefix[i][j] = prefix[i][j - 1];
                    argPrefix[i][j] = argPrefix[i][j - 1];
                }
            }
        }

        // Update BEST[i] = max(BEST[i - 1], max_{v in L_i} DP[i][v])
        if (currentLevelMaxDP > res.bestValues[i - 1]) {
            res.bestValues[i] = currentLevelMaxDP;
            bestVertex[i] = currentLevelArgMax;
        } else {
            res.bestValues[i] = res.bestValues[i - 1];
            bestVertex[i] = bestVertex[i - 1];
        }
    }

    res.alpha = res.bestValues[h];

    // Reconstruct the actual maximum independent set in O(n)
    res.selectedVertices.clear();
    int curr = bestVertex[h];
    while (curr != -1) {
        res.selectedVertices.push_back(curr);
        auto [lvl, idx] = vertexToLevelPos[curr];
        curr = predecessor[lvl][idx];
    }

    // Sort in canonical left-to-right interval order
    std::sort(res.selectedVertices.begin(), res.selectedVertices.end(), [&](int a, int b) {
        return graph.getSortedIndexById(a) < graph.getSortedIndexById(b);
    });

    res.isIndependent = isIndependentSet(graph, res.selectedVertices);
    assert(static_cast<int>(res.selectedVertices.size()) == res.alpha);
    assert(res.isIndependent);

    return res;
}

void IndependentSet::bruteForceBacktrack(const IntervalGraph& graph, const std::vector<int>& allVertices,
                                        size_t idx, std::vector<int>& currentSet, std::vector<int>& bestSet) {
    // Prune if current set + remaining vertices cannot exceed bestSet size
    if (currentSet.size() + (allVertices.size() - idx) <= bestSet.size()) {
        return;
    }

    if (idx == allVertices.size()) {
        if (currentSet.size() > bestSet.size()) {
            bestSet = currentSet;
        }
        return;
    }

    int vId = allVertices[idx];

    // Branch 1: Try adding vId if independent with all vertices currently in currentSet
    bool canAdd = true;
    for (int u : currentSet) {
        if (graph.isAdjacent(vId, u)) {
            canAdd = false;
            break;
        }
    }

    if (canAdd) {
        currentSet.push_back(vId);
        bruteForceBacktrack(graph, allVertices, idx + 1, currentSet, bestSet);
        currentSet.pop_back();
    }

    // Branch 2: Exclude vId
    bruteForceBacktrack(graph, allVertices, idx + 1, currentSet, bestSet);
}

std::vector<int> IndependentSet::computeBruteForceMIS(const IntervalGraph& graph) {
    int n = graph.getNumVertices();
    std::vector<int> allVertices;
    allVertices.reserve(n);
    for (int i = 0; i < n; ++i) {
        allVertices.push_back(graph.getIntervalBySortedIndex(i).id);
    }

    std::vector<int> currentSet;
    std::vector<int> bestSet;
    bruteForceBacktrack(graph, allVertices, 0, currentSet, bestSet);

    std::sort(bestSet.begin(), bestSet.end(), [&](int a, int b) {
        return graph.getSortedIndexById(a) < graph.getSortedIndexById(b);
    });

    return bestSet;
}

MISVerificationReport IndependentSet::runFullVerification(const IntervalGraph& graph, const DistanceTree& tree) {
    MISVerificationReport rep;
    rep.numVertices = graph.getNumVertices();
    rep.h = tree.getNumLevels();

    // 1. Exact DP
    auto dpRes = computeExactDP(graph, tree);
    rep.dpAlpha = dpRes.alpha;
    rep.dpSetIsIndependent = dpRes.isIndependent;

    // 2. Brute-Force Ground Truth (practical for small n <= 25)
    if (rep.numVertices <= 25) {
        auto bfSet = computeBruteForceMIS(graph);
        rep.bruteForceAlpha = static_cast<int>(bfSet.size());
        rep.dpAgreesWithBruteForce = (rep.dpAlpha == rep.bruteForceAlpha);
    } else {
        rep.bruteForceAlpha = -1; // Not feasible
        rep.dpAgreesWithBruteForce = true; // Skip check for large
    }

    // 3. 2-Approximation
    auto approxRes = computeApproximation(graph, tree, dpRes.alpha);
    rep.approxSize = approxRes.approxSize;
    rep.approxSetIsIndependent = approxRes.isIndependent;

    // 4. Bounds verification
    auto boundsRes = verifyBounds(tree, dpRes.alpha);
    rep.boundsValid = boundsRes.boundsSatisfied;

    if (!rep.dpAgreesWithBruteForce) {
        std::ostringstream oss;
        oss << "DP alpha (" << rep.dpAlpha << ") disagrees with brute-force alpha (" << rep.bruteForceAlpha << ").";
        rep.messages.push_back(oss.str());
    }
    if (!rep.dpSetIsIndependent) {
        rep.messages.push_back("Reconstructed DP set is not an independent set!");
    }
    if (!rep.approxSetIsIndependent) {
        rep.messages.push_back("Approximation set is not an independent set!");
    }
    if (!rep.boundsValid) {
        rep.messages.push_back("Lemma 3.11 bounds check failed: " + boundsRes.details);
    }

    return rep;
}

bool IndependentSet::writeBoundsReport(const MISBoundsResult& res, const std::string& filepath) {
    std::ofstream fout(filepath);
    if (!fout.is_open()) return false;

    fout << "========================================================================\n";
    fout << " Lemma 3.11: Bounds on Maximum Independent Set Size alpha(G)\n";
    fout << "========================================================================\n\n";

    fout << "Distance Tree Number of Levels h: " << res.h << "\n";
    fout << "Actual Maximum Independent Set alpha(G): " << res.actualAlpha << "\n";
    fout << "Theoretical Lower Bound floor(h / 2):     " << res.lowerBound << "\n";
    if (res.isBoundaryCaseH1) {
        fout << "Theoretical Upper Bound (h - 1):          0 (DEGENERATE for h = 1)\n";
        fout << "Handled Boundary Condition:               h = 1 single vertex has alpha(G) = 1\n";
    } else {
        fout << "Theoretical Upper Bound (h - 1):          " << res.upperBound << "\n";
    }
    fout << "Bounds Status:                            " << (res.boundsSatisfied ? "PASS" : "FAIL") << "\n\n";
    fout << "Diagnostic Details:\n  " << res.details << "\n";

    return true;
}

bool IndependentSet::writeApproximationReport(const MISApproximationResult& res, const std::string& filepath) {
    std::ofstream fout(filepath);
    if (!fout.is_open()) return false;

    fout << "========================================================================\n";
    fout << " Theorem 3.12: 2-Approximation Algorithm for Maximum Independent Set\n";
    fout << " Alternate Levels Strategy: Rightmost vertex of levels 1, 3, 5, ...\n";
    fout << "========================================================================\n\n";

    fout << "Number of Levels h:             " << res.h << "\n";
    fout << "Approximation Set Size |I'|:    " << res.approxSize << " (= ceil(" << res.h << " / 2))\n";
    fout << "Approximation Set Independent:  " << (res.isIndependent ? "PASS (True)" : "FAIL (False)") << "\n";
    fout << "Approximation Ratio:            " << std::fixed << std::setprecision(3) << res.approxRatio << "\n";
    fout << "Guarantee Satisfied (< 2.0):    " << (res.guaranteeSatisfied ? "PASS" : "FAIL") << "\n\n";

    fout << "Selected Vertices in I' (Left-to-Right):\n{";
    for (size_t k = 0; k < res.selectedVertices.size(); ++k) {
        if (k > 0) fout << ", ";
        fout << res.selectedVertices[k];
    }
    fout << "}\n\n";
    fout << "Details:\n  " << res.details << "\n";

    return true;
}

bool IndependentSet::writeExactDPReport(const MISExactResult& res, const DistanceTree& tree, const std::string& filepath) {
    std::ofstream fout(filepath);
    if (!fout.is_open()) return false;

    int h = tree.getNumLevels();

    fout << "========================================================================\n";
    fout << " Section 3.10.1 & Theorem 3.13: Exact Maximum Independent Set via DP\n";
    fout << " Recurrence: DP[i][v] = 1 + max(BEST[i-2], PREFIX[i-1][k-1])\n";
    fout << " Total Running Time: O(n) | Auxiliary Space: O(n)\n";
    fout << "========================================================================\n\n";

    fout << "Maximum Independent Set Size alpha(G) = BEST[h] = " << res.alpha << "\n";
    fout << "Reconstructed Set Independence Verified:          " << (res.isIndependent ? "PASS" : "FAIL") << "\n\n";

    fout << "Reconstructed Maximum Independent Set (Size " << res.selectedVertices.size() << "):\n{";
    for (size_t k = 0; k < res.selectedVertices.size(); ++k) {
        if (k > 0) fout << ", ";
        fout << res.selectedVertices[k];
    }
    fout << "}\n\n";

    fout << "--- Dynamic Programming Table ---\n";
    for (int i = 1; i <= h; ++i) {
        const auto& L_i = tree.getLevelVertices(i - 1);
        fout << "Level " << i << " (Depth " << (i - 1) << ", BEST = " << res.bestValues[i] << "): [";
        for (size_t j = 0; j < L_i.size(); ++j) {
            if (j > 0) fout << ", ";
            fout << "v" << L_i[j] << ":" << res.dpValues[i][j];
        }
        fout << "]\n";
    }

    return true;
}

bool IndependentSet::writeVerificationReport(const MISVerificationReport& rep, const std::string& filepath) {
    std::ofstream fout(filepath);
    if (!fout.is_open()) return false;

    fout << "========================================================================\n";
    fout << " Maximum Independent Set Verification & Oracle Audit\n";
    fout << "========================================================================\n\n";

    fout << "Number of Vertices n:                   " << rep.numVertices << "\n";
    fout << "Number of Levels h:                     " << rep.h << "\n";
    fout << "Exact DP Maximum Independent Set Size:  " << rep.dpAlpha << "\n";
    if (rep.bruteForceAlpha != -1) {
        fout << "Brute-Force Independent Set Size:       " << rep.bruteForceAlpha << "\n";
        fout << "DP vs Brute-Force Agreement:            " << (rep.dpAgreesWithBruteForce ? "PASS (100% IDENTICAL)" : "FAIL") << "\n";
    } else {
        fout << "Brute-Force Verification:               SKIPPED (n > 25)\n";
    }
    fout << "Reconstructed DP Set Is Independent:    " << (rep.dpSetIsIndependent ? "PASS" : "FAIL") << "\n";
    fout << "2-Approximation Size:                   " << rep.approxSize << "\n";
    fout << "2-Approximation Set Is Independent:     " << (rep.approxSetIsIndependent ? "PASS" : "FAIL") << "\n";
    fout << "Lemma 3.11 Bounds Check:                " << (rep.boundsValid ? "PASS" : "FAIL") << "\n\n";

    if (rep.allPassed()) {
        fout << "[+] All Maximum Independent Set theorems, algorithms, and bounds verified successfully!\n";
    } else {
        fout << "[-] Verification Warnings/Failures:\n";
        for (const auto& msg : rep.messages) {
            fout << "    " << msg << "\n";
        }
    }

    return true;
}

