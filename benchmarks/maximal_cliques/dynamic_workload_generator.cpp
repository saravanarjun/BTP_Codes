#include "dynamic_workload_generator.h"
#include "graph_generator.h"
#include "DistanceTree.h"
#include "dynamic_clique_structure.h"
#include <random>
#include <algorithm>
#include <sstream>

DynamicWorkloadSequence DynamicWorkloadGenerator::generateInsertionWorkload(
    int initialN,
    int numInsertions,
    uint32_t seed,
    double step,
    double width) {

    IntervalGraph initG = GraphGenerator::generateEqualLengthsGraph(initialN, step, width);
    return generateInsertionWorkloadFromGraph(initG, "equal-lengths-growth", numInsertions, seed, step, width);
}

DynamicWorkloadSequence DynamicWorkloadGenerator::generateInsertionWorkloadFromGraph(
    const IntervalGraph& initialGraph,
    const std::string& familyName,
    int numInsertions,
    uint32_t seed,
    double step,
    double width) {

    DynamicWorkloadSequence seq;
    seq.workloadName = "incremental-insertion";
    seq.familyName = familyName;
    seq.initialN = initialGraph.getNumVertices();
    seq.seed = seed;
    seq.insertionRatio = 1.0;
    seq.initialGraph = initialGraph;

    const auto& sorted = initialGraph.getSortedIntervals();
    int maxId = 0;
    double lastLeft = 0.0;
    double maxRight = 0.0;
    for (const auto& iv : sorted) {
        if (iv.id > maxId) maxId = iv.id;
        if (iv.left > lastLeft) lastLeft = iv.left;
        if (iv.right > maxRight) maxRight = iv.right;
    }

    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> widthJitter(-0.2, 0.2);

    seq.operations.reserve(numInsertions);
    int nextId = maxId + 1;

    for (int i = 0; i < numInsertions; ++i) {
        double curLeft = lastLeft + step;
        double curWidth = width + widthJitter(rng);
        if (curWidth < step) curWidth = step + 0.1;
        double curRight = curLeft + curWidth;

        DynamicOperation op;
        op.type = DynamicOpType::INSERTION;
        op.targetId = nextId;
        op.newInterval = Interval{nextId, curLeft, curRight};

        seq.operations.push_back(op);

        lastLeft = curLeft;
        nextId++;
    }

    return seq;
}

DynamicWorkloadSequence DynamicWorkloadGenerator::generateDeletionWorkload(
    int initialN,
    int numDeletions,
    uint32_t seed,
    double step,
    double width) {

    IntervalGraph initG = GraphGenerator::generateEqualLengthsGraph(initialN, step, width);
    return generateDeletionWorkloadFromGraph(initG, "medium-overlap-deletion", numDeletions, seed);
}

DynamicWorkloadSequence DynamicWorkloadGenerator::generateDeletionWorkloadFromGraph(
    const IntervalGraph& initialGraph,
    const std::string& familyName,
    int numDeletions,
    uint32_t seed) {

    DynamicWorkloadSequence seq;
    seq.workloadName = "decremental-deletion";
    seq.familyName = familyName;
    seq.initialN = initialGraph.getNumVertices();
    seq.seed = seed;
    seq.insertionRatio = 0.0;
    seq.initialGraph = initialGraph;

    DistanceTree simTree(seq.initialGraph);
    simTree.build();

    DynamicTheoremCliques sim;
    sim.initialize(seq.initialGraph, simTree);

    std::mt19937 rng(seed);
    seq.operations.reserve(numDeletions);

    for (int stepIdx = 0; stepIdx < numDeletions; ++stepIdx) {
        if (sim.getNumVertices() <= 2) break;

        auto curIntervals = sim.getCurrentIntervals();
        bool found = false;
        for (const auto& iv : curIntervals) {
            int u = iv.id;
            if (u == curIntervals.front().id) continue;

            DynamicUpdateStepTelemetry trial = sim.deleteVertex(u);
            if (trial.status == BenchmarkStatus::SUCCESS) {
                DynamicOperation op;
                op.type = DynamicOpType::DELETION;
                op.targetId = u;
                seq.operations.push_back(op);
                found = true;
                break;
            }
        }
        if (!found) break;
    }

    return seq;
}

