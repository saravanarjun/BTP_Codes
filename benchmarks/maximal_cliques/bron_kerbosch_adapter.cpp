#include "bron_kerbosch_adapter.h"
#include "MaximalCliques.h"
#include <algorithm>

std::vector<std::vector<int>> BronKerboschAdapter::runApprovedPhase5(const IntervalGraph& graph) {
    return MaximalCliques::enumerateBronKerbosch(graph);
}

void BronKerboschAdapter::recursivePivoted(
    std::vector<int>& R, std::vector<int>& P, std::vector<int>& X,
    const IntervalGraph& graph, std::vector<std::vector<int>>& result,
    const std::chrono::steady_clock::time_point& startTime, double timeoutMs,
    long long& callCount, bool& timedOut) {

    if (timedOut) return;

    callCount++;
    if ((callCount % 500) == 0) {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - startTime).count();
        if (elapsed > timeoutMs) {
            timedOut = true;
            return;
        }
    }

    if (P.empty() && X.empty()) {
        std::vector<int> clique = R;
        std::sort(clique.begin(), clique.end(), [&](int a, int b) {
            return graph.getSortedIndexById(a) < graph.getSortedIndexById(b);
        });
        result.push_back(clique);
        return;
    }

    // Tomita pivot selection: choose pivot u in P U X maximizing |P \cap N(u)|
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

    // Candidates in P \ N(pivot)
    std::vector<int> candidates;
    for (int v : P) {
        if (v == pivot || !graph.isAdjacent(pivot, v)) {
            candidates.push_back(v);
        }
    }

    for (int v : candidates) {
        if (timedOut) return;

        R.push_back(v);

        std::vector<int> newP, newX;
        for (int p : P) {
            if (p != v && graph.isAdjacent(v, p)) newP.push_back(p);
        }
        for (int x : X) {
            if (x != v && graph.isAdjacent(v, x)) newX.push_back(x);
        }

        recursivePivoted(R, newP, newX, graph, result, startTime, timeoutMs, callCount, timedOut);

        R.pop_back();

        P.erase(std::remove(P.begin(), P.end(), v), P.end());
        X.push_back(v);
    }
}

void BronKerboschAdapter::recursiveNaive(
    std::vector<int>& R, std::vector<int>& P, std::vector<int>& X,
    const IntervalGraph& graph, std::vector<std::vector<int>>& result,
    const std::chrono::steady_clock::time_point& startTime, double timeoutMs,
    long long& callCount, bool& timedOut) {

    if (timedOut) return;

    callCount++;
    if ((callCount % 500) == 0) {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - startTime).count();
        if (elapsed > timeoutMs) {
            timedOut = true;
            return;
        }
    }

    if (P.empty() && X.empty()) {
        std::vector<int> clique = R;
        std::sort(clique.begin(), clique.end(), [&](int a, int b) {
            return graph.getSortedIndexById(a) < graph.getSortedIndexById(b);
        });
        result.push_back(clique);
        return;
    }

    // Naive (no-pivot): iterate over all candidates currently in P
    std::vector<int> candidates = P;

    for (int v : candidates) {
        if (timedOut) return;

        R.push_back(v);

        std::vector<int> newP, newX;
        for (int p : P) {
            if (p != v && graph.isAdjacent(v, p)) newP.push_back(p);
        }
        for (int x : X) {
            if (x != v && graph.isAdjacent(v, x)) newX.push_back(x);
        }

        recursiveNaive(R, newP, newX, graph, result, startTime, timeoutMs, callCount, timedOut);

        R.pop_back();

        P.erase(std::remove(P.begin(), P.end(), v), P.end());
        X.push_back(v);
    }
}

BronKerboschResult BronKerboschAdapter::runPivotedWithTimeout(const IntervalGraph& graph, double timeoutMs) {
    BronKerboschResult res;
    res.timedOut = false;
    res.recursiveCalls = 0;

    int n = graph.getNumVertices();
    if (n == 0) {
        res.elapsedMs = 0.0;
        return res;
    }

    auto start = std::chrono::steady_clock::now();

    std::vector<int> R;
    std::vector<int> P;
    P.reserve(n);
    for (int i = 0; i < n; ++i) {
        P.push_back(graph.getIntervalBySortedIndex(i).id);
    }
    std::vector<int> X;

    recursivePivoted(R, P, X, graph, res.cliques, start, timeoutMs, res.recursiveCalls, res.timedOut);

    auto end = std::chrono::steady_clock::now();
    res.elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();

    if (!res.timedOut) {
        std::sort(res.cliques.begin(), res.cliques.end());
    } else {
        res.cliques.clear(); // Discard partial results on timeout
    }

    return res;
}

BronKerboschResult BronKerboschAdapter::runNaiveWithTimeout(const IntervalGraph& graph, double timeoutMs) {
    BronKerboschResult res;
    res.timedOut = false;
    res.recursiveCalls = 0;

    int n = graph.getNumVertices();
    if (n == 0) {
        res.elapsedMs = 0.0;
        return res;
    }

    auto start = std::chrono::steady_clock::now();

    std::vector<int> R;
    std::vector<int> P;
    P.reserve(n);
    for (int i = 0; i < n; ++i) {
        P.push_back(graph.getIntervalBySortedIndex(i).id);
    }
    std::vector<int> X;

    recursiveNaive(R, P, X, graph, res.cliques, start, timeoutMs, res.recursiveCalls, res.timedOut);

    auto end = std::chrono::steady_clock::now();
    res.elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();

    if (!res.timedOut) {
        std::sort(res.cliques.begin(), res.cliques.end());
    } else {
        res.cliques.clear();
    }

    return res;
}
