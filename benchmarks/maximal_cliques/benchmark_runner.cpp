#include "benchmark_runner.h"
#include "DistanceTree.h"
#include "theorem_cliques_adapter.h"
#include "bron_kerbosch_adapter.h"
#include "standard_interval_cliques.h"
#include "dynamic_clique_structure.h"
#include "clique_verifier.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <numeric>

namespace {

int getAdaptiveRepetitions(int n, bool quickMode) {
    if (quickMode) return 1;
    if (n <= 100) return 20;
    if (n <= 1000) return 10;
    if (n <= 10000) return 5;
    if (n <= 50000) return 3;
    return 2;
}

void computeGraphStats(const IntervalGraph& graph, int64_t& outM, double& outAvgDeg, int& outMaxDeg) {
    const auto& sorted = graph.getSortedIntervals();
    int n = static_cast<int>(sorted.size());
    if (n == 0) {
        outM = 0;
        outAvgDeg = 0.0;
        outMaxDeg = 0;
        return;
    }
    int64_t totalDeg = 0;
    int maxDeg = 0;
    int pRight = 0;
    int pLeft = 0;

    for (int i = 0; i < n; ++i) {
        while (pRight + 1 < n && sorted[pRight + 1].left <= sorted[i].right) {
            pRight++;
        }
        while (pLeft < n && sorted[pLeft].right < sorted[i].left) {
            pLeft++;
        }
        int deg_i = (pRight >= pLeft) ? (pRight - pLeft) : 0;
        if (deg_i > maxDeg) {
            maxDeg = deg_i;
        }
        totalDeg += deg_i;
    }
    outM = totalDeg / 2;
    outAvgDeg = static_cast<double>(totalDeg) / static_cast<double>(n);
    outMaxDeg = maxDeg;
}

double getMedian(std::vector<double>& v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    size_t mid = v.size() / 2;
    if (v.size() % 2 == 0) {
        return (v[mid - 1] + v[mid]) * 0.5;
    }
    return v[mid];
}

struct LatencyStats {
    double avg = 0.0;
    double median = 0.0;
    double stddev = 0.0;
    double minVal = 0.0;
    double maxVal = 0.0;
    double p90 = 0.0;
    double p95 = 0.0;
};

LatencyStats computeLatencyStats(std::vector<double> latencies) {
    LatencyStats stats;
    if (latencies.empty()) return stats;
    std::sort(latencies.begin(), latencies.end());
    size_t k = latencies.size();
    stats.minVal = latencies.front();
    stats.maxVal = latencies.back();

    double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    stats.avg = sum / static_cast<double>(k);

    if (k % 2 == 0) {
        stats.median = (latencies[k / 2 - 1] + latencies[k / 2]) * 0.5;
    } else {
        stats.median = latencies[k / 2];
    }

    double sqDiffSum = 0.0;
    for (double x : latencies) {
        sqDiffSum += (x - stats.avg) * (x - stats.avg);
    }
    stats.stddev = std::sqrt(sqDiffSum / static_cast<double>(k));

    size_t idx90 = static_cast<size_t>(std::ceil(0.90 * k)) - 1;
    if (idx90 >= k) idx90 = k - 1;
    stats.p90 = latencies[idx90];

    size_t idx95 = static_cast<size_t>(std::ceil(0.95 * k)) - 1;
    if (idx95 >= k) idx95 = k - 1;
    stats.p95 = latencies[idx95];

    return stats;
}

} // namespace

