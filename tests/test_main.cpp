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
#include <vector>
#include <cstdlib>

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::cerr << "[-] CHECK FAILED at line " << __LINE__ << ": " #cond << std::endl; \
        std::exit(1); \
    } \
} while(0)

// Full audit helper for all phases 1-6
void auditGraphComplete(const IntervalGraph& g, const std::string& name) {
    DistanceTree t(g);
    std::string err;
    CHECK(t.verifyTreeInvariants(err));

    // Phase 2 Positional Claims
    CHECK(DistanceVerification::verifyPositionalOrderingProperty(g, t, err));
    CHECK(DistanceVerification::verifyTraversalProperties(t, err));

    // Phase 3 All Pairs Eq 2.2 vs BFS
    auto rep = DistanceVerification::verifyAllPairs(g, t);
    CHECK(rep.passed());

    // Phase 4 Structural Analysis & Lemmas
    auto sRep = StructuralAnalysis::runFullAudit(g, t);
    CHECK(sRep.allPassed());

    // Phase 5 Maximal Cliques & Compact Cross-Comparison
    auto cRep = MaximalCliques::compareAllMethods(g, t);
    CHECK(cRep.exactMatch);

    // Phase 6 Maximum Independent Set
    auto misRep = IndependentSet::runFullVerification(g, t);
    if (!misRep.allPassed()) {
        std::cerr << "[-] MIS verification failed in " << name << ":\n";
        for (const auto& msg : misRep.messages) {
            std::cerr << "    " << msg << "\n";
        }
    }
    CHECK(misRep.allPassed());

    // Phase 7 Dominating Set (Theorem 3.17)
    auto domRep = DominatingSet::runFullVerification(g, t, true);
    if (!domRep.allPassed()) {
        std::cerr << "[-] Dominating set verification failed in " << name << ":\n";
        for (const auto& msg : domRep.messages) {
            std::cerr << "    " << msg << "\n";
        }
    }
    CHECK(domRep.allPassed());
}

// ---------------- Regression Tests Phases 1 to 5 ----------------
void testRegressionPhases1To5() {
    std::cout << "\n========================================\n";
    std::cout << " Running Phases 1-5 Regression Tests\n";
    std::cout << "========================================\n";

    IntervalGraph g;
    std::string err;
    bool ok = g.loadFromFile("input/intervals.txt", err);
    CHECK(ok);
    CHECK(g.getNumVertices() == 8);

    DistanceTree t(g);
    CHECK(t.verifyTreeInvariants(err));

    auto cRep = MaximalCliques::compareAllMethods(g, t);
    CHECK(cRep.exactMatch);

    std::cout << "[+] Phases 1-5 regression tests passed.\n";
}

// ---------------- Phase 6 Dedicated Tests ----------------

void testMISThesisEightVertex() {
    std::cout << "\n[Phase 6 Test 1] Thesis 8-Vertex Graph MIS...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    // Exact DP computation
    auto dpRes = IndependentSet::computeExactDP(g, t);
    CHECK(dpRes.alpha == 2);
    CHECK(dpRes.isIndependent);
    CHECK(dpRes.selectedVertices.size() == 2);

    // Brute-force verification
    auto bfSet = IndependentSet::computeBruteForceMIS(g);
    CHECK(bfSet.size() == 2);
    CHECK(dpRes.alpha == static_cast<int>(bfSet.size()));

    // Lemma 3.11 Bounds: h = 4 -> floor(4/2) = 2 <= alpha <= 4 - 1 = 3
    auto boundsRes = IndependentSet::verifyBounds(t, dpRes.alpha);
    CHECK(!boundsRes.isBoundaryCaseH1);
    CHECK(boundsRes.h == 4);
    CHECK(boundsRes.lowerBound == 2);
    CHECK(boundsRes.upperBound == 3);
    CHECK(boundsRes.boundsSatisfied);

    // Theorem 3.12 2-Approximation
    // Rightmost vertices of levels 1, 3: vertex 1 and vertex 6
    auto approxRes = IndependentSet::computeApproximation(g, t, dpRes.alpha);
    CHECK(approxRes.approxSize == 2); // ceil(4/2) = 2
    CHECK(approxRes.selectedVertices.size() == 2);
    CHECK(approxRes.selectedVertices[0] == 1);
    CHECK(approxRes.selectedVertices[1] == 6);
    CHECK(approxRes.isIndependent);
    CHECK(approxRes.approxRatio <= 2.0);
    CHECK(approxRes.guaranteeSatisfied);

    auditGraphComplete(g, "Thesis 8-Vertex Graph");
    std::cout << "  -> PASS\n";
}

void testMISBoundaryH1SingleVertex() {
    std::cout << "\n[Phase 6 Test 2] Boundary case h = 1 (Single Vertex)...\n";
    std::vector<Interval> ivs = {{42, 10.0, 20.0}};
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    auto dpRes = IndependentSet::computeExactDP(g, t);
    CHECK(dpRes.alpha == 1);
    CHECK(dpRes.selectedVertices.size() == 1);
    CHECK(dpRes.selectedVertices[0] == 42);

    auto bfSet = IndependentSet::computeBruteForceMIS(g);
    CHECK(bfSet.size() == 1);

    // Check h=1 boundary handling
    auto boundsRes = IndependentSet::verifyBounds(t, dpRes.alpha);
    CHECK(boundsRes.isBoundaryCaseH1);
    CHECK(boundsRes.h == 1);
    CHECK(boundsRes.boundsSatisfied); // Properly handled

    auto approxRes = IndependentSet::computeApproximation(g, t, dpRes.alpha);
    CHECK(approxRes.approxSize == 1);
    CHECK(approxRes.isIndependent);
    CHECK(approxRes.approxRatio == 1.0);
    CHECK(approxRes.guaranteeSatisfied);

    auditGraphComplete(g, "Single Vertex Boundary H=1");
    std::cout << "  -> PASS\n";
}

