#include "graph_generator.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

// 1. Equal-length intervals with regularly spaced endpoints
IntervalGraph GraphGenerator::generateEqualLengthsGraph(int n, OverlapProfile profile) {
    double step = 1.0;
    double width = 1.5;
    if (profile == OverlapProfile::MEDIUM_OVERLAP) {
        width = 8.0;
    } else if (profile == OverlapProfile::DENSE_OVERLAP) {
        width = 25.0;
    }
    return generateEqualLengthsGraph(n, step, width);
}

IntervalGraph GraphGenerator::generateEqualLengthsGraph(int n, double step, double width) {
    if (n <= 0) throw std::invalid_argument("Graph size n must be positive.");
    std::vector<Interval> intervals;
    intervals.reserve(n);
    for (int i = 0; i < n; ++i) {
        double left = static_cast<double>(i) * step;
        double right = left + width;
        intervals.push_back(Interval{i, left, right});
    }
    IntervalGraph g;
    std::string err;
    g.initFromIntervals(intervals, err);
    return g;
}

// 2. Unequal-length proper intervals with strictly increasing endpoints
IntervalGraph GraphGenerator::generateUnequalLengthsGraph(int n, double baseStep, double minW, double maxW) {
    if (n <= 0) throw std::invalid_argument("Graph size n must be positive.");
    std::vector<Interval> intervals;
    intervals.reserve(n);

    double curLeft = 0.0;
    double prevRight = -1.0;

    for (int i = 0; i < n; ++i) {
        double factor = 0.5 * (1.0 + std::sin(static_cast<double>(i) * 0.3));
        double width = minW + factor * (maxW - minW);

        if (i > 0) {
            curLeft += baseStep;
            if (curLeft + width <= prevRight) {
                width = (prevRight - curLeft) + baseStep * 0.5;
            }
        }
        double curRight = curLeft + width;
        prevRight = curRight;

        intervals.push_back(Interval{i, curLeft, curRight});
    }

    IntervalGraph g;
    std::string err;
    g.initFromIntervals(intervals, err);
    return g;
}

// 3. Randomized proper interval graphs (with deterministic seed)
IntervalGraph GraphGenerator::generateRandomProperGraph(int n, uint32_t seed, double minW, double maxW, double stepMean) {
    if (n <= 0) throw std::invalid_argument("Graph size n must be positive.");
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> stepDist(stepMean * 0.5, stepMean * 1.5);
    std::uniform_real_distribution<double> widthDist(minW, maxW);

    std::vector<Interval> intervals;
    intervals.reserve(n);

    double curLeft = 0.0;
    double prevRight = -1.0;

    for (int i = 0; i < n; ++i) {
        if (i > 0) {
            double step = stepDist(rng);
            curLeft += step;
        }
        double w = widthDist(rng);
        if (i > 0 && curLeft > prevRight) {
            curLeft = prevRight - 0.1; // enforce overlap
        }
        double curRight = curLeft + w;
        if (i > 0 && curRight <= prevRight) {
            curRight = prevRight + 0.2;
        }
        prevRight = curRight;

        intervals.push_back(Interval{i, curLeft, curRight});
    }

    IntervalGraph g;
    std::string err;
    g.initFromIntervals(intervals, err);
    return g;
}

// 4. Identical interval clusters (multiple intervals sharing identical endpoints)
IntervalGraph GraphGenerator::generateIdenticalIntervalsGraph(int numUnique, int clusterSize, double step, double width) {
    if (numUnique <= 0 || clusterSize <= 0) throw std::invalid_argument("Counts must be positive.");
    std::vector<Interval> intervals;
    int total = numUnique * clusterSize;
    intervals.reserve(total);

    int idCounter = 0;
    for (int u = 0; u < numUnique; ++u) {
        double left = static_cast<double>(u) * step;
        double right = left + width;
        for (int c = 0; c < clusterSize; ++c) {
            intervals.push_back(Interval{idCounter++, left, right});
        }
    }

    IntervalGraph g;
    std::string err;
    g.initFromIntervals(intervals, err);
    return g;
}