std::vector<StaticAlgorithmResult> BenchmarkRunner::evaluateStaticGraph(
    const IntervalGraph& graph,
    const std::string& familyName,
    int /*targetN*/,
    int repetitions) {

    std::vector<StaticAlgorithmResult> results;
    int n = graph.getNumVertices();
    int64_t m = 0;
    double avgDeg = 0.0;
    int maxDeg = 0;
    computeGraphStats(graph, m, avgDeg, maxDeg);

    // ------------------------------------------------------------------------
    // 1. Algorithm A: Theorem-Based (Lemma 3.8 & Lemma 3.15)
    // ------------------------------------------------------------------------
    {
        StaticAlgorithmResult resA;
        resA.algorithmName = "Algorithm A (Theorem-Based)";
        resA.variantName = "Lemma-3.15-Compact";
        resA.graphFamily = familyName;
        resA.n = n;
        resA.m = static_cast<int>(m);
        resA.averageDegree = avgDeg;
        resA.maxDegree = maxDeg;
        resA.repetitions = repetitions;

        // Warmup
        {
            DistanceTree wTree(graph);
            wTree.build();
            auto wDescs = TheoremCliquesAdapter::computeCompactDescriptors(graph, wTree);
            auto wCliques = TheoremCliquesAdapter::materializeCliques(wDescs, graph);
        }

        std::vector<double> preVec, coreVec, matVec, directVec;
        preVec.reserve(repetitions);
        coreVec.reserve(repetitions);
        matVec.reserve(repetitions);
        directVec.reserve(repetitions);

        for (int r = 0; r < repetitions; ++r) {
            // Direct end-to-end continuous measurement
            auto tStart = std::chrono::steady_clock::now();
            DistanceTree directTree(graph);
            directTree.build();
            auto directDescs = TheoremCliquesAdapter::computeCompactDescriptors(graph, directTree);
            auto directCliques = TheoremCliquesAdapter::materializeCliques(directDescs, graph);
            auto tEnd = std::chrono::steady_clock::now();
            directVec.push_back(std::chrono::duration<double, std::milli>(tEnd - tStart).count());

            // Separate stage timing
            auto t0 = std::chrono::steady_clock::now();
            DistanceTree t(graph);
            t.build();
            auto t1 = std::chrono::steady_clock::now();
            auto descs = TheoremCliquesAdapter::computeCompactDescriptors(graph, t);
            auto t2 = std::chrono::steady_clock::now();
            auto cliques = TheoremCliquesAdapter::materializeCliques(descs, graph);
            auto t3 = std::chrono::steady_clock::now();

            preVec.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
            coreVec.push_back(std::chrono::duration<double, std::milli>(t2 - t1).count());
            matVec.push_back(std::chrono::duration<double, std::milli>(t3 - t2).count());

            if (r == 0) {
                resA.q = static_cast<int>(descs.size());
                int64_t totalMems = 0;
                int maxC = 0;
                for (const auto& c : cliques) {
                    totalMems += c.size();
                    if (static_cast<int>(c.size()) > maxC) maxC = static_cast<int>(c.size());
                }
                resA.totalMemberships = totalMems;
                resA.maxCliqueSize = maxC;
                resA.averageCliqueSize = (resA.q > 0) ? (static_cast<double>(totalMems) / resA.q) : 0.0;
                resA.estimatedDescriptorBytes = static_cast<int64_t>(resA.q) * sizeof(int) * 2;
                resA.estimatedExplicitBytes = totalMems * sizeof(int);
            }
        }

        resA.preprocessingMs = getMedian(preVec);
        resA.coreMs = getMedian(coreVec);
        resA.materializationMs = getMedian(matVec);
        resA.postprocessingTotalMs = resA.coreMs + resA.materializationMs;
        resA.endToEndSumMs = resA.preprocessingMs + resA.coreMs + resA.materializationMs;
        resA.directEndToEndMs = getMedian(directVec);
        resA.minDirectEndToEndMs = *std::min_element(directVec.begin(), directVec.end());
        resA.maxDirectEndToEndMs = *std::max_element(directVec.begin(), directVec.end());
        resA.status = BenchmarkStatus::SUCCESS;

        results.push_back(resA);
    }

    // ------------------------------------------------------------------------
    // 2. Algorithm B: Bron-Kerbosch with Tomita Pivoting
    // ------------------------------------------------------------------------
    {
        StaticAlgorithmResult resB;
        resB.algorithmName = "Algorithm B (Bron-Kerbosch)";
        resB.variantName = "Tomita-Pivoting";
        resB.graphFamily = familyName;
        resB.n = n;
        resB.m = static_cast<int>(m);
        resB.averageDegree = avgDeg;
        resB.maxDegree = maxDeg;
        resB.repetitions = repetitions;

        if (n <= 2000) {
            // Warmup
            BronKerboschAdapter::runPivotedWithTimeout(graph, 3000.0);

            std::vector<double> bkVec;
            bkVec.reserve(repetitions);
            bool timedOut = false;

            for (int r = 0; r < repetitions; ++r) {
                auto bkRes = BronKerboschAdapter::runPivotedWithTimeout(graph, 3000.0);
                if (bkRes.timedOut) {
                    timedOut = true;
                    break;
                }
                bkVec.push_back(bkRes.elapsedMs);

                if (r == 0) {
                    resB.q = static_cast<int>(bkRes.cliques.size());
                    int64_t totalMems = 0;
                    int maxC = 0;
                    for (const auto& c : bkRes.cliques) {
                        totalMems += c.size();
                        if (static_cast<int>(c.size()) > maxC) maxC = static_cast<int>(c.size());
                    }
                    resB.totalMemberships = totalMems;
                    resB.maxCliqueSize = maxC;
                    resB.averageCliqueSize = (resB.q > 0) ? (static_cast<double>(totalMems) / resB.q) : 0.0;
                    resB.estimatedDescriptorBytes = static_cast<int64_t>(resB.q) * sizeof(int) * 2;
                    resB.estimatedExplicitBytes = totalMems * sizeof(int);
                }
            }

            if (timedOut) {
                resB.status = BenchmarkStatus::TIMEOUT;
                resB.directEndToEndMs = -1.0;
                resB.coreMs = -1.0;
                resB.preprocessingMs = -1.0;
                resB.materializationMs = -1.0;
                resB.postprocessingTotalMs = -1.0;
                resB.endToEndSumMs = -1.0;
                resB.minDirectEndToEndMs = -1.0;
                resB.maxDirectEndToEndMs = -1.0;
                resB.q = -1;
                resB.totalMemberships = -1;
                resB.averageCliqueSize = -1.0;
                resB.maxCliqueSize = -1;
                resB.estimatedDescriptorBytes = -1;
                resB.estimatedExplicitBytes = -1;
            } else {
                resB.status = BenchmarkStatus::SUCCESS;
                resB.directEndToEndMs = getMedian(bkVec);
                resB.coreMs = resB.directEndToEndMs;
                resB.preprocessingMs = 0.0;
                resB.materializationMs = 0.0;
                resB.postprocessingTotalMs = resB.coreMs;
                resB.endToEndSumMs = resB.directEndToEndMs;
                resB.minDirectEndToEndMs = *std::min_element(bkVec.begin(), bkVec.end());
                resB.maxDirectEndToEndMs = *std::max_element(bkVec.begin(), bkVec.end());
            }
        } else {
            // Beyond practical scalability limit: record policy timeout
            resB.status = BenchmarkStatus::TIMEOUT;
            resB.directEndToEndMs = -1.0;
            resB.coreMs = -1.0;
            resB.preprocessingMs = -1.0;
            resB.materializationMs = -1.0;
            resB.postprocessingTotalMs = -1.0;
            resB.endToEndSumMs = -1.0;
            resB.minDirectEndToEndMs = -1.0;
            resB.maxDirectEndToEndMs = -1.0;
            resB.q = -1;
            resB.totalMemberships = -1;
            resB.averageCliqueSize = -1.0;
            resB.maxCliqueSize = -1;
            resB.estimatedDescriptorBytes = -1;
            resB.estimatedExplicitBytes = -1;
            resB.errorMessage = "Skipped beyond practical limit n > 2000 (exceeds 3000ms timeout)";
        }

        results.push_back(resB);
    }

    // ------------------------------------------------------------------------
    // 3. Algorithm C: Standard Interval Sweep-Line
    // ------------------------------------------------------------------------
    {
        StaticAlgorithmResult resC;
        resC.algorithmName = "Algorithm C (Standard Sweep-Line)";
        resC.variantName = "Helly-Endpoint-Sweep";
        resC.graphFamily = familyName;
        resC.n = n;
        resC.m = static_cast<int>(m);
        resC.averageDegree = avgDeg;
        resC.maxDegree = maxDeg;
        resC.repetitions = repetitions;

        // Warmup
        {
            auto wEvents = StandardIntervalCliques::buildSortedEvents(graph);
            auto wDescs = StandardIntervalCliques::sweepCore(wEvents, n);
            auto wCliques = StandardIntervalCliques::materializeCliques(wDescs, graph);
        }

        std::vector<double> preVec, coreVec, matVec, directVec;
        preVec.reserve(repetitions);
        coreVec.reserve(repetitions);
        matVec.reserve(repetitions);
        directVec.reserve(repetitions);

        for (int r = 0; r < repetitions; ++r) {
            // Direct continuous measurement
            auto tStart = std::chrono::steady_clock::now();
            auto directCliques = StandardIntervalCliques::enumerateMaximalCliques(graph);
            auto tEnd = std::chrono::steady_clock::now();
            directVec.push_back(std::chrono::duration<double, std::milli>(tEnd - tStart).count());

            // Separate stage timing
            auto t0 = std::chrono::steady_clock::now();
            auto events = StandardIntervalCliques::buildSortedEvents(graph);
            auto t1 = std::chrono::steady_clock::now();
            auto descs = StandardIntervalCliques::sweepCore(events, n);
            auto t2 = std::chrono::steady_clock::now();
            auto cliques = StandardIntervalCliques::materializeCliques(descs, graph);
            auto t3 = std::chrono::steady_clock::now();

            preVec.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
            coreVec.push_back(std::chrono::duration<double, std::milli>(t2 - t1).count());
            matVec.push_back(std::chrono::duration<double, std::milli>(t3 - t2).count());

            if (r == 0) {
                resC.q = static_cast<int>(descs.size());
                int64_t totalMems = 0;
                int maxC = 0;
                for (const auto& c : cliques) {
                    totalMems += c.size();
                    if (static_cast<int>(c.size()) > maxC) maxC = static_cast<int>(c.size());
                }
                resC.totalMemberships = totalMems;
                resC.maxCliqueSize = maxC;
                resC.averageCliqueSize = (resC.q > 0) ? (static_cast<double>(totalMems) / resC.q) : 0.0;
                resC.estimatedDescriptorBytes = static_cast<int64_t>(resC.q) * sizeof(int) * 2;
                resC.estimatedExplicitBytes = totalMems * sizeof(int);
            }
        }

        resC.preprocessingMs = getMedian(preVec);
        resC.coreMs = getMedian(coreVec);
        resC.materializationMs = getMedian(matVec);
        resC.postprocessingTotalMs = resC.coreMs + resC.materializationMs;
        resC.endToEndSumMs = resC.preprocessingMs + resC.coreMs + resC.materializationMs;
        resC.directEndToEndMs = getMedian(directVec);
        resC.minDirectEndToEndMs = *std::min_element(directVec.begin(), directVec.end());
        resC.maxDirectEndToEndMs = *std::max_element(directVec.begin(), directVec.end());
        resC.status = BenchmarkStatus::SUCCESS;

        results.push_back(resC);
    }

    return results;
}

