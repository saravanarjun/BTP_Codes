#ifndef DYNAMIC_WORKLOAD_GENERATOR_H
#define DYNAMIC_WORKLOAD_GENERATOR_H

#include "clique_types.h"
#include "IntervalGraph.h"
#include <vector>
#include <string>
#include <cstdint>

struct DynamicOperation {
    DynamicOpType type;
    int targetId;
    Interval newInterval; // Valid if type == INSERTION
};

struct DynamicWorkloadSequence {
    std::string workloadName;
    std::string familyName;
    int initialN;
    uint32_t seed;
    double insertionRatio; // e.g. 1.0 for 100% ins, 0.0 for 100% del, 0.5 for 50/50
    IntervalGraph initialGraph;
    std::vector<DynamicOperation> operations;
};

class DynamicWorkloadGenerator {
public:
    // Generate Workload A: Incremental leaf insertions (100% insertions)
    static DynamicWorkloadSequence generateInsertionWorkload(
        int initialN,
        int numInsertions,
        uint32_t seed,
        double step = 1.0,
        double width = 3.5);

    // Generate Workload B: Decremental vertex deletions (100% deletions)
    static DynamicWorkloadSequence generateDeletionWorkload(
        int initialN,
        int numDeletions,
        uint32_t seed,
        double step = 1.0,
        double width = 8.0);

    // Generate Workload C: Mixed workload with specific insertion ratio
    // e.g. ratio = 0.75 for 75% ins / 25% del
    static DynamicWorkloadSequence generateMixedWorkload(
        int initialN,
        int totalOps,
        double insertionRatio,
        uint32_t seed,
        double step = 1.0,
        double width = 8.0);

    // Overloads accepting pre-built initial graphs for multi-family dynamic evaluation
    static DynamicWorkloadSequence generateInsertionWorkloadFromGraph(
        const IntervalGraph& initialGraph,
        const std::string& familyName,
        int numInsertions,
        uint32_t seed,
        double step = 1.0,
        double width = 3.5);

    static DynamicWorkloadSequence generateDeletionWorkloadFromGraph(
        const IntervalGraph& initialGraph,
        const std::string& familyName,
        int numDeletions,
        uint32_t seed);

    static DynamicWorkloadSequence generateMixedWorkloadFromGraph(
        const IntervalGraph& initialGraph,
        const std::string& familyName,
        int totalOps,
        double insertionRatio,
        uint32_t seed,
        double step = 1.0,
        double width = 8.0);
};

#endif // DYNAMIC_WORKLOAD_GENERATOR_H
