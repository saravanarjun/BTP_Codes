#include "IntervalGraph.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cassert>

IntervalGraph::IntervalGraph() {}

const Interval& IntervalGraph::getIntervalBySortedIndex(int idx) const {
    if (idx < 0 || idx >= static_cast<int>(sortedIntervals.size())) {
        throw std::out_of_range("Sorted interval index out of range: " + std::to_string(idx));
    }
    return sortedIntervals[idx];
}

const Interval& IntervalGraph::getIntervalById(int id) const {
    auto it = idToSortedIndex.find(id);
    if (it == idToSortedIndex.end()) {
        throw std::out_of_range("Vertex ID not found: " + std::to_string(id));
    }
    return sortedIntervals[it->second];
}

int IntervalGraph::getSortedIndexById(int id) const {
    auto it = idToSortedIndex.find(id);
    if (it == idToSortedIndex.end()) {
        throw std::out_of_range("Vertex ID not found: " + std::to_string(id));
    }
    return it->second;
}

bool IntervalGraph::hasVertexId(int id) const {
    return idToSortedIndex.find(id) != idToSortedIndex.end();
}

ValidationResult IntervalGraph::validateEndpoints(const std::vector<Interval>& intervals) {
    if (intervals.empty()) {
        return {false, "Input interval set is empty."};
    }
    for (size_t i = 0; i < intervals.size(); ++i) {
        const auto& iv = intervals[i];
        if (iv.left > iv.right) {
            std::ostringstream oss;
            oss << "Invalid interval for vertex ID " << iv.id 
                << ": left endpoint (" << iv.left 
                << ") exceeds right endpoint (" << iv.right << ").";
            return {false, oss.str()};
        }
        if (std::isnan(iv.left) || std::isnan(iv.right) || std::isinf(iv.left) || std::isinf(iv.right)) {
            std::ostringstream oss;
            oss << "Invalid interval for vertex ID " << iv.id 
                << ": endpoints must be finite real numbers.";
            return {false, oss.str()};
        }
    }
    return {true, "Endpoints valid."};
}

ValidationResult IntervalGraph::validateProperness(const std::vector<Interval>& sorted) {
    int n = static_cast<int>(sorted.size());
    if (n <= 1) {
        return {true, "Trivially proper."};
    }

    // Check adjacent elements in sorted order:
    // Sorted by left endpoint, then right endpoint.
    // If sorted[i] and sorted[i+1] have:
    // 1. sorted[i].left == sorted[i+1].left and sorted[i].right < sorted[i+1].right:
    //    sorted[i] is strictly contained in sorted[i+1].
    // 2. sorted[i].right > sorted[i+1].right:
    //    since sorted[i].left <= sorted[i+1].left, sorted[i+1] is contained in sorted[i].
    for (int i = 0; i < n - 1; ++i) {
        const auto& a = sorted[i];
        const auto& b = sorted[i + 1];

        if (a.left == b.left && a.right < b.right) {
            std::ostringstream oss;
            oss << "Properness violation: Vertex " << a.id << " [" << a.left << ", " << a.right << "]"
                << " is properly contained in Vertex " << b.id << " [" << b.left << ", " << b.right << "].";
            return {false, oss.str()};
        }

        if (a.right > b.right) {
            std::ostringstream oss;
            oss << "Properness violation: Vertex " << b.id << " [" << b.left << ", " << b.right << "]"
                << " is properly contained in Vertex " << a.id << " [" << a.left << ", " << a.right << "].";
            return {false, oss.str()};
        }
    }

    return {true, "Proper interval graph condition satisfied."};
}

ValidationResult IntervalGraph::validateConnectedness(const std::vector<Interval>& sorted) {
    int n = static_cast<int>(sorted.size());
    if (n <= 1) {
        return {true, "Trivially connected."};
    }

    // Sweep-line connectedness check in O(n):
    // Maintain max right endpoint seen so far among sorted[0 ... i-1].
    // If sorted[i].left > maxRight, there is a gap, meaning no interval in the prefix
    // can intersect sorted[i] or any subsequent interval.
    double maxRight = sorted[0].right;
    for (int i = 1; i < n; ++i) {
        if (sorted[i].left > maxRight) {
            std::ostringstream oss;
            oss << "Connectedness violation: Component split detected between prefix vertices and vertex "
                << sorted[i].id << " [" << sorted[i].left << ", " << sorted[i].right << "]. "
                << "Max right endpoint of previous component was " << maxRight 
                << ", which is strictly less than left endpoint " << sorted[i].left << ".";
            return {false, oss.str()};
        }
        if (sorted[i].right > maxRight) {
            maxRight = sorted[i].right;
        }
    }

    return {true, "Graph is connected."};
}