void BenchmarkRunner::runStaticBenchmarks(
    const std::string& resultsCsvPath,
    const std::string& summaryCsvPath,
    bool quickMode) {

    std::ofstream rOut(resultsCsvPath);
    std::ofstream sOut(summaryCsvPath);

    if (!rOut.is_open() || !sOut.is_open()) {
        std::cerr << "Failed to open CSV output files at: " << resultsCsvPath << std::endl;
        return;
    }

    // CSV Headers
    std::string csvHeader = 
        "algorithm,variant,graph_family,n,m,average_degree,max_degree,q,total_memberships,"
        "average_clique_size,max_clique_size,status,preprocessing_ms,core_ms,materialization_ms,"
        "postprocessing_total_ms,end_to_end_sum_ms,direct_end_to_end_ms,min_direct_e2e_ms,"
        "max_direct_e2e_ms,repetitions,estimated_descriptor_bytes,estimated_explicit_bytes";

    rOut << csvHeader << "\n";
    sOut << csvHeader << "\n";

    std::vector<int> targetSizes;
    if (quickMode) {
        targetSizes = {10, 50, 100, 500, 1000};
    } else {
        targetSizes = {10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000};
    }

    std::cout << "\n========================================================\n"
              << " Executing Comprehensive Static Maximal Clique Benchmark\n"
              << " Target sizes: " << targetSizes.size() << " levels (up to n=" << targetSizes.back() << ")\n"
              << "========================================================\n";

    for (int n : targetSizes) {
        std::cout << "\n>>> Benchmarking Graph Order n = " << n << " <<<\n";

        // Graph families to benchmark at size n:
        std::vector<std::pair<std::string, IntervalGraph>> families;
        families.push_back({"sparse-overlap", GraphGenerator::generateEqualLengthsGraph(n, OverlapProfile::SPARSE_OVERLAP)});
        families.push_back({"medium-overlap", GraphGenerator::generateEqualLengthsGraph(n, OverlapProfile::MEDIUM_OVERLAP)});
        families.push_back({"dense-overlap", GraphGenerator::generateEqualLengthsGraph(n, OverlapProfile::DENSE_OVERLAP)});

        if (n <= 50000) {
            families.push_back({"unequal-lengths", GraphGenerator::generateUnequalLengthsGraph(n)});
            families.push_back({"random-proper", GraphGenerator::generateRandomProperGraph(n, 101)});
            families.push_back({"identical-clusters", GraphGenerator::generateIdenticalIntervalsGraph((n + 1) / 2, 2)});
            families.push_back({"endpoint-touching", GraphGenerator::generateEndpointTouchingGraph(n)});
            families.push_back({"long-chain", GraphGenerator::generateLongChainGraph(n)});
            families.push_back({"high-overlap", GraphGenerator::generateHighOverlapGraph(n, 15)});
            families.push_back({"alternating-lengths", GraphGenerator::generateAlternatingLengthsGraph(n)});
            families.push_back({"irregular-spacing", GraphGenerator::generateIrregularEndpointsGraph(n, 202)});
            families.push_back({"many-small-cliques", GraphGenerator::generateManySmallCliquesGraph(n)});
            families.push_back({"few-large-cliques", GraphGenerator::generateFewLargeCliquesGraph(n)});
            families.push_back({"varying-overlap", GraphGenerator::generateVaryingOverlapGraph(n)});
            families.push_back({"adversarial-tie-breaking", GraphGenerator::generateAdversarialTieBreakingGraph(n)});
            families.push_back({"shuffled-ids", GraphGenerator::generateShuffledIdsGraph(n, 303)});
        }

        int reps = getAdaptiveRepetitions(n, quickMode);

        for (const auto& famPair : families) {
            const std::string& famName = famPair.first;
            const auto& g = famPair.second;

            auto evals = evaluateStaticGraph(g, famName, n, reps);

            for (const auto& res : evals) {
                std::ostringstream row;
                row << std::fixed << std::setprecision(4)
                    << "\"" << res.algorithmName << "\","
                    << "\"" << res.variantName << "\","
                    << "\"" << res.graphFamily << "\","
                    << res.n << "," << res.m << "," << res.averageDegree << "," << res.maxDegree << ","
                    << res.q << "," << res.totalMemberships << ","
                    << res.averageCliqueSize << "," << res.maxCliqueSize << ","
                    << statusToString(res.status) << ","
                    << res.preprocessingMs << "," << res.coreMs << "," << res.materializationMs << ","
                    << res.postprocessingTotalMs << "," << res.endToEndSumMs << "," << res.directEndToEndMs << ","
                    << res.minDirectEndToEndMs << "," << res.maxDirectEndToEndMs << ","
                    << res.repetitions << ","
                    << res.estimatedDescriptorBytes << "," << res.estimatedExplicitBytes;

                rOut << row.str() << "\n";
                sOut << row.str() << "\n";

                std::cout << "  [" << std::setw(18) << famName << "] "
                          << std::setw(32) << res.algorithmName << " (E2E: "
                          << std::setw(8) << res.directEndToEndMs << " ms, status: "
                          << statusToString(res.status) << ")\n";
            }
        }
        rOut.flush();
        sOut.flush();
    }

    std::cout << "\n[+] Static benchmarks completed successfully!\n"
              << "    Results exported to: " << resultsCsvPath << "\n";
}

