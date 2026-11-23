#include "standard_interval_cliques.h"
#include <algorithm>

std::vector<SweepEndpointEvent> StandardIntervalCliques::buildSortedEvents(const IntervalGraph& graph) {
    int n = graph.getNumVertices();
    std::vector<SweepEndpointEvent> events;
    events.reserve(2 * n);

    const auto& sortedIvs = graph.getSortedIntervals();
    for (int i = 0; i < n; ++i) {
        const auto& iv = sortedIvs[i];
        SweepEndpointEvent evStart;
        evStart.coord = iv.left;
        evStart.type = SweepEventType::START;
        evStart.intervalId = iv.id;
        evStart.sortedIndex = i;
        events.push_back(evStart);

        SweepEndpointEvent evEnd;
        evEnd.coord = iv.right;
        evEnd.type = SweepEventType::END;
        evEnd.intervalId = iv.id;
        evEnd.sortedIndex = i;
        events.push_back(evEnd);
    }

    // Sort endpoints: coordinates ascending; starts before ends at ties; ID tie-breaker
    std::sort(events.begin(), events.end());
    return events;
}

std::vector<SweepCliqueDescriptor> StandardIntervalCliques::sweepCore(
    const std::vector<SweepEndpointEvent>& events,
    int numVertices) {

    std::vector<SweepCliqueDescriptor> descriptors;
    if (numVertices <= 0) return descriptors;

    // Maintain active set using doubly-linked list with O(1) random-access removal handles
    std::list<int> activeList;
    std::vector<std::list<int>::iterator> nodeIterators(numVertices);

    bool hasNewStart = false;

    for (const auto& ev : events) {
        if (ev.type == SweepEventType::START) {
            activeList.push_back(ev.sortedIndex);
            nodeIterators[ev.sortedIndex] = --activeList.end();
            hasNewStart = true;
        } else {
            // END event: interval is about to leave
            if (hasNewStart && !activeList.empty()) {
                SweepCliqueDescriptor desc;
                desc.firstSortedIndex = activeList.front();
                desc.lastSortedIndex = activeList.back();
                desc.firstId = -1; // Populated in materialization or if needed
                desc.lastId = -1;
                desc.size = static_cast<int>(activeList.size());
                desc.endCoord = ev.coord;
                descriptors.push_back(desc);

                hasNewStart = false; // Reset trigger
            }

            // Remove departing interval in O(1)
            activeList.erase(nodeIterators[ev.sortedIndex]);
        }
    }

    return descriptors;
}

std::vector<std::vector<int>> StandardIntervalCliques::materializeCliques(
    const std::vector<SweepCliqueDescriptor>& descriptors,
    const IntervalGraph& graph) {

    std::vector<std::vector<int>> cliques;
    cliques.reserve(descriptors.size());

    const auto& sortedIvs = graph.getSortedIntervals();

    for (const auto& desc : descriptors) {
        std::vector<int> clique;
        clique.reserve(desc.size);

        // In proper interval graphs, active intervals at any point form a contiguous sorted index range
        for (int idx = desc.firstSortedIndex; idx <= desc.lastSortedIndex; ++idx) {
            clique.push_back(sortedIvs[idx].id);
        }

        cliques.push_back(std::move(clique));
    }

    return cliques;
}

std::vector<std::vector<int>> StandardIntervalCliques::enumerateMaximalCliques(const IntervalGraph& graph) {
    int n = graph.getNumVertices();
    if (n == 0) return {};

    auto events = buildSortedEvents(graph);
    auto descriptors = sweepCore(events, n);
    return materializeCliques(descriptors, graph);
}
