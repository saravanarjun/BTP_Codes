#ifndef BRON_KERBOSCH_ADAPTER_H
#define BRON_KERBOSCH_ADAPTER_H

#include "IntervalGraph.h"
#include <vector>
#include <chrono>

struct BronKerboschResult {
    std::vector<std::vector<int>> cliques;
    bool timedOut;
    double elapsedMs;
    long long recursiveCalls;
};

class BronKerboschAdapter {
public:
    // Direct call to approved Phase 5 implementation (MaximalCliques::enumerateBronKerbosch)
    // Suitable for small verification graphs without timeout protection
    static std::vector<std::vector<int>> runApprovedPhase5(const IntervalGraph& graph);

    // Algorithm B: Pivoted Bron-Kerbosch matching Phase 5 with safe timeout
    // Checks elapsed time every 500 recursive calls to prevent indefinite freezing
    static BronKerboschResult runPivotedWithTimeout(const IntervalGraph& graph, double timeoutMs = 3000.0);

    // Optional Fourth Curve: Naive (non-pivoted) Bron-Kerbosch with safe timeout
    static BronKerboschResult runNaiveWithTimeout(const IntervalGraph& graph, double timeoutMs = 3000.0);

private:
    static void recursivePivoted(
        std::vector<int>& R, std::vector<int>& P, std::vector<int>& X,
        const IntervalGraph& graph, std::vector<std::vector<int>>& result,
        const std::chrono::steady_clock::time_point& startTime, double timeoutMs,
        long long& callCount, bool& timedOut);

    static void recursiveNaive(
        std::vector<int>& R, std::vector<int>& P, std::vector<int>& X,
        const IntervalGraph& graph, std::vector<std::vector<int>>& result,
        const std::chrono::steady_clock::time_point& startTime, double timeoutMs,
        long long& callCount, bool& timedOut);
};

#endif // BRON_KERBOSCH_ADAPTER_H