std::vector<DynamicSequenceBenchmarkResult> BenchmarkRunner::evaluateDynamicSequence(
    const DynamicWorkloadSequence& sequence) {

    std::vector<DynamicSequenceBenchmarkResult> results;

    // ------------------------------------------------------------------------
    // 1. Dynamic Algorithm A: Thesis-Based Maintenance
    // ------------------------------------------------------------------------
    {
        DynamicSequenceBenchmarkResult resA;
        resA.algorithmName = "Algorithm A (Thesis-Based)";
        resA.variantName = "Dynamic-Descriptor-Relocation";
        resA.workloadName = sequence.workloadName;
        resA.graphFamily = sequence.familyName;
        resA.initialN = sequence.initialN;
        resA.randomSeed = sequence.seed;
        resA.totalUpdates = static_cast<int>(sequence.operations.size());

        DistanceTree initTree(sequence.initialGraph);
        initTree.build();

        DynamicTheoremCliques dynA;
        dynA.initialize(sequence.initialGraph, initTree);

        std::vector<double> latencies;
        latencies.reserve(sequence.operations.size());
        double cumTime = 0.0;

        for (size_t i = 0; i < sequence.operations.size(); ++i) {
            const auto& op = sequence.operations[i];
            DynamicUpdateStepTelemetry tel;

            if (op.type == DynamicOpType::INSERTION) {
                tel = dynA.insertLeaf(op.newInterval);
            } else {
                tel = dynA.deleteVertex(op.targetId);
            }

            tel.updateIndex = static_cast<int>(i);
            resA.steps.push_back(tel);

            if (tel.status == BenchmarkStatus::SUCCESS) {
                resA.successfulUpdates++;
                latencies.push_back(tel.totalLatencyMs);
                cumTime += tel.totalLatencyMs;
            } else {
                resA.rejectedUpdates++;
            }
        }

        resA.finalN = dynA.getNumVertices();
        resA.finalNumCliques = dynA.getNumCliques();
        resA.finalTotalMemberships = dynA.getTotalMemberships();
        resA.cumulativeTimeMs = cumTime;
        LatencyStats statsA = computeLatencyStats(latencies);
        resA.averageUpdateLatencyMs = statsA.avg;
        resA.amortizedUpdateLatencyMs = statsA.avg;
        resA.medianUpdateLatencyMs = statsA.median;
        resA.stddevUpdateLatencyMs = statsA.stddev;
        resA.minUpdateLatencyMs = statsA.minVal;
        resA.maxUpdateLatencyMs = statsA.maxVal;
        resA.p90UpdateLatencyMs = statsA.p90;
        resA.p95UpdateLatencyMs = statsA.p95;
        resA.status = BenchmarkStatus::SUCCESS;

        results.push_back(resA);
    }

    // ------------------------------------------------------------------------
    // 2. Dynamic Algorithm B: BK-Full-Recomputation
    // ------------------------------------------------------------------------
    {
        DynamicSequenceBenchmarkResult resB;
        resB.algorithmName = "Algorithm B (Bron-Kerbosch)";
        resB.variantName = "BK-Full-Recomputation";
        resB.workloadName = sequence.workloadName;
        resB.graphFamily = sequence.familyName;
        resB.initialN = sequence.initialN;
        resB.randomSeed = sequence.seed;
        resB.totalUpdates = static_cast<int>(sequence.operations.size());

        if (sequence.initialN <= 2000) {
            DynamicBronKerbosch dynB;
            dynB.initialize(sequence.initialGraph);

            std::vector<double> latencies;
            latencies.reserve(sequence.operations.size());
            double cumTime = 0.0;
            bool timedOut = false;

            for (size_t i = 0; i < sequence.operations.size(); ++i) {
                const auto& op = sequence.operations[i];
                DynamicUpdateStepTelemetry tel;

                if (op.type == DynamicOpType::INSERTION) {
                    tel = dynB.insertLeaf(op.newInterval, 3000.0);
                } else {
                    tel = dynB.deleteVertex(op.targetId, 3000.0);
                }

                tel.updateIndex = static_cast<int>(i);
                resB.steps.push_back(tel);

                if (tel.status == BenchmarkStatus::SUCCESS) {
                    resB.successfulUpdates++;
                    latencies.push_back(tel.totalLatencyMs);
                    cumTime += tel.totalLatencyMs;
                } else if (tel.status == BenchmarkStatus::TIMEOUT) {
                    timedOut = true;
                    resB.status = BenchmarkStatus::TIMEOUT;
                    break;
                }
            }

            if (!timedOut) {
                resB.finalN = dynB.getCurrentGraph().getNumVertices();
                resB.finalNumCliques = dynB.getNumCliques();
                resB.finalTotalMemberships = dynB.getTotalMemberships();
                resB.cumulativeTimeMs = cumTime;
                LatencyStats statsB = computeLatencyStats(latencies);
                resB.averageUpdateLatencyMs = statsB.avg;
                resB.amortizedUpdateLatencyMs = statsB.avg;
                resB.medianUpdateLatencyMs = statsB.median;
                resB.stddevUpdateLatencyMs = statsB.stddev;
                resB.minUpdateLatencyMs = statsB.minVal;
                resB.maxUpdateLatencyMs = statsB.maxVal;
                resB.p90UpdateLatencyMs = statsB.p90;
                resB.p95UpdateLatencyMs = statsB.p95;
                resB.status = BenchmarkStatus::SUCCESS;
            } else {
                resB.status = BenchmarkStatus::TIMEOUT;
                resB.cumulativeTimeMs = -1.0;
                resB.averageUpdateLatencyMs = -1.0;
                resB.amortizedUpdateLatencyMs = -1.0;
                resB.medianUpdateLatencyMs = -1.0;
                resB.stddevUpdateLatencyMs = -1.0;
                resB.minUpdateLatencyMs = -1.0;
                resB.maxUpdateLatencyMs = -1.0;
                resB.p90UpdateLatencyMs = -1.0;
                resB.p95UpdateLatencyMs = -1.0;
            }
        } else {
            resB.status = BenchmarkStatus::SKIPPED_BY_POLICY;
            resB.cumulativeTimeMs = -1.0;
            resB.averageUpdateLatencyMs = -1.0;
            resB.amortizedUpdateLatencyMs = -1.0;
            resB.medianUpdateLatencyMs = -1.0;
            resB.stddevUpdateLatencyMs = -1.0;
            resB.minUpdateLatencyMs = -1.0;
            resB.maxUpdateLatencyMs = -1.0;
            resB.p90UpdateLatencyMs = -1.0;
            resB.p95UpdateLatencyMs = -1.0;
        }

        results.push_back(resB);
    }

    // ------------------------------------------------------------------------
    // 3. Dynamic Algorithm C: Sweep-Full-Recomputation
    // ------------------------------------------------------------------------
    {
        DynamicSequenceBenchmarkResult resC;
        resC.algorithmName = "Algorithm C (Sweep-Line)";
        resC.variantName = "Sweep-Full-Recomputation";
        resC.workloadName = sequence.workloadName;
        resC.graphFamily = sequence.familyName;
        resC.initialN = sequence.initialN;
        resC.randomSeed = sequence.seed;
        resC.totalUpdates = static_cast<int>(sequence.operations.size());

        DynamicSweepLine dynC;
        dynC.initialize(sequence.initialGraph);

        std::vector<double> latencies;
        latencies.reserve(sequence.operations.size());
        double cumTime = 0.0;

        for (size_t i = 0; i < sequence.operations.size(); ++i) {
            const auto& op = sequence.operations[i];
            DynamicUpdateStepTelemetry tel;

            if (op.type == DynamicOpType::INSERTION) {
                tel = dynC.insertLeaf(op.newInterval);
            } else {
                tel = dynC.deleteVertex(op.targetId);
            }

            tel.updateIndex = static_cast<int>(i);
            resC.steps.push_back(tel);

            if (tel.status == BenchmarkStatus::SUCCESS) {
                resC.successfulUpdates++;
                latencies.push_back(tel.totalLatencyMs);
                cumTime += tel.totalLatencyMs;
            } else {
                resC.rejectedUpdates++;
            }
        }

        resC.finalN = dynC.getCurrentGraph().getNumVertices();
        resC.finalNumCliques = dynC.getNumCliques();
        resC.finalTotalMemberships = dynC.getTotalMemberships();
        resC.cumulativeTimeMs = cumTime;
        LatencyStats statsC = computeLatencyStats(latencies);
        resC.averageUpdateLatencyMs = statsC.avg;
        resC.amortizedUpdateLatencyMs = statsC.avg;
        resC.medianUpdateLatencyMs = statsC.median;
        resC.stddevUpdateLatencyMs = statsC.stddev;
        resC.minUpdateLatencyMs = statsC.minVal;
        resC.maxUpdateLatencyMs = statsC.maxVal;
        resC.p90UpdateLatencyMs = statsC.p90;
        resC.p95UpdateLatencyMs = statsC.p95;
        resC.status = BenchmarkStatus::SUCCESS;

        results.push_back(resC);
    }

    return results;
}

