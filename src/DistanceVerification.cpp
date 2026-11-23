#include "DistanceVerification.h"
#include <queue>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <random>
#include <cassert>

int DistanceVerification::evaluateEquation2_2Raw(const DistanceTree& tree, int uId, int vId) {
    const auto& uNode = tree.getNodeById(uId);
    const auto& vNode = tree.getNodeById(vId);

    if (uNode.posLevel <= vNode.posLevel) {
        std::ostringstream oss;
        oss << "Equation (2.2) precondition violated: requires pos_level(u) > pos_level(v). "
            << "Got pos_level(" << uId << ") = " << uNode.posLevel 
            << ", pos_level(" << vId << ") = " << vNode.posLevel;
        throw std::invalid_argument(oss.str());
    }

    int deltaDepth = uNode.depth - vNode.depth;
    int postorderBracket = (uNode.posPost > vNode.posPost) ? 1 : 0;
    return deltaDepth + postorderBracket;
}

int DistanceVerification::getDistanceFormula(const DistanceTree& tree, int xId, int yId) {
    if (xId == yId) {
        return 0;
    }

    const auto& xNode = tree.getNodeById(xId);
    const auto& yNode = tree.getNodeById(yId);

    // Apply Equation (2.2) symmetrically by identifying which vertex has larger pos_level
    if (xNode.posLevel > yNode.posLevel) {
        int deltaDepth = xNode.depth - yNode.depth;
        int bracket = (xNode.posPost > yNode.posPost) ? 1 : 0;
        return deltaDepth + bracket;
    } else {
        int deltaDepth = yNode.depth - xNode.depth;
        int bracket = (yNode.posPost > xNode.posPost) ? 1 : 0;
        return deltaDepth + bracket;
    }
}

int DistanceVerification::computeBfsDistance(const IntervalGraph& graph, int startId, int targetId) {
    if (startId == targetId) {
        return 0;
    }

    std::unordered_map<int, int> dist;
    std::queue<int> q;

    dist[startId] = 0;
    q.push(startId);

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        int d = dist[curr];
        if (curr == targetId) {
            return d;
        }

        std::vector<int> nbrs = graph.getNeighbours(curr);
        for (int nxt : nbrs) {
            if (dist.find(nxt) == dist.end()) {
                dist[nxt] = d + 1;
                if (nxt == targetId) {
                    return d + 1;
                }
                q.push(nxt);
            }
        }
    }

    return -1; // Unreachable (disconnected)
}

std::vector<int> DistanceVerification::computeSingleSourceBfs(const IntervalGraph& graph, int startId) {
    int n = graph.getNumVertices();
    std::vector<int> distances(n, -1);

    std::unordered_map<int, int> distMap;
    std::queue<int> q;

    distMap[startId] = 0;
    q.push(startId);

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        int d = distMap[curr];
        int currIdx = graph.getSortedIndexById(curr);
        distances[currIdx] = d;

        std::vector<int> nbrs = graph.getNeighbours(curr);
        for (int nxt : nbrs) {
            if (distMap.find(nxt) == distMap.end()) {
                distMap[nxt] = d + 1;
                q.push(nxt);
            }
        }
    }

    return distances;
}

DistanceVerificationReport DistanceVerification::verifyAllPairs(const IntervalGraph& graph, const DistanceTree& tree) {
    DistanceVerificationReport report;
    int n = graph.getNumVertices();
    report.numVertices = n;
    report.totalPairsTested = 0;
    report.matchedPairs = 0;

    const auto& sortedIvs = graph.getSortedIntervals();

    for (int i = 0; i < n; ++i) {
        int uId = sortedIvs[i].id;
        std::vector<int> bfsDists = computeSingleSourceBfs(graph, uId);

        for (int j = 0; j < n; ++j) {
            if (i == j) continue; // Distinct pairs
            int vId = sortedIvs[j].id;

            report.totalPairsTested++;

            int bfsDist = bfsDists[j];
            int formulaDist = getDistanceFormula(tree, uId, vId);

            // Also directly verify raw Equation 2.2 when pos_level(u) > pos_level(v)
            const auto& uNode = tree.getNodeById(uId);
            const auto& vNode = tree.getNodeById(vId);
            if (uNode.posLevel > vNode.posLevel) {
                int rawDist = evaluateEquation2_2Raw(tree, uId, vId);
                (void)rawDist;
                assert(rawDist == formulaDist);
            }

            if (bfsDist == formulaDist) {
                report.matchedPairs++;
            } else {
                DistanceMismatch mm;
                mm.uId = uId;
                mm.vId = vId;
                mm.uDepth = uNode.depth;
                mm.vDepth = vNode.depth;
                mm.uPosLevel = uNode.posLevel;
                mm.vPosLevel = vNode.posLevel;
                mm.uPosPost = uNode.posPost;
                mm.vPosPost = vNode.posPost;
                mm.formulaDist = formulaDist;
                mm.bfsDist = bfsDist;

                std::ostringstream oss;
                oss << "Mismatch between u=" << uId << " and v=" << vId
                    << ": BFS=" << bfsDist << ", Formula=" << formulaDist
                    << " [uDepth=" << mm.uDepth << ", vDepth=" << mm.vDepth
                    << ", uPost=" << mm.uPosPost << ", vPost=" << mm.vPosPost << "]";
                mm.details = oss.str();
                report.mismatches.push_back(mm);
            }
        }
    }

    return report;
}

