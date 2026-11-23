#ifndef INTERVAL_GRAPH_H
#define INTERVAL_GRAPH_H

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

struct Interval {
    int id;
    double left;
    double right;

    // Strict weak ordering as defined in thesis Section 2.2, 2.5:
    // Sorted by left endpoint; ties broken by right endpoint;
    // deterministic final tie-breaker by original vertex id.
    bool operator<(const Interval& other) const {
        if (left != other.left) {
            return left < other.left;
        }
        if (right != other.right) {
            return right < other.right;
        }
        return id < other.id;
    }

    bool operator==(const Interval& other) const {
        return id == other.id && left == other.left && right == other.right;
    }

    bool operator!=(const Interval& other) const {
        return !(*this == other);
    }
};

struct ValidationResult {
    bool valid;
    std::string message;
};

class IntervalGraph {
public:
    IntervalGraph();

    // Load from input file format (first line: n, following lines: id left right)
    bool loadFromFile(const std::string& filepath, std::string& errorMessage);

    // Initialize directly from an existing vector of intervals
    bool initFromIntervals(const std::vector<Interval>& rawIntervals, std::string& errorMessage);

    // Validation routines
    static ValidationResult validateEndpoints(const std::vector<Interval>& intervals);
    static ValidationResult validateProperness(const std::vector<Interval>& sortedIntervals);
    static ValidationResult validateConnectedness(const std::vector<Interval>& sortedIntervals);

    // Accessors
    int getNumVertices() const { return static_cast<int>(sortedIntervals.size()); }
    const std::vector<Interval>& getSortedIntervals() const { return sortedIntervals; }
    const Interval& getIntervalBySortedIndex(int idx) const;
    const Interval& getIntervalById(int id) const;
    int getSortedIndexById(int id) const;
    bool hasVertexId(int id) const;

    // Adjacency and neighbourhoods based on interval intersection (Chapter 2, Eq 2.2 & Definition)
    // I_u and I_v intersect <=> l_u <= r_v and l_v <= r_u
    bool isAdjacent(int uId, int vId) const;
    bool isAdjacentBySortedIndex(int uIdx, int vIdx) const;
    std::vector<int> getNeighbours(int vId) const;
    std::vector<int> getClosedNeighbours(int vId) const;

    // Independent explicit adjacency representation for verification
    std::vector<std::vector<bool>> getAdjacencyMatrix() const;

    // Output export
    bool writeSortedIntervals(const std::string& filepath) const;

private:
    std::vector<Interval> sortedIntervals;
    std::unordered_map<int, int> idToSortedIndex;
};

#endif // INTERVAL_GRAPH_H