DynamicWorkloadSequence DynamicWorkloadGenerator::generateMixedWorkload(
    int initialN,
    int totalOps,
    double insertionRatio,
    uint32_t seed,
    double step,
    double width) {

    IntervalGraph initG = GraphGenerator::generateEqualLengthsGraph(initialN, step, width);
    return generateMixedWorkloadFromGraph(initG, "mixed-overlap", totalOps, insertionRatio, seed, step, width);
}

DynamicWorkloadSequence DynamicWorkloadGenerator::generateMixedWorkloadFromGraph(
    const IntervalGraph& initialGraph,
    const std::string& familyName,
    int totalOps,
    double insertionRatio,
    uint32_t seed,
    double step,
    double width) {

    DynamicWorkloadSequence seq;
    std::ostringstream oss;
    oss << "mixed-" << static_cast<int>(insertionRatio * 100) << "ins-" << static_cast<int>((1.0 - insertionRatio) * 100) << "del";
    seq.workloadName = oss.str();
    seq.familyName = familyName;
    seq.initialN = initialGraph.getNumVertices();
    seq.seed = seed;
    seq.insertionRatio = insertionRatio;
    seq.initialGraph = initialGraph;

    DistanceTree simTree(seq.initialGraph);
    simTree.build();

    DynamicTheoremCliques sim;
    sim.initialize(seq.initialGraph, simTree);

    const auto& sorted = initialGraph.getSortedIntervals();
    int maxId = 0;
    double maxLeft = 0.0;
    double maxRight = 0.0;
    for (const auto& iv : sorted) {
        if (iv.id > maxId) maxId = iv.id;
        if (iv.left > maxLeft) maxLeft = iv.left;
        if (iv.right > maxRight) maxRight = iv.right;
    }

    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> coin(0.0, 1.0);

    int nextId = (maxId + 1) * 2;
    seq.operations.reserve(totalOps);

    for (int opIdx = 0; opIdx < totalOps; ++opIdx) {
        bool doInsert = (coin(rng) < insertionRatio) || (sim.getNumVertices() <= 3);

        if (doInsert) {
            maxLeft += step;
            double curRight = maxLeft + width;
            Interval newIv{nextId++, maxLeft, curRight};

            DynamicUpdateStepTelemetry tel = sim.insertLeaf(newIv);
            if (tel.status == BenchmarkStatus::SUCCESS) {
                DynamicOperation op;
                op.type = DynamicOpType::INSERTION;
                op.targetId = newIv.id;
                op.newInterval = newIv;
                seq.operations.push_back(op);
            }
        } else {
            auto curIntervals = sim.getCurrentIntervals();
            bool deleted = false;
            for (const auto& iv : curIntervals) {
                int u = iv.id;
                if (u == curIntervals.front().id) continue;

                DynamicUpdateStepTelemetry trial = sim.deleteVertex(u);
                if (trial.status == BenchmarkStatus::SUCCESS) {
                    DynamicOperation op;
                    op.type = DynamicOpType::DELETION;
                    op.targetId = u;
                    seq.operations.push_back(op);
                    deleted = true;
                    break;
                }
            }
            if (!deleted) {
                maxLeft += step;
                double curRight = maxLeft + width;
                Interval newIv{nextId++, maxLeft, curRight};
                DynamicUpdateStepTelemetry tel = sim.insertLeaf(newIv);
                if (tel.status == BenchmarkStatus::SUCCESS) {
                    DynamicOperation op;
                    op.type = DynamicOpType::INSERTION;
                    op.targetId = newIv.id;
                    op.newInterval = newIv;
                    seq.operations.push_back(op);
                }
            }
        }
    }

    return seq;
}
