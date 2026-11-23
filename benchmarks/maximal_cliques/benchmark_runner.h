#ifndef BENCHMARK_RUNNER_H
#define BENCHMARK_RUNNER_H

#include "clique_types.h"
#include "graph_generator.h"
#include "dynamic_workload_generator.h"
#include <string>
#include <vector>

class BenchmarkRunner {
public:
    // Execute complete static benchmark suite across graph families and sizes
    static void runStaticBenchmarks(
        const std::string& resultsCsvPath,
        const std::string& summaryCsvPath,
        bool quickMode = false);

    // Execute complete dynamic benchmark suite (insertions, deletions, mixed)
    static void runDynamicBenchmarks(
        const std::string& insCsvPath,
        const std::string& delCsvPath,
        const std::string& mixedCsvPath,
        const std::string& summaryCsvPath,
        bool quickMode = false);

    // Execute comprehensive verification suite
    static bool runVerificationSuite(const std::string& verificationCsvPath);

private:
    // Helper to evaluate static algorithms on a single graph
    static std::vector<StaticAlgorithmResult> evaluateStaticGraph(
        const IntervalGraph& graph,
        const std::string& familyName,
        int targetN,
        int repetitions);

    // Helper to evaluate dynamic sequence across all 3 algorithms
    static std::vector<DynamicSequenceBenchmarkResult> evaluateDynamicSequence(
        const DynamicWorkloadSequence& sequence);
};

#endif // BENCHMARK_RUNNER_H