// 5. Endpoint-touching intervals under closed-interval semantics (r_i = l_{i+1})
IntervalGraph GraphGenerator::generateEndpointTouchingGraph(int n, double width) {
    if (n <= 0) throw std::invalid_argument("Graph size n must be positive.");
    std::vector<Interval> intervals;
    intervals.reserve(n);

    for (int i = 0; i < n; ++i) {
        double left = static_cast<double>(i) * width;
        double right = left + width;
        intervals.push_back(Interval{i, left, right});
    }

    IntervalGraph g;
    std::string err;
    g.initFromIntervals(intervals, err);
    return g;
}

// 6. Long chains of small-overlap intervals (path-like chain, k = 1)
IntervalGraph GraphGenerator::generateLongChainGraph(int n) {
    return generateEqualLengthsGraph(n, 1.0, 1.2);
}

// 7. High-overlap interval families (large bandwidth, broad maximal cliques)
IntervalGraph GraphGenerator::generateHighOverlapGraph(int n, int bandwidth) {
    double step = 1.0;
    double width = static_cast<double>(bandwidth) * step + 0.5;
    return generateEqualLengthsGraph(n, step, width);
}

// 8. Alternating short and long interval lengths while preserving properness
IntervalGraph GraphGenerator::generateAlternatingLengthsGraph(int n, double shortW, double longW, double step) {
    if (n <= 0) throw std::invalid_argument("Graph size n must be positive.");
    std::vector<Interval> intervals;
    intervals.reserve(n);

    double curLeft = 0.0;
    double prevRight = -1.0;

    for (int i = 0; i < n; ++i) {
        double w = (i % 2 == 0) ? shortW : longW;
        if (i > 0) {
            curLeft += step;
            if (curLeft + w <= prevRight) {
                w = (prevRight - curLeft) + 0.5;
            }
        }
        double curRight = curLeft + w;
        prevRight = curRight;

        intervals.push_back(Interval{i, curLeft, curRight});
    }

    IntervalGraph g;
    std::string err;
    g.initFromIntervals(intervals, err);
    return g;
}

// 9. Irregularly spaced endpoints
IntervalGraph GraphGenerator::generateIrregularEndpointsGraph(int n, uint32_t seed) {
    if (n <= 0) throw std::invalid_argument("Graph size n must be positive.");
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> deltaDist(0.2, 1.8);

    std::vector<Interval> intervals;
    intervals.reserve(n);

    double curLeft = 0.0;
    double width = 4.0;
    for (int i = 0; i < n; ++i) {
        if (i > 0) {
            curLeft += deltaDist(rng);
        }
        intervals.push_back(Interval{i, curLeft, curLeft + width});
    }

    IntervalGraph g;
    std::string err;
    g.initFromIntervals(intervals, err);
    return g;
}

// 10. Graphs with many small maximal cliques (k = 2)
IntervalGraph GraphGenerator::generateManySmallCliquesGraph(int n) {
    return generateEqualLengthsGraph(n, 1.0, 2.5);
}

// 11. Graphs with fewer but large maximal cliques
IntervalGraph GraphGenerator::generateFewLargeCliquesGraph(int n) {
    double width = std::max(5.0, static_cast<double>(n) * 0.4);
    return generateEqualLengthsGraph(n, 1.0, width);
}