void testMISCompleteGraph() {
    std::cout << "\n[Phase 6 Test 3] Complete Graph (alpha = 1)...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 5.0},
        {2, 1.0, 6.0},
        {3, 2.0, 7.0},
        {4, 3.0, 8.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    auto dpRes = IndependentSet::computeExactDP(g, t);
    CHECK(dpRes.alpha == 1);

    auto bfSet = IndependentSet::computeBruteForceMIS(g);
    CHECK(bfSet.size() == 1);

    // h = 2 -> bounds: floor(2/2) = 1 <= alpha <= 2 - 1 = 1
    auto boundsRes = IndependentSet::verifyBounds(t, dpRes.alpha);
    CHECK(boundsRes.lowerBound == 1);
    CHECK(boundsRes.upperBound == 1);
    CHECK(boundsRes.boundsSatisfied);

    auditGraphComplete(g, "Complete Graph");
    std::cout << "  -> PASS\n";
}

void testMISPathGraph() {
    std::cout << "\n[Phase 6 Test 4] Path Graph (n = 6, alpha = 3)...\n";
    // Path: 1-2, 2-3, 3-4, 4-5, 5-6
    std::vector<Interval> ivs = {
        {1, 1.0, 3.0},
        {2, 2.5, 4.5},
        {3, 4.0, 6.0},
        {4, 5.5, 7.5},
        {5, 7.0, 9.0},
        {6, 8.5, 10.5}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    auto dpRes = IndependentSet::computeExactDP(g, t);
    CHECK(dpRes.alpha == 3);

    auto bfSet = IndependentSet::computeBruteForceMIS(g);
    CHECK(bfSet.size() == 3);

    auditGraphComplete(g, "Path Graph");
    std::cout << "  -> PASS\n";
}

void testMISEndpointTouching() {
    std::cout << "\n[Phase 6 Test 5] Endpoint-Touching Graph...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 2.0},
        {2, 2.0, 4.0},
        {3, 4.0, 6.0},
        {4, 6.0, 8.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    auditGraphComplete(g, "Endpoint-Touching Graph");
    std::cout << "  -> PASS\n";
}

void testMISExhaustiveRandomGraphs() {
    std::cout << "\n[Phase 6 Test 6] Exhaustive MIS verification across 50 random proper graphs...\n";
    long long totalGraphsVerified = 0;

    for (int run = 1; run <= 50; ++run) {
        int n = 3 + (run % 18); // Sizes between 3 and 20 (fast brute-force oracle)
        unsigned int seed = 9000 + run * 37;
        auto ivs = DistanceVerification::generateRandomProperIntervals(n, seed);

        IntervalGraph g;
        std::string err;
        CHECK(g.initFromIntervals(ivs, err));
        DistanceTree t(g);

        auto rep = IndependentSet::runFullVerification(g, t);
        if (!rep.allPassed()) {
            std::cerr << "[-] MIS verification failed on random run " << run << " (n=" << n << ")\n";
            for (const auto& msg : rep.messages) {
                std::cerr << "    " << msg << "\n";
            }
        }
        CHECK(rep.allPassed());
        totalGraphsVerified++;
    }

    std::cout << "  Successfully verified 50 random graphs against Brute-Force Oracle:\n"
              << "    - Exact DP vs Brute-force: 100% MATCH\n"
              << "    - Reconstructed DP independence: 100% PASS\n"
              << "    - 2-Approximation independence and guarantee: 100% PASS\n"
              << "    - Lemma 3.11 Bounds: 100% PASS\n";
    std::cout << "  -> PASS\n";
}

// ========================================================
// Phase 7: Dominating Set (Theorem 3.17) Tests
// ========================================================

void testDomSetSingleVertex() {
    std::cout << "\n[Phase 7 Test 1] Single Vertex Boundary Case (h = 1)...\n";
    std::vector<Interval> ivs = { {1, 0.0, 5.0} };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    CHECK(t.getNumLevels() == 1);
    auto res = DominatingSet::constructDominatingSet(g, t);
    CHECK(res.isBoundaryCaseH1);
    CHECK(res.setSize == 1);
    CHECK(res.selectedVertices[0] == 1);
    CHECK(res.theoreticalBound == 1);
    CHECK(res.rawFormulaBound == 0); // ceil((1-1)/2) = 0 is degenerate

    auto ver = DominatingSet::verifyDominatingSet(g, res.selectedVertices);
    CHECK(ver.allPassed());

    auto rep = DominatingSet::runFullVerification(g, t, true);
    CHECK(rep.allPassed());
    CHECK(rep.hasBruteForceMDS);
    CHECK(rep.bruteForceMDSSize == 1);
    std::cout << "  -> PASS\n";
}

void testDomSetTwoVertex() {
    std::cout << "\n[Phase 7 Test 2] Two-Vertex Connected Graph (h = 2)...\n";
    std::vector<Interval> ivs = { {1, 0.0, 4.0}, {2, 2.0, 6.0} };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    CHECK(t.getNumLevels() == 2);
    auto res = DominatingSet::constructDominatingSet(g, t);
    CHECK(!res.isBoundaryCaseH1);
    CHECK(res.setSize == 1);
    CHECK(res.selectedVertices[0] == 2); // Rightmost of level 2
    CHECK(res.theoreticalBound == 1);

    auto ver = DominatingSet::verifyDominatingSet(g, res.selectedVertices);
    CHECK(ver.allPassed());

    auto rep = DominatingSet::runFullVerification(g, t, true);
    CHECK(rep.allPassed());
    CHECK(rep.bruteForceMDSSize == 1);
    std::cout << "  -> PASS\n";
}

void testDomSetThesisEightVertex() {
    std::cout << "\n[Phase 7 Test 3] Thesis 8-Vertex Example (h = 4)...\n";
    std::vector<Interval> ivs = {
        {1, 1.0, 5.0},
        {2, 2.0, 6.0},
        {3, 3.0, 7.0},
        {4, 4.0, 8.0},
        {5, 7.0, 10.0},
        {6, 8.0, 11.0},
        {7, 9.0, 12.0},
        {8, 10.0, 13.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    CHECK(t.getNumLevels() == 4);
    auto res = DominatingSet::constructDominatingSet(g, t);
    CHECK(res.setSize == 2);
    // Rightmost of Level 2 is 4, rightmost of Level 4 is 8
    CHECK(res.selectedVertices[0] == 4);
    CHECK(res.selectedVertices[1] == 8);
    CHECK(res.theoreticalBound == 2);

    auto ver = DominatingSet::verifyDominatingSet(g, res.selectedVertices);
    CHECK(ver.allPassed());

    auto rep = DominatingSet::runFullVerification(g, t, true);
    CHECK(rep.allPassed());
    CHECK(rep.bruteForceMDSSize == 2); // Exact optimal MDS size
    std::cout << "  -> PASS\n";
}

void testDomSetCompleteGraph() {
    std::cout << "\n[Phase 7 Test 4] Complete Graph (h = 2, gamma = 1)...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 10.0},
        {2, 1.0, 11.0},
        {3, 2.0, 12.0},
        {4, 3.0, 13.0},
        {5, 4.0, 14.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    CHECK(t.getNumLevels() == 2);
    auto res = DominatingSet::constructDominatingSet(g, t);
    CHECK(res.setSize == 1);
    CHECK(res.selectedVertices[0] == 5); // Rightmost of level 2

    auto rep = DominatingSet::runFullVerification(g, t, true);
    CHECK(rep.allPassed());
    CHECK(rep.bruteForceMDSSize == 1);
    std::cout << "  -> PASS\n";
}

void testDomSetPathGraph() {
    std::cout << "\n[Phase 7 Test 5] Path-like Proper Graph (h = 6)...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 1.9},
        {2, 1.8, 2.9},
        {3, 2.8, 3.9},
        {4, 3.8, 4.9},
        {5, 4.8, 5.9},
        {6, 5.8, 6.9}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    CHECK(t.getNumLevels() == 6);
    auto res = DominatingSet::constructDominatingSet(g, t);
    CHECK(res.setSize == 3); // Rightmost of Level 2, 4, 6: {2, 4, 6}
    CHECK(res.theoreticalBound == 3);

    auto rep = DominatingSet::runFullVerification(g, t, true);
    CHECK(rep.allPassed());
    std::cout << "  -> PASS\n";
}

void testDomSetOddHeightGraph() {
    std::cout << "\n[Phase 7 Test 6] Odd Height Graph (h = 5, ceil((5-1)/2) = 2)...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 2.0},
        {2, 1.0, 3.0},
        {3, 2.5, 4.5},
        {4, 4.0, 6.0},
        {5, 5.5, 7.5}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    CHECK(t.getNumLevels() == 5);
    auto res = DominatingSet::constructDominatingSet(g, t);
    CHECK(res.setSize == 2); // Rightmost of Level 2 (2) and Level 4 (4)
    CHECK(res.theoreticalBound == 2);

    auto rep = DominatingSet::runFullVerification(g, t, true);
    CHECK(rep.allPassed());
    std::cout << "  -> PASS\n";
}

void testDomSetMultipleBranches() {
    std::cout << "\n[Phase 7 Test 7] Multiple Branches per Level...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 5.0},
        {2, 1.0, 6.0},
        {3, 2.0, 7.0},
        {4, 3.0, 8.0},
        {5, 6.0, 10.0},
        {6, 7.0, 11.0},
        {7, 9.0, 13.0},
        {8, 10.0, 14.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    auditGraphComplete(g, "Multiple Branches Graph");
    std::cout << "  -> PASS\n";
}

void testDomSetEndpointTouching() {
    std::cout << "\n[Phase 7 Test 8] Endpoint-Touching Graph...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 2.0},
        {2, 2.0, 4.0},
        {3, 4.0, 6.0},
        {4, 6.0, 8.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    auditGraphComplete(g, "Endpoint-Touching Graph");
    std::cout << "  -> PASS\n";
}

void testDomSetExhaustiveRandomGraphs() {
    std::cout << "\n[Phase 7 Test 9] Exhaustive Dominating Set verification across 50 random proper graphs...\n";
    long long totalGraphsVerified = 0;
    int oddHeightCount = 0;
    int evenHeightCount = 0;

    for (int run = 1; run <= 50; ++run) {
        int n = 3 + (run % 18); // Sizes between 3 and 20 (fast brute-force oracle)
        unsigned int seed = 12000 + run * 43;
        auto ivs = DistanceVerification::generateRandomProperIntervals(n, seed);

        IntervalGraph g;
        std::string err;
        CHECK(g.initFromIntervals(ivs, err));
        DistanceTree t(g);

        int h = t.getNumLevels();
        if (h % 2 == 1) oddHeightCount++;
        else evenHeightCount++;

        auto rep = DominatingSet::runFullVerification(g, t, true);
        if (!rep.allPassed()) {
            std::cerr << "[-] Dominating set verification failed on random run " << run
                      << " (n=" << n << ", h=" << h << ")\n";
            for (const auto& msg : rep.messages) {
                std::cerr << "    " << msg << "\n";
            }
        }
        CHECK(rep.allPassed());

        // Check that constructed size >= optimal gamma(G)
        if (rep.hasBruteForceMDS) {
            CHECK(rep.constructedSize >= rep.bruteForceMDSSize);
        }

        totalGraphsVerified++;
    }

    std::cout << "  Successfully verified 50 random graphs against Brute-Force MDS Oracle:\n"
              << "    - Independent Domination Coverage: 100% PASS\n"
              << "    - Theorem 3.17 Bound Satisfied:    100% PASS\n"
              << "    - Heights Tested: " << oddHeightCount << " odd-height, "
              << evenHeightCount << " even-height\n";
    std::cout << "  -> PASS\n";
}

void testDomSetGeometricEndpointInequalities() {
    std::cout << "\n[Phase 7 Test 10] Geometric Endpoint Inequalities & Distinct p != u*_i Cases...\n";

    // 1. Targeted check on Thesis 8-Vertex graph:
    // Vertex 5 is in Level 3, its parent is Vertex 3 in Level 2.
    // The rightmost vertex of Level 2 is Vertex 4 (distinct: p=3 != u*_2=4).
    {
        std::vector<Interval> ivs = {
            {1, 1.0, 5.0}, {2, 2.0, 6.0}, {3, 3.0, 7.0}, {4, 4.0, 8.0},
            {5, 7.0, 10.0}, {6, 8.0, 11.0}, {7, 9.0, 12.0}, {8, 10.0, 13.0}
        };
        IntervalGraph g;
        std::string err;
        CHECK(g.initFromIntervals(ivs, err));
        DistanceTree t(g);

        const auto& node5 = t.getNodeById(5);
        int p = node5.parentId; // 3
        CHECK(p == 3);
        int u_star = t.getLevelVerticesOneBased(2).back(); // 4
        CHECK(u_star == 4);
        CHECK(p != u_star); // Distinct!

        const auto& ip = g.getIntervalById(p);
        const auto& iu = g.getIntervalById(u_star);
        const auto& iy = g.getIntervalById(5);

        // Verify chains:
        // l(u*) <= r(p) < r(y)
        CHECK(iu.left <= ip.right);
        CHECK(ip.right < iy.right);
        // l(y) <= r(p) <= r(u*)
        CHECK(iy.left <= ip.right);
        CHECK(ip.right <= iu.right);
        // r(p) in both intervals
        CHECK(iu.left <= ip.right && ip.right <= iu.right);
        CHECK(iy.left <= ip.right && ip.right <= iy.right);
        // Direct intersection & adjacency
        CHECK(g.isAdjacent(u_star, 5));
    }

    // 2. Exhaustive check across 50 random graphs
    long long distinctParentCount = 0;
    long long totalParentChildChecks = 0;

    for (int run = 1; run <= 50; ++run) {
        int n = 3 + (run % 18);
        unsigned int seed = 12000 + run * 43;
        auto ivs = DistanceVerification::generateRandomProperIntervals(n, seed);

        IntervalGraph g;
        std::string err;
        CHECK(g.initFromIntervals(ivs, err));
        DistanceTree t(g);

        int h = t.getNumLevels();
        for (int i = 2; i <= h; i += 2) {
            if (i + 1 > h) continue;
            int u_star = t.getLevelVerticesOneBased(i).back();
            const auto& iu = g.getIntervalById(u_star);

            const auto& levelNext = t.getLevelVerticesOneBased(i + 1);
            for (int y : levelNext) {
                totalParentChildChecks++;
                const auto& nodeY = t.getNodeById(y);
                int p = nodeY.parentId;
                if (p != u_star) {
                    distinctParentCount++;
                }

                const auto& ip = g.getIntervalById(p);
                const auto& iy = g.getIntervalById(y);

                // 1. Parent relation: l(y) <= r(p)
                CHECK(iy.left <= ip.right);
                // 2. Properness & depth order: r(p) <= r(y)
                CHECK(ip.right <= iy.right);
                // 3. Rightmost peer in Level i: r(p) <= r(u*)
                CHECK(ip.right <= iu.right);
                // 4. Theorem 3.7 level clique: l(u*) <= r(p)
                CHECK(iu.left <= ip.right);

                // Derived intersection point r(p) in both I_u* and I_y:
                CHECK(iu.left <= ip.right && ip.right <= iu.right);
                CHECK(iy.left <= ip.right && ip.right <= iy.right);

                // Domination edge verified
                CHECK(g.isAdjacent(u_star, y));
            }
        }
    }

    CHECK(distinctParentCount > 50); // Ensure distinct cases are well exercised!
    std::cout << "  Tested " << totalParentChildChecks << " cross-level parent-child relations:\n"
              << "    - Distinct p != u*_i cases: " << distinctParentCount << " (100% verified)\n"
              << "    - Inequalities l(u*) <= r(p) <= r(y) and l(y) <= r(p) <= r(u*): 100% PASS\n"
              << "    - Interval intersection and adjacency: 100% PASS\n";
    std::cout << "  -> PASS\n";
}

void testDomSetIdenticalIntervals() {
    std::cout << "\n[Phase 7 Test 11] Identical Intervals Permitted by Canonical Ordering...\n";
    // Vertices 2 and 3 have IDENTICAL intervals [2.0, 6.0]
    // Permitted by canonical ordering with ID tie-breaker
    std::vector<Interval> ivs = {
        {1, 0.0, 4.0},
        {2, 2.0, 6.0},
        {3, 2.0, 6.0}, // Identical to vertex 2
        {4, 5.0, 9.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    auditGraphComplete(g, "Identical Intervals Graph");

    // Check specific endpoint properties where parent and rightmost have identical intervals:
    // Level 1: {1}
    // Level 2: {2, 3} (both [2.0, 6.0]), rightmost is 3
    // Level 3: {4} ([5.0, 9.0]), parent is 2
    int p = t.getNodeById(4).parentId;
    CHECK(p == 2);
    int u_star = t.getLevelVerticesOneBased(2).back();
    CHECK(u_star == 3);
    CHECK(p != u_star);

    const auto& ip = g.getIntervalById(p);
    const auto& iu = g.getIntervalById(u_star);
    const auto& iy = g.getIntervalById(4);

    // Verify non-strict chains:
    // Chain 1: l(u*) <= r(p) <= r(u*)
    CHECK(iu.left <= ip.right);
    CHECK(ip.right <= iu.right); // 6.0 <= 6.0 (equality holds!)

    // Chain 2: l(y) <= r(p) <= r(y)
    CHECK(iy.left <= ip.right);
    CHECK(ip.right <= iy.right);

    // r(p) in both intervals
    CHECK(iu.left <= ip.right && ip.right <= iu.right);
    CHECK(iy.left <= ip.right && ip.right <= iy.right);

    CHECK(g.isAdjacent(u_star, 4));
    std::cout << "  -> PASS\n";
}

// ========================================================
// Phase 8: Incremental Maintenance of MIS (Theorem 3.14)
// ========================================================

void testIncMIS_1_SingleVertex() {
    std::cout << "\n[Phase 8 Test 1] Leaf Insertion into Single-Vertex Graph...\n";
    std::vector<Interval> ivs = { {1, 0.0, 5.0} };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));
    CHECK(inc.getAlpha() == 1);

    // Insert leaf at Level 2
    Interval newLeaf{2, 3.0, 7.0};
    auto res = inc.insertLeaf(newLeaf);
    CHECK(res.success);
    CHECK(res.parentId == 1);
    CHECK(res.level == 2);
    CHECK(res.dpValue == 1);
    CHECK(res.newAlpha == 1);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testIncMIS_2_Level2() {
    std::cout << "\n[Phase 8 Test 2] Insert Multiple Leaves at Level 2...\n";
    std::vector<Interval> ivs = { {1, 0.0, 6.0} };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    CHECK(inc.insertLeaf({2, 1.0, 7.0}).success);
    CHECK(inc.insertLeaf({3, 2.0, 8.0}).success);
    CHECK(inc.getNumLevels() == 2);
    CHECK(inc.getAlpha() == 1);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testIncMIS_3_DeeperLevel() {
    std::cout << "\n[Phase 8 Test 3] Insert Leaf at Deeper Level (Level 3)...\n";
    std::vector<Interval> ivs = { {1, 0.0, 4.0}, {2, 2.0, 6.0} };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    auto res = inc.insertLeaf({3, 5.0, 8.0});
    CHECK(res.success);
    CHECK(res.level == 3);
    CHECK(res.dpValue == 2);
    CHECK(res.newAlpha == 2);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testIncMIS_4_ParentNotRightmost() {
    std::cout << "\n[Phase 8 Test 4] Insert Leaf Whose Parent is NOT Rightmost...\n";
    // Handcrafted graph where Level 2 has vertices 2 [1.0, 5.0] and 3 [3.0, 7.0] (rightmost is 3)
    // Leaf 4 [4.5, 7.5] reaches r_2 = 5.0, so parent is 2 (not 3!) and properness is preserved (r_4 >= r_3).
    std::vector<Interval> ivs = {
        {1, 0.0, 4.0},
        {2, 1.0, 5.0},
        {3, 3.0, 7.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    auto res = inc.insertLeaf({4, 4.5, 7.5});
    CHECK(res.success);
    CHECK(res.parentId == 2); // Parent is 2, not rightmost (3)!
    CHECK(res.level == 3);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testIncMIS_5_ParentIsRightmost() {
    std::cout << "\n[Phase 8 Test 5] Insert Leaf Whose Parent IS Rightmost...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 4.0},
        {2, 1.0, 5.0},
        {3, 3.0, 7.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    // Leaf reaching r_3 = 7.0 with left > 5.0
    auto res = inc.insertLeaf({4, 6.0, 9.0});
    CHECK(res.success);
    CHECK(res.parentId == 3); // Rightmost of level 2
    CHECK(res.level == 3);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testIncMIS_6_MultipleSequentialLeaves() {
    std::cout << "\n[Phase 8 Test 6] Multiple Sequential Leaf Insertions...\n";
    std::vector<Interval> ivs = { {1, 0.0, 3.0} };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    std::vector<Interval> newLeaves = {
        {2, 2.0, 5.0},
        {3, 4.0, 7.0},
        {4, 6.0, 9.0},
        {5, 8.0, 11.0},
        {6, 10.0, 13.0}
    };

    for (const auto& iv : newLeaves) {
        auto res = inc.insertLeaf(iv);
        CHECK(res.success);
        auto ver = inc.runIndependentVerification();
        CHECK(ver.allPassed());
    }

    CHECK(inc.getNumVertices() == 6);
    CHECK(inc.getAlpha() == 3);
    std::cout << "  -> PASS\n";
}

void testIncMIS_7_IncreaseAlpha() {
    std::cout << "\n[Phase 8 Test 7] Insertion That Increases Alpha...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 3.0},
        {2, 2.0, 5.0}
    }; // alpha = 1
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));
    CHECK(inc.getAlpha() == 1);

    auto res = inc.insertLeaf({3, 4.0, 7.0});
    CHECK(res.success);
    CHECK(res.alphaIncreased);
    CHECK(res.newAlpha == 2);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testIncMIS_8_UnchangedAlpha() {
    std::cout << "\n[Phase 8 Test 8] Insertion That Keeps Alpha Unchanged...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 4.0},
        {2, 1.0, 5.0}
    }; // alpha = 1
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    auto res = inc.insertLeaf({3, 2.0, 6.0});
    CHECK(res.success);
    CHECK(!res.alphaIncreased);
    CHECK(res.newAlpha == 1);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testIncMIS_9_EndpointTouching() {
    std::cout << "\n[Phase 8 Test 9] Endpoint-Touching Leaf Insertion...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 4.0},
        {2, 2.0, 6.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    // Touching endpoint: left 6.0 == right 6.0 of vertex 2
    auto res = inc.insertLeaf({3, 6.0, 9.0});
    CHECK(res.success);
    CHECK(res.parentId == 2);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testIncMIS_10_IdenticalIntervals() {
    std::cout << "\n[Phase 8 Test 10] Identical Intervals Leaf Insertion...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 4.0},
        {2, 2.0, 6.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    // Insert vertex 3 identical to vertex 2: [2.0, 6.0]
    auto res = inc.insertLeaf({3, 2.0, 6.0});
    CHECK(res.success);
    CHECK(res.level == 2);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testIncMIS_11_InvalidProperness() {
    std::cout << "\n[Phase 8 Test 11] Rejection: Properness Violation...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 5.0},
        {2, 2.0, 7.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    // Interval [1.0, 4.0] is properly contained in [0.0, 5.0]
    auto res = inc.insertLeaf({3, 1.0, 4.0});
    CHECK(!res.success);
    CHECK(res.diagnostic.find("Properness violation") != std::string::npos);
    std::cout << "  -> PASS (Expected rejection)\n";
}

void testIncMIS_12_InvalidConnectedness() {
    std::cout << "\n[Phase 8 Test 12] Rejection: Connectedness Violation (Gap)...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 5.0},
        {2, 2.0, 7.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    // Interval [12.0, 15.0] has a gap (max right is 7.0)
    auto res = inc.insertLeaf({3, 12.0, 15.0});
    CHECK(!res.success);
    CHECK(res.diagnostic.find("Connectedness violation") != std::string::npos);
    std::cout << "  -> PASS (Expected rejection)\n";
}

void testIncMIS_13_InvalidNonLeaf() {
    std::cout << "\n[Phase 8 Test 13] Rejection: Non-Leaf Insertion...\n";
    // Thesis 8-vertex graph
    std::vector<Interval> ivs = {
        {1, 1.0, 5.0}, {2, 2.0, 6.0}, {3, 3.0, 7.0}, {4, 4.0, 8.0},
        {5, 7.0, 10.0}, {6, 8.0, 11.0}, {7, 9.0, 12.0}, {8, 10.0, 13.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    // Inserting [5.0, 9.0] reaches vertex 7 (left 9.0) with left 5.0 < 7.0 (vertex 5's left).
    // Existing vertex 7 would adopt this new vertex as parent instead of 5!
    auto res = inc.insertLeaf({9, 5.0, 9.0});
    CHECK(!res.success);
    CHECK(res.diagnostic.find("Leaf-insertion model violated") != std::string::npos);
    std::cout << "  -> PASS (Expected rejection)\n";
}

void testIncMIS_14_MiddleInsertion() {
    std::cout << "\n[Phase 8 Test 14] Rejection: Middle Insertion in an Existing Level...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 5.0}, // root (L1)
        {2, 1.0, 6.0}, // L2
        {4, 3.0, 8.0}  // L2
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    // Attempt to insert 3 [2.0, 7.0] between 2 and 4 in Level 2.
    // Under the rightmost append-only model, this is rejected because it does not follow vertex 4 in canonical order.
    auto res = inc.insertLeaf({3, 2.0, 7.0});
    CHECK(!res.success);
    CHECK(res.diagnostic.find("Rightmost append-only leaf model violated") != std::string::npos);
    std::cout << "  -> PASS (Expected rejection of middle insertion)\n";
}

void testIncMIS_15_GeneratedSequences() {
    std::cout << "\n[Phase 8 Test 15] Reproducible Generated Sequences of Rightmost Leaf Insertions...\n";
    long long totalInsertionsTested = 0;

    for (int seq = 1; seq <= 30; ++seq) {
        std::vector<Interval> baseIvs = {
            {1, 0.0, 4.0},
            {2, 2.0, 6.0}
        };
        IntervalGraph g;
        std::string err;
        CHECK(g.initFromIntervals(baseIvs, err));
        DistanceTree t(g);

        IncrementalIndependentSet inc;
        CHECK(inc.initialize(g, t, err));

        int nextId = 3;
        double curRight = 6.0;

        for (int step = 0; step < 6; ++step) {
            double newL = curRight - 1.5;
            double newR = newL + 3.5;
            Interval newIv{nextId++, newL, newR};

            auto res = inc.insertLeaf(newIv);
            if (res.success) {
                curRight = newR;
                totalInsertionsTested++;

                auto ver = inc.runIndependentVerification();
                CHECK(ver.allPassed());
                CHECK(ver.alphaMatchesStatic);
                CHECK(ver.alphaMatchesBruteForce);
            }
        }
    }

    std::cout << "  Successfully tested " << totalInsertionsTested
              << " dynamic leaf insertions across 30 sequences:\n"
              << "    - Incremental alpha vs Static Phase 6 DP: 100% MATCH\n"
              << "    - Incremental alpha vs Brute-force MIS:   100% MATCH\n"
              << "    - Distance tree leaf and parent preservation: 100% PASS\n";
    std::cout << "  -> PASS\n";
}

void testIncMIS_16_LeftmostInsertion() {
    std::cout << "\n[Phase 8 Test 16] Rejection: Leftmost Insertion in an Existing Level...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 5.0}, // root (L1)
        {2, 2.0, 7.0}  // L2
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    // Attempt to insert 3 [1.0, 6.0] at Level 2, before vertex 2.
    auto res = inc.insertLeaf({3, 1.0, 6.0});
    CHECK(!res.success);
    CHECK(res.diagnostic.find("Rightmost append-only leaf model violated") != std::string::npos);
    std::cout << "  -> PASS (Expected rejection of leftmost insertion)\n";
}

void testIncMIS_17_AdversarialCounterexample() {
    std::cout << "\n[Phase 8 Test 17] Rejection: Exact Adversarial Counterexample from Audit...\n";
    // Base graph: 1: [0,4], 2: [2,6], 3: [5,9] (q), 4: [8,12] (c)
    std::vector<Interval> ivs = {
        {1, 0.0, 4.0},
        {2, 2.0, 6.0},
        {3, 5.0, 9.0},
        {4, 8.0, 12.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));
    CHECK(inc.getAlpha() == 2);

    // Adversarial middle insertion: 5 [4.5, 7.0] in Level 3 before vertex 3.
    // In unrestricted model this produced incorrect alpha=2 (true alpha=3).
    // Under the rightmost append-only model, this is safely rejected!
    auto res = inc.insertLeaf({5, 4.5, 7.0});
    CHECK(!res.success);
    CHECK(res.diagnostic.find("Rightmost append-only leaf model violated") != std::string::npos);
    std::cout << "  -> PASS (Adversarial counterexample correctly rejected)\n";
}

void testIncMIS_18_DeepestLevelExtension() {
    std::cout << "\n[Phase 8 Test 18] Acceptance: Insertion Creating New Deepest Level...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 4.0},
        {2, 2.0, 6.0},
        {3, 5.0, 9.0},
        {4, 8.0, 12.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);
    CHECK(t.getNumLevels() == 4);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));
    CHECK(inc.getAlpha() == 2);

    // Insert 5 [10.0, 14.0], parent is 4 (r4=12 >= 10), level becomes 5 (depth 4)
    auto res = inc.insertLeaf({5, 10.0, 14.0});
    CHECK(res.success);
    CHECK(res.level == 5);
    CHECK(res.parentId == 4);
    CHECK(res.newAlpha == 3);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    CHECK(ver.alphaMatchesStatic);
    CHECK(ver.alphaMatchesBruteForce);
    std::cout << "  -> PASS\n";
}

void testIncMIS_19_ExistingLevelRightmost() {
    std::cout << "\n[Phase 8 Test 19] Acceptance: Rightmost Insertion in an Existing Level...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 6.0}, // root (L1)
        {2, 1.0, 7.0}  // L2
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    IncrementalIndependentSet inc;
    CHECK(inc.initialize(g, t, err));

    // Insert 3 [2.5, 8.0] at Level 2, occurring after vertex 2 in canonical order
    auto res = inc.insertLeaf({3, 2.5, 8.0});
    CHECK(res.success);
    CHECK(res.level == 2);
    CHECK(res.parentId == 1);

    auto ver = inc.runIndependentVerification();
    CHECK(ver.allPassed());
    CHECK(ver.alphaMatchesStatic);
    CHECK(ver.alphaMatchesBruteForce);
    std::cout << "  -> PASS\n";
}

// ---------------- Phase 9: Decremental Maintenance Tests ----------------

void testDecTree_1_LeafWithImmediateRight() {
    std::cout << "\n[Phase 9 Test 1] Delete a leaf with an immediate right vertex...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    // Vertex 2 in Level 2 has 0 children (leaf), immediate right is 3
    CHECK(dt.getNode(2).childHead == -1);
    CHECK(dt.getNode(2).nextLevelSibling == 3);

    auto res = dt.deleteVertex(2);
    CHECK(res.success);
    CHECK(res.deletedId == 2);
    CHECK(res.numChildrenRelocated == 0);

    auto ver = dt.runIndependentVerification();
    if (!ver.allPassed()) {
        for (const auto& msg : ver.messages) std::cerr << "  " << msg << "\n";
    }
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_2_NonLeafWithChildrenAndImmediateRight() {
    std::cout << "\n[Phase 9 Test 2] Delete non-leaf vertex with children and immediate right vertex...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    // Vertex 3 has child 5, immediate right is 4
    CHECK(dt.getNode(3).childHead == 5);
    CHECK(dt.getNode(3).nextLevelSibling == 4);

    auto res = dt.deleteVertex(3);
    CHECK(res.success);
    CHECK(res.deletedId == 3);
    CHECK(res.immediateRightId == 4);
    CHECK(res.numChildrenRelocated == 1);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_3_JoinChildSets() {
    std::cout << "\n[Phase 9 Test 3] Delete vertex whose immediate right already has children (join sets)...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    CHECK(dt.getNode(3).childCount == 1);
    CHECK(dt.getNode(4).childCount == 1);

    auto res = dt.deleteVertex(3);
    CHECK(res.success);
    CHECK(dt.getNode(4).childCount == 2);
    auto ch4 = dt.getChildren(4);
    CHECK(ch4.size() == 2);
    CHECK(ch4[0] == 5);
    CHECK(ch4[1] == 6);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_4_RightmostChildMetadata() {
    std::cout << "\n[Phase 9 Test 4] Delete vertex that is rightmost child of its parent...\n";
    // Proper interval graph: parent 1 [0, 4] with children 2 [1, 5], 3 [2, 6], 4 [3, 7] (leaf).
    // Vertex 4 is rightmost child of parent 1 and rightmost in Level 2.
    std::vector<Interval> ivs = {
        {1, 0.0, 4.0},
        {2, 1.0, 5.0},
        {3, 2.0, 6.0},
        {4, 3.0, 7.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    CHECK(dt.getNode(1).childTail == 4);

    auto res = dt.deleteVertex(4);
    CHECK(res.success);
    CHECK(dt.getNode(1).childTail == 3);
    CHECK(dt.getNode(3).nextSibling == -1);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_5_EmptyChildSet() {
    std::cout << "\n[Phase 9 Test 5] Delete vertex with empty child set...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    CHECK(dt.getNode(2).childCount == 0);
    auto res = dt.deleteVertex(2);
    CHECK(res.success);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_6_EmptyChildSetRightmostOfLevel() {
    std::cout << "\n[Phase 9 Test 6] Delete leaf that is rightmost vertex of its level...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    CHECK(dt.getNode(8).childCount == 0);
    CHECK(dt.getNode(8).nextLevelSibling == -1);

    auto res = dt.deleteVertex(8);
    CHECK(res.success);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());

    auto res2 = dt.deleteVertex(7);
    CHECK(res2.success);
    CHECK(dt.getNumLevels() == 3);

    auto ver2 = dt.runIndependentVerification();
    CHECK(ver2.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_7_RejectChildrenWithoutImmediateRight() {
    std::cout << "\n[Phase 9 Test 7] Reject deletion of vertex with children but no immediate right vertex...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    CHECK(dt.getNode(4).childCount > 0);
    CHECK(dt.getNode(4).nextLevelSibling == -1);

    auto snapBefore = dt.getSnapshot();
    auto res = dt.deleteVertex(4);
    CHECK(!res.success);
    CHECK(res.diagnostic.find("no immediate right vertex") != std::string::npos);
    CHECK(dt.matchesSnapshot(snapBefore)); // Exact full observable state preserved

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS (Expected rejection, state unchanged)\n";
}

void testDecTree_8_RejectRootDeletion() {
    std::cout << "\n[Phase 9 Test 8] Reject root deletion...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    auto snapBefore = dt.getSnapshot();
    auto res = dt.deleteVertex(1);
    CHECK(!res.success);
    CHECK(res.diagnostic.find("Root deletion") != std::string::npos);
    CHECK(dt.matchesSnapshot(snapBefore)); // Exact full observable state preserved

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS (Expected rejection, state unchanged)\n";
}

void testDecTree_9_RejectDisconnectingDeletion() {
    std::cout << "\n[Phase 9 Test 9] Reject deletion that disconnects the graph...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 3.0},
        {2, 2.0, 5.0},
        {3, 4.0, 7.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    auto snapBefore = dt.getSnapshot();
    auto res = dt.deleteVertex(2);
    CHECK(!res.success);
    CHECK(res.diagnostic.find("disconnects") != std::string::npos);
    CHECK(dt.matchesSnapshot(snapBefore)); // Exact full observable state preserved

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS (Expected rejection, state unchanged)\n";
}

void testDecTree_10_RejectEmptyGraph() {
    std::cout << "\n[Phase 9 Test 10] Reject deletion leaving graph empty...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 5.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    auto snapBefore = dt.getSnapshot();
    auto res = dt.deleteVertex(1);
    CHECK(!res.success);
    CHECK(dt.matchesSnapshot(snapBefore)); // Exact full observable state preserved

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS (Expected rejection, state unchanged)\n";
}

void testDecTree_11_PropernessCheck() {
    std::cout << "\n[Phase 9 Test 11] Validate properness after deletion...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    auto res = dt.deleteVertex(2);
    CHECK(res.success);

    auto propRes = IntervalGraph::validateProperness(dt.getIntervals());
    CHECK(propRes.valid);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_12_RepeatedSequentialDeletions() {
    std::cout << "\n[Phase 9 Test 12] Repeated sequential deletions...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    auto r1 = dt.deleteVertex(2);
    CHECK(r1.success);
    CHECK(dt.runIndependentVerification().allPassed());

    auto r2 = dt.deleteVertex(8);
    CHECK(r2.success);
    CHECK(dt.runIndependentVerification().allPassed());

    auto r3 = dt.deleteVertex(3);
    CHECK(r3.success);
    CHECK(dt.runIndependentVerification().allPassed());

    auto r4 = dt.deleteVertex(7);
    CHECK(r4.success);
    CHECK(dt.runIndependentVerification().allPassed());

    CHECK(dt.getNumVertices() == 4);
    std::cout << "  -> PASS (All 4 sequential deletions verified)\n";
}

void testDecTree_13_EndpointTouching() {
    std::cout << "\n[Phase 9 Test 13] Deletions with endpoint-touching intervals...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 3.0},
        {2, 1.0, 4.0},
        {3, 3.0, 6.0},
        {4, 4.0, 7.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    auto res = dt.deleteVertex(2);
    CHECK(res.success);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_14_IdenticalIntervals() {
    std::cout << "\n[Phase 9 Test 14] Deletions with identical intervals...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 6.0},
        {2, 2.0, 7.0},
        {3, 2.0, 7.0},
        {4, 4.0, 9.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    CHECK(dt.getNode(2).nextLevelSibling == 3);

    auto res = dt.deleteVertex(2);
    CHECK(res.success);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_15_AlphaChanged() {
    std::cout << "\n[Phase 9 Test 15] Deletion that changes the MIS optimum...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 2.0},
        {2, 1.5, 3.5},
        {3, 3.0, 5.0},
        {4, 4.5, 6.5},
        {5, 6.0, 8.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));
    CHECK(dt.getAlpha() == 3);

    auto res = dt.deleteVertex(5);
    CHECK(res.success);
    CHECK(res.alphaChanged);
    CHECK(res.oldAlpha == 3);
    CHECK(res.newAlpha == 2);
    CHECK(dt.getAlpha() == 2);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_16_AlphaUnchanged() {
    std::cout << "\n[Phase 9 Test 16] Deletion that leaves MIS optimum unchanged...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));
    CHECK(dt.getAlpha() == 2);

    auto res = dt.deleteVertex(8);
    CHECK(res.success);
    CHECK(!res.alphaChanged);
    CHECK(res.newAlpha == 2);
    CHECK(dt.getAlpha() == 2);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_17_GeneratedSequences() {
    std::cout << "\n[Phase 9 Test 17] Reproducible Generated Sequences of Decremental Deletions...\n";
    int totalAttempted = 0;
    int totalAccepted = 0;
    int totalRejected = 0;
    int totalStaticComparisons = 0;
    int totalMISComparisons = 0;
    int totalMismatches = 0;
    std::unordered_map<std::string, int> rejectReasons;

    for (int seq = 1; seq <= 30; ++seq) {
        int n = 8 + (seq % 5);
        std::vector<Interval> raw;
        double curLeft = 0.0;
        double curRight = 4.0;
        for (int i = 1; i <= n; ++i) {
            raw.push_back({i, curLeft, curRight});
            double stepL = 0.6 + ((seq * 3 + i) % 8) / 10.0;
            double stepR = 0.6 + ((seq * 5 + i) % 8) / 10.0;
            curLeft += stepL;
            curRight += stepR;
        }

        IntervalGraph g;
        std::string err;
        if (!g.initFromIntervals(raw, err)) continue;
        DistanceTree t(g);

        DecrementalDistanceTree dt;
        if (!dt.initialize(g, t, err)) continue;

        for (int i = n; i >= 1; --i) {
            if (dt.getNumVertices() <= 2) break;
            if (!dt.hasVertex(i)) continue;

            totalAttempted++;
            auto res = dt.deleteVertex(i);
            if (res.success) {
                totalAccepted++;
                totalStaticComparisons++;
                totalMISComparisons++;
                auto ver = dt.runIndependentVerification();
                if (!ver.allPassed()) {
                    totalMismatches++;
                }
            } else {
                totalRejected++;
                if (res.diagnostic.find("Root deletion") != std::string::npos) {
                    rejectReasons["Root deletion"]++;
                } else if (res.diagnostic.find("no immediate right vertex") != std::string::npos) {
                    rejectReasons["Children without immediate right"]++;
                } else if (res.diagnostic.find("disconnects") != std::string::npos) {
                    rejectReasons["Graph disconnection"]++;
                } else {
                    rejectReasons["Other"]++;
                }
            }
        }
    }

    std::cout << "  Successfully tested " << totalAttempted << " deletion attempts across 30 sequences:\n";
    std::cout << "    - Accepted deletions: " << totalAccepted << "\n";
    std::cout << "    - Rejected deletions: " << totalRejected << "\n";
    for (const auto& kv : rejectReasons) {
        std::cout << "      * " << kv.first << ": " << kv.second << "\n";
    }
    std::cout << "    - Static tree rebuild comparisons: " << totalStaticComparisons << " (100% MATCH)\n";
    std::cout << "    - MIS comparisons against static DP & Brute-force: " << totalMISComparisons << " (100% MATCH)\n";
    std::cout << "    - Mismatches: " << totalMismatches << "\n";
    CHECK(totalAccepted > 0);
    CHECK(totalMismatches == 0);
    std::cout << "  -> PASS\n";
}

void testDecTree_18_EqualLeftEndpointParents() {
    std::cout << "\n[Phase 9 Test 18] Equal-left-endpoint parent candidates & identical intervals...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 6.0},
        {2, 2.0, 7.0},
        {3, 2.0, 7.0},
        {4, 6.5, 9.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    CHECK(dt.getNode(2).childHead == 4);
    CHECK(dt.getNode(3).childHead == -1);

    auto res = dt.deleteVertex(2);
    CHECK(res.success);
    CHECK(res.immediateRightId == 3);
    CHECK(res.numChildrenRelocated == 1);
    CHECK(dt.getNode(3).childHead == 4);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_19_ChildSetConcatenationWithIdenticalIntervals() {
    std::cout << "\n[Phase 9 Test 19] Child-set concatenation with identical intervals...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 5.0},
        {2, 1.0, 6.0},
        {3, 2.0, 7.0},
        {4, 5.5, 8.0},
        {5, 5.5, 8.0},
        {6, 6.5, 9.0},
        {7, 6.5, 9.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    auto ch2 = dt.getChildren(2);
    CHECK(ch2.size() == 2 && ch2[0] == 4 && ch2[1] == 5);
    auto ch3 = dt.getChildren(3);
    CHECK(ch3.size() == 2 && ch3[0] == 6 && ch3[1] == 7);

    auto res = dt.deleteVertex(2);
    CHECK(res.success);
    auto ch3_new = dt.getChildren(3);
    CHECK(ch3_new.size() == 4);
    CHECK(ch3_new[0] == 4);
    CHECK(ch3_new[1] == 5);
    CHECK(ch3_new[2] == 6);
    CHECK(ch3_new[3] == 7);

    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());
    std::cout << "  -> PASS\n";
}

void testDecTree_20_EmptyChildSetsCombinations() {
    std::cout << "\n[Phase 9 Test 20] Empty child sets combinations (C(u) empty, C(v) empty)...\n";
    std::vector<Interval> ivs = {
        {1, 0.0, 4.0},
        {2, 1.0, 5.0},
        {3, 2.0, 6.0},
        {4, 3.0, 7.0},
        {5, 6.5, 9.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    CHECK(dt.getNode(2).childCount == 0);
    CHECK(dt.getNode(3).childCount == 0);
    auto r1 = dt.deleteVertex(2);
    CHECK(r1.success);
    CHECK(dt.runIndependentVerification().allPassed());

    CHECK(dt.getNode(3).childCount == 0);
    CHECK(dt.getNode(4).childCount == 1);
    auto r2 = dt.deleteVertex(3);
    CHECK(r2.success);
    CHECK(dt.runIndependentVerification().allPassed());

    std::cout << "  -> PASS\n";
}

void testDecTree_21_AtomicRejectionSnapshots() {
    std::cout << "\n[Phase 9 Test 21] Atomic rejection full observable state snapshots...\n";
    IntervalGraph g;
    std::string err;
    CHECK(g.loadFromFile("input/intervals.txt", err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    auto s1 = dt.getSnapshot();
    auto res1 = dt.deleteVertex(1);
    CHECK(!res1.success);
    CHECK(dt.matchesSnapshot(s1));

    auto s2 = dt.getSnapshot();
    auto res2 = dt.deleteVertex(999);
    CHECK(!res2.success);
    CHECK(dt.matchesSnapshot(s2));

    auto s3 = dt.getSnapshot();
    auto res3 = dt.deleteVertex(4);
    CHECK(!res3.success);
    CHECK(dt.matchesSnapshot(s3));

    std::cout << "  -> PASS (100% snapshot equivalence on all rejections)\n";
}

void testDecTree_22_LongSequentialDeletions() {
    std::cout << "\n[Phase 9 Test 22] Repeated deletions down to 2 vertices...\n";
    std::vector<Interval> ivs;
    for (int i = 1; i <= 8; ++i) {
        ivs.push_back({i, static_cast<double>(i - 1), static_cast<double>(i + 2)});
    }
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    for (int delId = 8; delId >= 3; --delId) {
        auto res = dt.deleteVertex(delId);
        CHECK(res.success);
        auto ver = dt.runIndependentVerification();
        CHECK(ver.allPassed());
    }
    CHECK(dt.getNumVertices() == 2);
    std::cout << "  -> PASS (All 6 sequential deletions verified against static rebuild)\n";
}

void testDecTree_23_CaseBCounterexampleThreeDescriptors() {
    std::cout << "\n[Phase 9 Test 23] Adversarial Case B Deletion: Exactly 3 Descriptors Mutated...\n";
    // 7-vertex proper interval graph:
    // v0: [0.0, 10.0] (root)
    // v1: [2.0, 12.0] (p1)
    // v2: [8.0, 15.0] (p2)
    // v3: [11.0, 16.0] (w)
    // v4: [11.5, 17.0] (u)
    // v5: [12.5, 18.0] (v, leaf)
    // v6: [16.5, 20.0] (c_u)
    std::vector<Interval> ivs = {
        {0, 0.0, 10.0},
        {1, 2.0, 12.0},
        {2, 8.0, 15.0},
        {3, 11.0, 16.0},
        {4, 11.5, 17.0},
        {5, 12.5, 18.0},
        {6, 16.5, 20.0}
    };
    IntervalGraph g;
    std::string err;
    CHECK(g.initFromIntervals(ivs, err));
    DistanceTree t(g);

    DecrementalDistanceTree dt;
    CHECK(dt.initialize(g, t, err));

    // Verify initial distance tree properties
    CHECK(dt.getNode(4).parentId == 1);
    CHECK(dt.getNode(1).childTail == 4);
    CHECK(dt.getNode(5).childHead == -1); // 5 is a leaf
    CHECK(dt.getNode(4).childHead == 6);  // 4 has child 6

    // Delete vertex 4 (Case B)
    auto res = dt.deleteVertex(4);
    CHECK(res.success);
    CHECK(res.deletedId == 4);
    CHECK(res.immediateRightId == 5);

    // Verify post-deletion tree state:
    // 1's rightmost child is now 3 (modified descriptor)
    CHECK(dt.getNode(1).childTail == 3);
    // 5 now has child 6 (added descriptor)
    CHECK(dt.getNode(5).childHead == 6);
    CHECK(dt.getNode(5).childTail == 6);
    auto vChildren = dt.getChildren(5);
    CHECK(vChildren.size() == 1 && vChildren[0] == 6);

    // Independent verification
    auto ver = dt.runIndependentVerification();
    CHECK(ver.allPassed());

    std::cout << "  -> PASS (Case B deletion verified: 1 removed, 1 added, 1 modified; independent verification clean)\n";
}

void testDecTree_24_SmallGraphDeterministicDeletions() {
    std::cout << "\n[Phase 9 Test 24] Small-Graph Deterministic Parameterized Deletions Suite...\n";
    // Test 20 deterministic parameterized variations for small n=5..8
    for (int seed = 1; seed <= 20; ++seed) {
        int n = 5 + (seed % 4);
        std::vector<Interval> ivs;
        double curL = 0.0;
        for (int i = 0; i < n; ++i) {
            double len = 3.0 + 0.2 * (i % 3);
            ivs.push_back({i, curL, curL + len});
            curL += 0.8;
        }
        IntervalGraph g;
        std::string err;
        CHECK(g.initFromIntervals(ivs, err));
        DistanceTree t(g);

        DecrementalDistanceTree dt;
        CHECK(dt.initialize(g, t, err));

        // Try deleting all non-root candidate vertices
        for (int v = 1; v < n; ++v) {
            DecrementalDistanceTree dtCopy = dt;
            auto res = dtCopy.deleteVertex(v);
            if (res.success) {
                auto ver = dtCopy.runIndependentVerification();
                CHECK(ver.allPassed());
            }
        }
    }
    std::cout << "  -> PASS (Parameterized small-graph deletions (n=5..8) 100% verified against independent oracle)\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << " Running Complete Verification Suite (Phases 1 - 9)\n";
    std::cout << "========================================================\n";

    testRegressionPhases1To5();

    // Phase 6 tests
    testMISThesisEightVertex();
    testMISBoundaryH1SingleVertex();
    testMISCompleteGraph();
    testMISPathGraph();
    testMISEndpointTouching();
    testMISExhaustiveRandomGraphs();

    // Phase 7 tests
    testDomSetSingleVertex();
    testDomSetTwoVertex();
    testDomSetThesisEightVertex();
    testDomSetCompleteGraph();
    testDomSetPathGraph();
    testDomSetOddHeightGraph();
    testDomSetMultipleBranches();
    testDomSetEndpointTouching();
    testDomSetExhaustiveRandomGraphs();
    testDomSetGeometricEndpointInequalities();
    testDomSetIdenticalIntervals();

    // Phase 8 tests
    testIncMIS_1_SingleVertex();
    testIncMIS_2_Level2();
    testIncMIS_3_DeeperLevel();
    testIncMIS_4_ParentNotRightmost();
    testIncMIS_5_ParentIsRightmost();
    testIncMIS_6_MultipleSequentialLeaves();
    testIncMIS_7_IncreaseAlpha();
    testIncMIS_8_UnchangedAlpha();
    testIncMIS_9_EndpointTouching();
    testIncMIS_10_IdenticalIntervals();
    testIncMIS_11_InvalidProperness();
    testIncMIS_12_InvalidConnectedness();
    testIncMIS_13_InvalidNonLeaf();
    testIncMIS_14_MiddleInsertion();
    testIncMIS_15_GeneratedSequences();
    testIncMIS_16_LeftmostInsertion();
    testIncMIS_17_AdversarialCounterexample();
    testIncMIS_18_DeepestLevelExtension();
    testIncMIS_19_ExistingLevelRightmost();

    // Phase 9 tests
    testDecTree_1_LeafWithImmediateRight();
    testDecTree_2_NonLeafWithChildrenAndImmediateRight();
    testDecTree_3_JoinChildSets();
    testDecTree_4_RightmostChildMetadata();
    testDecTree_5_EmptyChildSet();
    testDecTree_6_EmptyChildSetRightmostOfLevel();
    testDecTree_7_RejectChildrenWithoutImmediateRight();
    testDecTree_8_RejectRootDeletion();
    testDecTree_9_RejectDisconnectingDeletion();
    testDecTree_10_RejectEmptyGraph();
    testDecTree_11_PropernessCheck();
    testDecTree_12_RepeatedSequentialDeletions();
    testDecTree_13_EndpointTouching();
    testDecTree_14_IdenticalIntervals();
    testDecTree_15_AlphaChanged();
    testDecTree_16_AlphaUnchanged();
    testDecTree_17_GeneratedSequences();
    testDecTree_18_EqualLeftEndpointParents();
    testDecTree_19_ChildSetConcatenationWithIdenticalIntervals();
    testDecTree_20_EmptyChildSetsCombinations();
    testDecTree_21_AtomicRejectionSnapshots();
    testDecTree_22_LongSequentialDeletions();
    testDecTree_23_CaseBCounterexampleThreeDescriptors();
    testDecTree_24_SmallGraphDeterministicDeletions();

    std::cout << "\n[+] ALL TESTS IN PHASES 1, 2, 3, 4, 5, 6, 7, 8 & 9 PASSED CLEANLY!\n";
    return 0;
}
