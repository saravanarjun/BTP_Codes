#ifndef CLIQUE_TYPES_H
#define CLIQUE_TYPES_H

#include "IntervalGraph.h"
#include <vector>
#include <string>
#include <cstdint>

// Status of an algorithm execution or dynamic step
enum class BenchmarkStatus {
    SUCCESS,
    TIMEOUT,
    MEMORY_LIMIT,
    INVALID_INPUT,
    CORRECTNESS_FAILURE,
    UNSUPPORTED_UPDATE,
    SKIPPED_BY_POLICY
};

inline std::string statusToString(BenchmarkStatus s) {
    switch (s) {
        case BenchmarkStatus::SUCCESS: return "SUCCESS";
        case BenchmarkStatus::TIMEOUT: return "TIMEOUT";
        case BenchmarkStatus::MEMORY_LIMIT: return "MEMORY_LIMIT";
        case BenchmarkStatus::INVALID_INPUT: return "INVALID_INPUT";
        case BenchmarkStatus::CORRECTNESS_FAILURE: return "CORRECTNESS_FAILURE";
        case BenchmarkStatus::UNSUPPORTED_UPDATE: return "UNSUPPORTED_UPDATE";
        case BenchmarkStatus::SKIPPED_BY_POLICY: return "SKIPPED_BY_POLICY";
        default: return "UNKNOWN";
    }
}

// Compact maximal-clique descriptor (Lemma 3.15)
struct CompactDescriptor {
    int parentId;
    int rightmostChildId;
    double leftBound;    // l_p
    double rightBound;   // l_c

    bool operator==(const CompactDescriptor& o) const {
        return parentId == o.parentId && rightmostChildId == o.rightmostChildId;
    }
};

// Static algorithm execution result
struct StaticAlgorithmResult {
    std::string algorithmName;
    std::string variantName;
    std::string graphFamily;
    int n = 0;
    int m = 0;
    double averageDegree = 0.0;
    int maxDegree = 0;
    int q = 0;                  // Number of maximal cliques
    int64_t totalMemberships = 0; // S = sum |C_j|
    double averageCliqueSize = 0.0;
    int maxCliqueSize = 0;

    // Timing metrics (milliseconds)
    double preprocessingMs = 0.0;
    double coreMs = 0.0;
    double materializationMs = 0.0;
    double postprocessingTotalMs = 0.0;
    double endToEndSumMs = 0.0;
    double directEndToEndMs = 0.0;

    // Repetitions and jitter
    double minDirectEndToEndMs = 0.0;
    double maxDirectEndToEndMs = 0.0;
    int repetitions = 0;

    // Storage estimates
    int64_t estimatedDescriptorBytes = 0;
    int64_t estimatedExplicitBytes = 0;

    BenchmarkStatus status = BenchmarkStatus::SUCCESS;
    std::string errorMessage;

    // Output representation
    std::vector<CompactDescriptor> descriptors;
    std::vector<std::vector<int>> materializedCliques;
};

// Dynamic operation type
enum class DynamicOpType {
    INSERTION,
    DELETION
};

inline std::string opTypeToString(DynamicOpType op) {
    return op == DynamicOpType::INSERTION ? "INSERTION" : "DELETION";
}

// Single dynamic update step telemetry
struct DynamicUpdateStepTelemetry {
    int updateIndex = 0;
    DynamicOpType opType = DynamicOpType::INSERTION;
    int targetVertexId = -1;
    Interval targetInterval{ -1, 0.0, 0.0 };

    int nBefore = 0;
    int nAfter = 0;
    int mBefore = 0;
    int mAfter = 0;
    int qBefore = 0;
    int qAfter = 0;

    // Per-algorithm step metrics
    double updateApplicationMs = 0.0;
    double algorithmUpdateMs = 0.0;
    double materializationMs = 0.0;
    double validationMs = 0.0;
    double totalLatencyMs = 0.0;

    // Telemetry on descriptor delta (set-theoretic ground truth)
    int descriptorsAdded = 0;
    int descriptorsRemoved = 0;
    int descriptorsModified = 0;
    int descriptorsPreserved = 0;
    int membershipsChanged = 0;

    BenchmarkStatus status = BenchmarkStatus::SUCCESS;
    std::string diagnostic;
};

// Complete dynamic sequence execution result
struct DynamicSequenceBenchmarkResult {
    std::string algorithmName;
    std::string variantName;
    std::string workloadName;
    std::string graphFamily;
    int initialN = 0;
    int finalN = 0;
    int totalUpdates = 0;
    int successfulUpdates = 0;
    int rejectedUpdates = 0;
    uint32_t randomSeed = 0;

    // Timing summaries and statistical metrics across the sequence
    double cumulativeTimeMs = 0.0;
    double averageUpdateLatencyMs = 0.0;
    double amortizedUpdateLatencyMs = 0.0;
    double medianUpdateLatencyMs = 0.0;
    double stddevUpdateLatencyMs = 0.0;
    double minUpdateLatencyMs = 0.0;
    double maxUpdateLatencyMs = 0.0;
    double p90UpdateLatencyMs = 0.0;
    double p95UpdateLatencyMs = 0.0;

    // Output descriptors count at end
    int finalNumCliques = 0;
    int64_t finalTotalMemberships = 0;

    BenchmarkStatus status = BenchmarkStatus::SUCCESS;
    std::vector<DynamicUpdateStepTelemetry> steps;
};

#endif // CLIQUE_TYPES_H