bool IntervalGraph::initFromIntervals(const std::vector<Interval>& rawIntervals, std::string& errorMessage) {
    // 1. Validate endpoint ordering
    auto endRes = validateEndpoints(rawIntervals);
    if (!endRes.valid) {
        errorMessage = endRes.message;
        return false;
    }

    // Check for unique vertex IDs
    std::unordered_map<int, int> idCheck;
    for (const auto& iv : rawIntervals) {
        if (idCheck.find(iv.id) != idCheck.end()) {
            errorMessage = "Duplicate vertex ID detected: " + std::to_string(iv.id);
            return false;
        }
        idCheck[iv.id] = 1;
    }

    // 2. Sort intervals using Chapter 2 left-to-right ordering
    std::vector<Interval> sorted = rawIntervals;
    std::sort(sorted.begin(), sorted.end());

    // 3. Validate properness
    auto propRes = validateProperness(sorted);
    if (!propRes.valid) {
        errorMessage = propRes.message;
        return false;
    }

    // 4. Validate connectedness
    auto connRes = validateConnectedness(sorted);
    if (!connRes.valid) {
        errorMessage = connRes.message;
        return false;
    }

    // Commit sorted intervals and build ID-to-index map
    sortedIntervals = std::move(sorted);
    idToSortedIndex.clear();
    for (int i = 0; i < static_cast<int>(sortedIntervals.size()); ++i) {
        idToSortedIndex[sortedIntervals[i].id] = i;
    }

    errorMessage.clear();
    return true;
}

bool IntervalGraph::loadFromFile(const std::string& filepath, std::string& errorMessage) {
    std::ifstream fin(filepath);
    if (!fin.is_open()) {
        errorMessage = "Failed to open file: " + filepath;
        return false;
    }

    int n = 0;
    if (!(fin >> n)) {
        errorMessage = "Malformed input: Unable to read vertex count n.";
        return false;
    }

    if (n <= 0) {
        errorMessage = "Invalid vertex count n: " + std::to_string(n) + " (must be >= 1).";
        return false;
    }

    std::vector<Interval> raw;
    raw.reserve(n);

    for (int i = 0; i < n; ++i) {
        int id;
        double l, r;
        if (!(fin >> id >> l >> r)) {
            errorMessage = "Malformed input: Failed to read interval entry at line " + std::to_string(i + 2);
            return false;
        }
        raw.push_back({id, l, r});
    }

    return initFromIntervals(raw, errorMessage);
}

bool IntervalGraph::isAdjacent(int uId, int vId) const {
    const Interval& u = getIntervalById(uId);
    const Interval& v = getIntervalById(vId);
    return (u.left <= v.right && v.left <= u.right);
}

bool IntervalGraph::isAdjacentBySortedIndex(int uIdx, int vIdx) const {
    const Interval& u = getIntervalBySortedIndex(uIdx);
    const Interval& v = getIntervalBySortedIndex(vIdx);
    return (u.left <= v.right && v.left <= u.right);
}

std::vector<int> IntervalGraph::getNeighbours(int vId) const {
    std::vector<int> nbrs;
    const Interval& target = getIntervalById(vId);
    for (const auto& iv : sortedIntervals) {
        if (iv.id != vId && target.left <= iv.right && iv.left <= target.right) {
            nbrs.push_back(iv.id);
        }
    }
    return nbrs;
}

std::vector<int> IntervalGraph::getClosedNeighbours(int vId) const {
    std::vector<int> closedNbrs;
    const Interval& target = getIntervalById(vId);
    for (const auto& iv : sortedIntervals) {
        if (target.left <= iv.right && iv.left <= target.right) {
            closedNbrs.push_back(iv.id);
        }
    }
    return closedNbrs;
}

std::vector<std::vector<bool>> IntervalGraph::getAdjacencyMatrix() const {
    int n = getNumVertices();
    std::vector<std::vector<bool>> adj(n, std::vector<bool>(n, false));
    for (int i = 0; i < n; ++i) {
        adj[i][i] = true; // self-loop for closed or false for open
        for (int j = i + 1; j < n; ++j) {
            bool conn = (sortedIntervals[i].left <= sortedIntervals[j].right &&
                         sortedIntervals[j].left <= sortedIntervals[i].right);
            adj[i][j] = conn;
            adj[j][i] = conn;
        }
    }
    return adj;
}

bool IntervalGraph::writeSortedIntervals(const std::string& filepath) const {
    std::ofstream fout(filepath);
    if (!fout.is_open()) {
        return false;
    }

    fout << "# Sorted Proper Intervals (Chapter 2 Ordering)\n";
    fout << "# Sorted_Pos\tVertex_ID\tLeft\tRight\tLength\n";
    for (int i = 0; i < static_cast<int>(sortedIntervals.size()); ++i) {
        const auto& iv = sortedIntervals[i];
        fout << i << "\t" << iv.id << "\t" 
             << std::fixed << std::setprecision(4) << iv.left << "\t" 
             << iv.right << "\t" << (iv.right - iv.left) << "\n";
    }
    return true;
}

