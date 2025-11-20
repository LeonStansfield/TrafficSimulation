#include "Pathfinder.hpp"
#include "raymath.h"
#include <map>
#include <queue>
#include "Pathfinder.hpp"
#include "raymath.h"
#include <map>
#include <queue>
#include <limits>
#include <algorithm> // for std::reverse
#include <iostream>

Pathfinder::Pathfinder(const Map* m) : map(m) {}

// A* Implementation
std::vector<const Road*> Pathfinder::findPath(long startIntersectionId, long endIntersectionId) {
    std::cout << "Pathfinder::findPath " << startIntersectionId << " -> " << endIntersectionId << std::endl;
    if (!map) return {};

    auto& intersections = map->getIntersections();
    auto& outgoingRoads = map->getOutgoingRoads();

    if (intersections.find(startIntersectionId) == intersections.end() ||
        intersections.find(endIntersectionId) == intersections.end()) {
        std::cerr << "Pathfinder Error: Invalid start or end ID." << std::endl;
        return {}; // Invalid start or end
    }

    Vector2 endPos = intersections.at(endIntersectionId).position;

    // Heuristic function (Euclidean distance)
    auto heuristic = [&](long nodeId) {
        if (intersections.find(nodeId) == intersections.end()) return 0.0f;
        return Vector2Distance(intersections.at(nodeId).position, endPos);
    };

    // Priority queue for open set: { f_cost, node_id }
    std::priority_queue<std::pair<float, long>,
                        std::vector<std::pair<float, long>>,
                        std::greater<std::pair<float, long>>> openSet;

    // map<node_id, g_cost>
    std::map<long, float> gCost;

    // map<node_id, {came_from_node_id, road_taken_to_get_here}
    std::map<long, std::pair<long, const Road*>> cameFrom;

    for (auto const& [id, intersection] : intersections) {
        gCost[id] = std::numeric_limits<float>::infinity();
    }

    gCost[startIntersectionId] = 0.0f;
    openSet.push({ heuristic(startIntersectionId), startIntersectionId });
    cameFrom[startIntersectionId] = {-1, nullptr};

    int iterations = 0;
    while (!openSet.empty()) {
        iterations++;
        if (iterations > 10000) {
             std::cerr << "Pathfinder: Exceeded iteration limit! Possible infinite loop." << std::endl;
             break;
        }

        long currentId = openSet.top().second;
        openSet.pop();

        if (currentId == endIntersectionId) {
            // Reconstruct path
            std::vector<const Road*> path;
            long tempId = endIntersectionId;
            while (tempId != startIntersectionId) {
                auto& entry = cameFrom[tempId];
                if (entry.second == nullptr) break; 
                path.push_back(entry.second);
                tempId = entry.first;
            }
            std::reverse(path.begin(), path.end());
            std::cout << "Path found with " << path.size() << " steps." << std::endl;
            return path;
        }

        if (outgoingRoads.find(currentId) == outgoingRoads.end()) {
            continue; // Dead end, no outgoing roads
        }

        for (const Road* road : outgoingRoads.at(currentId)) {
            long neighborId = road->toIntersectionId;
            // The cost to move along this road is its length
            float tentative_gCost = gCost[currentId] + road->length;

            if (tentative_gCost < gCost[neighborId]) {
                gCost[neighborId] = tentative_gCost;
                float fCost = tentative_gCost + heuristic(neighborId);
                openSet.push({ fCost, neighborId });
                cameFrom[neighborId] = { currentId, road };
            }
        }
    }

    std::cout << "No path found." << std::endl;
    return {}; // No path found
}