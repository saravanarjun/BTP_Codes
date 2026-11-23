#ifndef STANDARD_INTERVAL_CLIQUES_H
#define STANDARD_INTERVAL_CLIQUES_H

#include "IntervalGraph.h"
#include <vector>
#include <list>

enum class SweepEventType {
    START = 0, // Process START before END at equal coordinates
    END = 1
};

struct SweepEndpointEvent {
    double coord;
    SweepEventType type;
    int intervalId;
    int sortedIndex;

    bool operator<(const SweepEndpointEvent& other) const {
        if (coord != other.coord) {
            return coord < other.coord;
        }
        if (type != other.type) {
            return type < other.type; // START (0) precedes END (1)
        }
        return intervalId < other.intervalId;
    }
};

// Compact descriptor produced by Algorithm C sweep-line
struct SweepCliqueDescriptor {
    int firstSortedIndex;
    int lastSortedIndex;
    int firstId;
    int lastId;
    int size;
    double endCoord;
};

class StandardIntervalCliques {
public:
    // Build and sort all 2n endpoint events from the interval graph
    static std::vector<SweepEndpointEvent> buildSortedEvents(const IntervalGraph& graph);

    // Algorithm C Core: Sweep line generating compact clique descriptors
    // Complexity: O(n) after events are sorted
    static std::vector<SweepCliqueDescriptor> sweepCore(
        const std::vector<SweepEndpointEvent>& events,
        int numVertices);

    // Algorithm C Materialization: Expand descriptors into explicit vector<vector<int>>
    // Complexity: O(S) where S = sum |C_j|
    static std::vector<std::vector<int>> materializeCliques(
        const std::vector<SweepCliqueDescriptor>& descriptors,
        const IntervalGraph& graph);

    // Direct end-to-end execution: sorts events, sweeps, and materializes
    static std::vector<std::vector<int>> enumerateMaximalCliques(const IntervalGraph& graph);
};

#endif // STANDARD_INTERVAL_CLIQUES_H