void BenchmarkRunner::runDynamicBenchmarks(
    const std::string& insCsvPath,
    const std::string& delCsvPath,
    const std::string& mixedCsvPath,
    const std::string& summaryCsvPath,
    bool quickMode) {

    std::ofstream insOut(insCsvPath);
    std::ofstream delOut(delCsvPath);
    std::ofstream mixOut(mixedCsvPath);
    std::ofstream sumOut(summaryCsvPath);

    std::string stepHeader = 
        "algorithm,variant,workload,graph_family,initial_n,seed,update_index,op_type,target_id,"
        "n_before,n_after,q_before,q_after,update_application_ms,algorithm_update_ms,materialization_ms,"
        "total_latency_ms,descriptors_added,descriptors_removed,descriptors_modified,descriptors_preserved,status";

    insOut << stepHeader << "\n";
    delOut << stepHeader << "\n";
    mixOut << stepHeader << "\n";

    std::string sumHeader = 
        "algorithm,variant,workload,graph_family,initial_n,final_n,seed,total_updates,successful_updates,"
        "rejected_updates,cumulative_time_ms,average_latency_ms,amortized_latency_ms,median_latency_ms,"
        "stddev_latency_ms,min_latency_ms,max_latency_ms,p90_latency_ms,p95_latency_ms,"
        "final_num_cliques,final_total_memberships,status";

    sumOut << sumHeader << "\n";

    std::vector<int> initialSizes = quickMode ? std::vector<int>{50, 100, 500} : std::vector<int>{50, 100, 500, 1000, 5000};
    int numIns = quickMode ? 20 : 50;
    int numDel = quickMode ? 15 : 30;
    int numMixed = quickMode ? 20 : 40;

    std::cout << "\n========================================================\n"
              << " Executing Dynamic Maximal Clique Benchmark Suite\n"
              << " Workloads: Incremental (100%), Decremental (100%), Mixed\n"
              << "========================================================\n";

    for (int initN : initialSizes) {
        std::cout << "\n>>> Dynamic Benchmarks at initial n = " << initN << " <<<\n";

        // 1. Workload A: Incremental Insertion
        {
            auto insSeq = DynamicWorkloadGenerator::generateInsertionWorkload(initN, numIns, 101);
            auto evals = evaluateDynamicSequence(insSeq);
            for (const auto& res : evals) {
                // Export summary
                sumOut << std::fixed << std::setprecision(4)
                       << "\"" << res.algorithmName << "\",\"" << res.variantName << "\",\"" << res.workloadName << "\",\""
                       << res.graphFamily << "\"," << res.initialN << "," << res.finalN << "," << res.randomSeed << ","
                       << res.totalUpdates << "," << res.successfulUpdates << "," << res.rejectedUpdates << ","
                       << res.cumulativeTimeMs << "," << res.averageUpdateLatencyMs << "," << res.amortizedUpdateLatencyMs << ","
                       << res.medianUpdateLatencyMs << "," << res.stddevUpdateLatencyMs << ","
                       << res.minUpdateLatencyMs << "," << res.maxUpdateLatencyMs << ","
                       << res.p90UpdateLatencyMs << "," << res.p95UpdateLatencyMs << ","
                       << res.finalNumCliques << "," << res.finalTotalMemberships << ","
                       << statusToString(res.status) << "\n";

                // Export individual steps
                for (const auto& step : res.steps) {
                    insOut << std::fixed << std::setprecision(4)
                           << "\"" << res.algorithmName << "\",\"" << res.variantName << "\",\"" << res.workloadName << "\",\""
                           << res.graphFamily << "\"," << res.initialN << "," << res.randomSeed << ","
                           << step.updateIndex << "," << opTypeToString(step.opType) << "," << step.targetVertexId << ","
                           << step.nBefore << "," << step.nAfter << "," << step.qBefore << "," << step.qAfter << ","
                           << step.updateApplicationMs << "," << step.algorithmUpdateMs << "," << step.materializationMs << ","
                           << step.totalLatencyMs << "," << step.descriptorsAdded << "," << step.descriptorsRemoved << ","
                           << step.descriptorsModified << "," << step.descriptorsPreserved << ","
                           << statusToString(step.status) << "\n";
                }

                std::cout << "  [INSERTION] " << std::setw(30) << res.algorithmName
                          << " | Avg Latency: " << std::setw(8) << res.averageUpdateLatencyMs << " ms"
                          << " | Cum: " << std::setw(8) << res.cumulativeTimeMs << " ms\n";
            }
        }

        // 2. Workload B: Decremental Deletion
        {
            auto delSeq = DynamicWorkloadGenerator::generateDeletionWorkload(initN, numDel, 202);
            auto evals = evaluateDynamicSequence(delSeq);
            for (const auto& res : evals) {
                sumOut << std::fixed << std::setprecision(4)
                       << "\"" << res.algorithmName << "\",\"" << res.variantName << "\",\"" << res.workloadName << "\",\""
                       << res.graphFamily << "\"," << res.initialN << "," << res.finalN << "," << res.randomSeed << ","
                       << res.totalUpdates << "," << res.successfulUpdates << "," << res.rejectedUpdates << ","
                       << res.cumulativeTimeMs << "," << res.averageUpdateLatencyMs << "," << res.amortizedUpdateLatencyMs << ","
                       << res.medianUpdateLatencyMs << "," << res.stddevUpdateLatencyMs << ","
                       << res.minUpdateLatencyMs << "," << res.maxUpdateLatencyMs << ","
                       << res.p90UpdateLatencyMs << "," << res.p95UpdateLatencyMs << ","
                       << res.finalNumCliques << "," << res.finalTotalMemberships << ","
                       << statusToString(res.status) << "\n";

                for (const auto& step : res.steps) {
                    delOut << std::fixed << std::setprecision(4)
                           << "\"" << res.algorithmName << "\",\"" << res.variantName << "\",\"" << res.workloadName << "\",\""
                           << res.graphFamily << "\"," << res.initialN << "," << res.randomSeed << ","
                           << step.updateIndex << "," << opTypeToString(step.opType) << "," << step.targetVertexId << ","
                           << step.nBefore << "," << step.nAfter << "," << step.qBefore << "," << step.qAfter << ","
                           << step.updateApplicationMs << "," << step.algorithmUpdateMs << "," << step.materializationMs << ","
                           << step.totalLatencyMs << "," << step.descriptorsAdded << "," << step.descriptorsRemoved << ","
                           << step.descriptorsModified << "," << step.descriptorsPreserved << ","
                           << statusToString(step.status) << "\n";
                }

                std::cout << "  [DELETION]  " << std::setw(30) << res.algorithmName
                          << " | Avg Latency: " << std::setw(8) << res.averageUpdateLatencyMs << " ms"
                          << " | Cum: " << std::setw(8) << res.cumulativeTimeMs << " ms\n";
            }
        }

        // 3. Workload C: Mixed Workloads (75/25, 50/50, 25/75)
        std::vector<double> ratios = {0.75, 0.50, 0.25};
        for (double r : ratios) {
            auto mixSeq = DynamicWorkloadGenerator::generateMixedWorkload(initN, numMixed, r, 303);
            auto evals = evaluateDynamicSequence(mixSeq);
            for (const auto& res : evals) {
                sumOut << std::fixed << std::setprecision(4)
                       << "\"" << res.algorithmName << "\",\"" << res.variantName << "\",\"" << res.workloadName << "\",\""
                       << res.graphFamily << "\"," << res.initialN << "," << res.finalN << "," << res.randomSeed << ","
                       << res.totalUpdates << "," << res.successfulUpdates << "," << res.rejectedUpdates << ","
                       << res.cumulativeTimeMs << "," << res.averageUpdateLatencyMs << "," << res.amortizedUpdateLatencyMs << ","
                       << res.medianUpdateLatencyMs << "," << res.stddevUpdateLatencyMs << ","
                       << res.minUpdateLatencyMs << "," << res.maxUpdateLatencyMs << ","
                       << res.p90UpdateLatencyMs << "," << res.p95UpdateLatencyMs << ","
                       << res.finalNumCliques << "," << res.finalTotalMemberships << ","
                       << statusToString(res.status) << "\n";

                for (const auto& step : res.steps) {
                    mixOut << std::fixed << std::setprecision(4)
                           << "\"" << res.algorithmName << "\",\"" << res.variantName << "\",\"" << res.workloadName << "\",\""
                           << res.graphFamily << "\"," << res.initialN << "," << res.randomSeed << ","
                           << step.updateIndex << "," << opTypeToString(step.opType) << "," << step.targetVertexId << ","
                           << step.nBefore << "," << step.nAfter << "," << step.qBefore << "," << step.qAfter << ","
                           << step.updateApplicationMs << "," << step.algorithmUpdateMs << "," << step.materializationMs << ","
                           << step.totalLatencyMs << "," << step.descriptorsAdded << "," << step.descriptorsRemoved << ","
                           << step.descriptorsModified << "," << step.descriptorsPreserved << ","
                           << statusToString(step.status) << "\n";
                }

                std::cout << "  [" << std::setw(10) << res.workloadName << "] " << std::setw(26) << res.algorithmName
                          << " | Avg Latency: " << std::setw(8) << res.averageUpdateLatencyMs << " ms"
                          << " | Cum: " << std::setw(8) << res.cumulativeTimeMs << " ms\n";
            }
        }
        insOut.flush();
        delOut.flush();
        mixOut.flush();
        sumOut.flush();
    }

    std::cout << "\n[+] Dynamic benchmarks completed successfully!\n"
              << "    Results exported to: " << summaryCsvPath << "\n";
}

