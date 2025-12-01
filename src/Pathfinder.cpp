#include "Pathfinder.hpp"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <iostream>

// Node for A* priority queue
struct Node {
    long id;
    float gScore; // Cost from start
    float fScore; // Estimated total cost (g + h)

    bool operator>(const Node& other) const {
        return fScore > other.fScore;
    }
};

#include "raymath.h"
#include <cfloat>

std::deque<long> Pathfinder::findPath(const Map* map, long startIntersection, long endIntersection) {
    if (!map) return {};
    
    const auto& intersections = map->getIntersections();
    const auto& outgoingRoads = map->getOutgoingRoads();
    
    // Check if start/end exist
    if (intersections.find(startIntersection) == intersections.end() || 
        intersections.find(endIntersection) == intersections.end()) {
        return {};
    }

    Vector2 targetPos = intersections.at(endIntersection).position;
    
    std::deque<long> path;
    long currentId = startIntersection;
    long previousId = -1; // To avoid immediate U-turns

    int pathLength = 20;

    for (int i = 0; i < pathLength; ++i) {
        auto it = outgoingRoads.find(currentId);
        if (it == outgoingRoads.end() || it->second.empty()) {
            break; 
        }

        const std::vector<const Road*>& roads = it->second;
        
        const Road* bestRoad = nullptr;
        float minDist = FLT_MAX;
        
        // Filter valid roads (avoid U-turn)
        std::vector<const Road*> validRoads;
        for (const Road* road : roads) {
            if (road->toIntersectionId != previousId) {
                validRoads.push_back(road);
            }
        }
        
        // If all roads lead back (dead end except for entry), allow U-turn
        if (validRoads.empty()) {
            validRoads = roads;
        }

        // Greedy selection: Pick the road that gets us closest to the target
        // Add a small random chance to pick a sub-optimal road to add variety? 
        // For now, strict greedy is fine as destinations are random.
        for (const Road* road : validRoads) {
            long nextId = road->toIntersectionId;
            if (intersections.find(nextId) != intersections.end()) {
                 float dist = Vector2Distance(intersections.at(nextId).position, targetPos);
                 if (dist < minDist) {
                     minDist = dist;
                     bestRoad = road;
                 }
            }
        }

        if (bestRoad) {
            path.push_back(bestRoad->toIntersectionId);
            previousId = currentId;
            currentId = bestRoad->toIntersectionId;
        } else {
            break;
        }
        
        if (currentId == endIntersection) break; // Reached destination early
    }

    return path;
}