bool DistanceVerification::verifyPositionalOrderingProperty(const IntervalGraph& graph, const DistanceTree& tree, std::string& errorMsg) {
    int n = graph.getNumVertices();
    const auto& sortedIvs = graph.getSortedIntervals();

    // Verify claim: pos_level(u) < pos_level(v) <=> u precedes v in interval ordering
    for (int i = 0; i < n; ++i) {
        int uId = sortedIvs[i].id;
        const auto& uNode = tree.getNodeById(uId);

        for (int j = i + 1; j < n; ++j) {
            int vId = sortedIvs[j].id;
            const auto& vNode = tree.getNodeById(vId);

            // Since i < j in sorted order, u precedes v
            if (uNode.posLevel >= vNode.posLevel) {
                std::ostringstream oss;
                oss << "Positional violation: vertex " << uId << " (sorted index " << i 
                    << ") precedes vertex " << vId << " (sorted index " << j 
                    << "), but pos_level(" << uId << ") = " << uNode.posLevel 
                    << " >= pos_level(" << vId << ") = " << vNode.posLevel;
                errorMsg = oss.str();
                return false;
            }
        }
    }

    // Verify Section 2.8 claim: When two vertices have the same depth,
    // their preorder, postorder, and level-order positions are all consistent with interval ordering.
    for (int d = 0; d <= tree.getMaxDepth(); ++d) {
        const auto& lvl = tree.getLevelVertices(d);
        for (size_t k = 0; k + 1 < lvl.size(); ++k) {
            const auto& a = tree.getNodeById(lvl[k]);
            const auto& b = tree.getNodeById(lvl[k + 1]);

            if (a.posLevel >= b.posLevel || a.posPre >= b.posPre || a.posPost >= b.posPost) {
                std::ostringstream oss;
                oss << "Level-depth consistency violation at depth " << d 
                    << " between consecutive level vertices " << a.vertexId << " and " << b.vertexId << ": "
                    << "posLevel(" << a.posLevel << " vs " << b.posLevel << "), "
                    << "posPre(" << a.posPre << " vs " << b.posPre << "), "
                    << "posPost(" << a.posPost << " vs " << b.posPost << ")";
                errorMsg = oss.str();
                return false;
            }
        }
    }

    errorMsg.clear();
    return true;
}

bool DistanceVerification::verifyTraversalProperties(const DistanceTree& tree, std::string& errorMsg) {
    int n = static_cast<int>(tree.getAllNodes().size());
    std::vector<bool> seenPre(n, false);
    std::vector<bool> seenPost(n, false);
    std::vector<bool> seenLevel(n, false);

    for (const auto& node : tree.getAllNodes()) {
        if (node.posPre < 0 || node.posPre >= n || seenPre[node.posPre]) {
            errorMsg = "Invalid or duplicate pos_pre value: " + std::to_string(node.posPre);
            return false;
        }
        seenPre[node.posPre] = true;

        if (node.posPost < 0 || node.posPost >= n || seenPost[node.posPost]) {
            errorMsg = "Invalid or duplicate pos_post value: " + std::to_string(node.posPost);
            return false;
        }
        seenPost[node.posPost] = true;

        if (node.posLevel < 0 || node.posLevel >= n || seenLevel[node.posLevel]) {
            errorMsg = "Invalid or duplicate pos_level value: " + std::to_string(node.posLevel);
            return false;
        }
        seenLevel[node.posLevel] = true;
    }

    errorMsg.clear();
    return true;
}

std::vector<Interval> DistanceVerification::generateRandomProperIntervals(int n, unsigned int seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> stepDist(0.5, 2.5);
    std::uniform_real_distribution<double> lenDist(2.0, 5.0);

    std::vector<Interval> ivs;
    ivs.reserve(n);

    double currentLeft = 0.0;
    double prevRight = 0.0;

    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            double len = lenDist(rng);
            ivs.push_back({i + 1, currentLeft, currentLeft + len});
            prevRight = currentLeft + len;
        } else {
            // Pick step to advance left endpoint
            double step = stepDist(rng);
            double newLeft = currentLeft + step;

            // To guarantee connectedness: newLeft <= prevRight
            // If newLeft > prevRight, clamp step to ensure overlap
            if (newLeft > prevRight) {
                newLeft = prevRight - 0.2; // Overlap by 0.2
            }

            // To guarantee properness: newRight > prevRight
            double minRight = prevRight + 0.1;
            double chosenLen = lenDist(rng);
            double newRight = newLeft + chosenLen;
            if (newRight < minRight) {
                newRight = minRight + 0.5;
            }

            ivs.push_back({i + 1, newLeft, newRight});
            currentLeft = newLeft;
            prevRight = newRight;
        }
    }

    return ivs;
}