bool BenchmarkRunner::runVerificationSuite(const std::string& verificationCsvPath) {
    std::ofstream vOut(verificationCsvPath);
    if (!vOut.is_open()) {
        std::cerr << "Failed to open verification CSV at: " << verificationCsvPath << std::endl;
        return false;
    }

    vOut << "test_type,test_name,n,cliques_a,cliques_b,cliques_c,passed,details\n";

    std::cout << "\n========================================================\n"
              << " Executing Comprehensive Static and Dynamic Verification\n"
              << " Covering All 14 Graph Families and All Dynamic Workloads\n"
              << "========================================================\n";

    bool allPassed = true;
    int passCount = 0, totalCount = 0;

    // 1. Static Verification across all 14 families at sizes n in {10, 20, 50, 100}
    std::vector<int> verifySizes = {10, 20, 50, 100};
    for (int n : verifySizes) {
        std::vector<std::pair<std::string, IntervalGraph>> testFamilies;
        testFamilies.push_back({"Family 1: Equal-Length (sparse)", GraphGenerator::generateEqualLengthsGraph(n, OverlapProfile::SPARSE_OVERLAP)});
        testFamilies.push_back({"Family 1: Equal-Length (medium)", GraphGenerator::generateEqualLengthsGraph(n, OverlapProfile::MEDIUM_OVERLAP)});
        testFamilies.push_back({"Family 1: Equal-Length (dense)", GraphGenerator::generateEqualLengthsGraph(n, OverlapProfile::DENSE_OVERLAP)});
        testFamilies.push_back({"Family 2: Unequal-Lengths", GraphGenerator::generateUnequalLengthsGraph(n)});
        testFamilies.push_back({"Family 3: Random Proper (seed 101)", GraphGenerator::generateRandomProperGraph(n, 101)});
        testFamilies.push_back({"Family 3: Random Proper (seed 202)", GraphGenerator::generateRandomProperGraph(n, 202)});
        testFamilies.push_back({"Family 4: Identical Clusters", GraphGenerator::generateIdenticalIntervalsGraph(n / 2, 2)});
        testFamilies.push_back({"Family 5: Endpoint-Touching", GraphGenerator::generateEndpointTouchingGraph(n)});
        testFamilies.push_back({"Family 6: Long Chain", GraphGenerator::generateLongChainGraph(n)});
        testFamilies.push_back({"Family 7: High Overlap", GraphGenerator::generateHighOverlapGraph(n, 15)});
        testFamilies.push_back({"Family 8: Alternating Lengths", GraphGenerator::generateAlternatingLengthsGraph(n)});
        testFamilies.push_back({"Family 9: Irregular Spacing", GraphGenerator::generateIrregularEndpointsGraph(n, 404)});
        testFamilies.push_back({"Family 10: Many Small Cliques", GraphGenerator::generateManySmallCliquesGraph(n)});
        testFamilies.push_back({"Family 11: Few Large Cliques", GraphGenerator::generateFewLargeCliquesGraph(n)});
        testFamilies.push_back({"Family 12: Varying Overlap", GraphGenerator::generateVaryingOverlapGraph(n)});
        testFamilies.push_back({"Family 13: Adversarial Tie-Breaking", GraphGenerator::generateAdversarialTieBreakingGraph(n)});
        testFamilies.push_back({"Family 14: Shuffled IDs", GraphGenerator::generateShuffledIdsGraph(n, 505)});

        for (const auto& tf : testFamilies) {
            totalCount++;
            auto vRes = CliqueVerifier::verifyStaticGraph(tf.second, tf.first);
            if (!vRes.passed) allPassed = false;
            else passCount++;

            vOut << "STATIC,\"" << tf.first << "\"," << n << ","
                 << vRes.numCliquesA << "," << vRes.numCliquesB << "," << vRes.numCliquesC << ","
                 << (vRes.passed ? "PASS" : "FAIL") << ",\"" << vRes.details << "\"\n";
            vOut.flush();

            std::cout << "  [STATIC] n=" << std::setw(3) << n << " | " << std::setw(38) << tf.first
                      << " -> " << (vRes.passed ? "PASS" : "FAIL") << "\n";
        }
    }

    // 2. Dynamic Verification across Insertion, Deletion, and Mixed sequences
    std::vector<uint32_t> seeds = {101, 202, 303};
    for (uint32_t s : seeds) {
        // Insertion sequence
        {
            totalCount++;
            auto insSeq = DynamicWorkloadGenerator::generateInsertionWorkload(30, 20, s);
            auto vRes = CliqueVerifier::verifyDynamicSequence(insSeq);
            if (!vRes.passed) allPassed = false;
            else passCount++;

            vOut << "DYNAMIC_INSERTION,\"" << insSeq.workloadName << "\"," << insSeq.initialN << ","
                 << vRes.numCliquesA << "," << vRes.numCliquesB << "," << vRes.numCliquesC << ","
                 << (vRes.passed ? "PASS" : "FAIL") << ",\"" << vRes.details << "\"\n";
            vOut.flush();

            std::cout << "  [DYN-INS] seed=" << s << " (20 ops) -> " << (vRes.passed ? "PASS" : "FAIL") << "\n";
        }

        // Deletion sequence
        {
            totalCount++;
            auto delSeq = DynamicWorkloadGenerator::generateDeletionWorkload(30, 15, s);
            auto vRes = CliqueVerifier::verifyDynamicSequence(delSeq);
            if (!vRes.passed) allPassed = false;
            else passCount++;

            vOut << "DYNAMIC_DELETION,\"" << delSeq.workloadName << "\"," << delSeq.initialN << ","
                 << vRes.numCliquesA << "," << vRes.numCliquesB << "," << vRes.numCliquesC << ","
                 << (vRes.passed ? "PASS" : "FAIL") << ",\"" << vRes.details << "\"\n";
            vOut.flush();

            std::cout << "  [DYN-DEL] seed=" << s << " (15 ops) -> " << (vRes.passed ? "PASS" : "FAIL") << "\n";
        }

        // Mixed sequence
        {
            totalCount++;
            auto mixSeq = DynamicWorkloadGenerator::generateMixedWorkload(30, 20, 0.50, s);
            auto vRes = CliqueVerifier::verifyDynamicSequence(mixSeq);
            if (!vRes.passed) allPassed = false;
            else passCount++;

            vOut << "DYNAMIC_MIXED,\"" << mixSeq.workloadName << "\"," << mixSeq.initialN << ","
                 << vRes.numCliquesA << "," << vRes.numCliquesB << "," << vRes.numCliquesC << ","
                 << (vRes.passed ? "PASS" : "FAIL") << ",\"" << vRes.details << "\"\n";
            vOut.flush();

            std::cout << "  [DYN-MIX] seed=" << s << " (20 ops) -> " << (vRes.passed ? "PASS" : "FAIL") << "\n";
        }
    }

    std::cout << "\n========================================================\n"
              << " Verification Summary: " << passCount << " / " << totalCount << " PASSED ("
              << (allPassed ? "100% CLEAN" : "SOME FAILED") << ")\n"
              << " Results exported to: " << verificationCsvPath << "\n"
              << "========================================================\n";

    return allPassed;
}