// 12. Graphs with varying maximal-clique sizes and overlap patterns
IntervalGraph GraphGenerator::generateVaryingOverlapGraph(int n) {
    if (n <= 0) throw std::invalid_argument("Graph size n must be positive.");
    std::vector<Interval> intervals;
    intervals.reserve(n);

    double curLeft = 0.0;
    double prevRight = -1.0;
    for (int i = 0; i < n; ++i) {
        double phase = 2.0 * 3.141592653589793 * static_cast<double>(i) / 50.0;
        double width = 3.0 + 5.0 * (0.5 * (1.0 + std::sin(phase)));
        if (i > 0) {
            curLeft += 1.0;
            if (curLeft + width <= prevRight) {
                width = (prevRight - curLeft) + 0.5;
            }
        }
        double curRight = curLeft + width;
        prevRight = curRight;

        intervals.push_back(Interval{i, curLeft, curRight});
    }

    IntervalGraph g;
    std::string err;
    g.initFromIntervals(intervals, err);
    return g;
}

// 13. Adversarial cases for parent selection and tie-breaking
IntervalGraph GraphGenerator::generateAdversarialTieBreakingGraph(int n) {
    if (n <= 0) throw std::invalid_argument("Graph size n must be positive.");
    std::vector<Interval> intervals;
    intervals.reserve(n);

    int idCounter = 0;
    int groupSize = 2;
    int numGroups = (n + groupSize - 1) / groupSize;

    for (int grp = 0; grp < numGroups && idCounter < n; ++grp) {
        double left = static_cast<double>(grp) * 1.5;
        double right = left + 3.0; // identical endpoints in group preserve properness while testing canonical tie-breaking
        for (int k = 0; k < groupSize && idCounter < n; ++k) {
            intervals.push_back(Interval{idCounter++, left, right});
        }
    }

    IntervalGraph g;
    std::string err;
    g.initFromIntervals(intervals, err);
    return g;
}

// 14. Graphs with shuffled vertex IDs (verifying independence from ID == sorted index)
IntervalGraph GraphGenerator::generateShuffledIdsGraph(int n, uint32_t seed) {
    if (n <= 0) throw std::invalid_argument("Graph size n must be positive.");
    std::vector<Interval> baseIntervals;
    baseIntervals.reserve(n);
    for (int i = 0; i < n; ++i) {
        double left = static_cast<double>(i) * 1.0;
        double right = left + 3.5;
        baseIntervals.push_back(Interval{i, left, right});
    }

    std::vector<int> permutedIds(n);
    for (int i = 0; i < n; ++i) permutedIds[i] = i * 10 + 7;
    std::mt19937 rng(seed);
    std::shuffle(permutedIds.begin(), permutedIds.end(), rng);

    for (int i = 0; i < n; ++i) {
        baseIntervals[i].id = permutedIds[i];
    }

    IntervalGraph g;
    std::string err;
    g.initFromIntervals(baseIntervals, err);
    return g;
}

// Graph validation helper
bool GraphGenerator::validateProperConnectedGraph(const IntervalGraph& graph, std::string& outDiagnostic) {
    int n = graph.getNumVertices();
    if (n <= 0) {
        outDiagnostic = "Graph is empty.";
        return false;
    }

    const auto& sorted = graph.getSortedIntervals();

    // 1. Verify properness: no interval strictly contains another
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const auto& a = sorted[i];
            const auto& b = sorted[j];
            if (a.left <= b.left && a.right >= b.right && (a.left < b.left || a.right > b.right)) {
                std::ostringstream oss;
                oss << "Properness violation: Interval " << a.id << " [" << a.left << ", " << a.right
                    << "] strictly contains interval " << b.id << " [" << b.left << ", " << b.right << "].";
                outDiagnostic = oss.str();
                return false;
            }
        }
    }

    // 2. Verify connectedness: consecutive intervals intersect
    for (int i = 0; i + 1 < n; ++i) {
        if (sorted[i].right < sorted[i + 1].left) {
            std::ostringstream oss;
            oss << "Connectedness violation: Disconnected gap between interval " << sorted[i].id
                << " right=" << sorted[i].right << " and interval " << sorted[i + 1].id
                << " left=" << sorted[i + 1].left << ".";
            outDiagnostic = oss.str();
            return false;
        }
    }

    return true;
}
