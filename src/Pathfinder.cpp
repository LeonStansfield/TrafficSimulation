#include "Pathfinder.hpp"
#include "raymath.h"
#include <algorithm> // for std::reverse
#include <iostream>
#include <limits>
#include <map>
#include <queue>

Pathfinder::Pathfinder(const Map *m) : map(m) {}

std::vector<const Road *> Pathfinder::findPath(long startIntersectionId,
                                               long endIntersectionId) {
  if (!map)
    return {};

  auto &intersections = map->getIntersections();
  auto &outgoingRoads = map->getOutgoingRoads();

  if (intersections.find(startIntersectionId) == intersections.end() ||
      intersections.find(endIntersectionId) == intersections.end()) {
    std::cerr << "Pathfinder Error: Invalid start or end ID." << std::endl;
    return {}; // Invalid start or end
  }

  Vector2 endPos = intersections.at(endIntersectionId).position;

  // Define a maximum expected speed for the heuristic to ensure admissibility.
  // 130 km/h is roughly 36 m/s. Let's use 50.0 m/s (~110 mph) to be safe and
  // efficient.
  const float MAX_POSSIBLE_SPEED = 50.0f;

  // Heuristic function: Estimate minimum time to destination
  auto heuristic = [&](long nodeId) {
    if (intersections.find(nodeId) == intersections.end())
      return 0.0f;
    float dist = Vector2Distance(intersections.at(nodeId).position, endPos);
    return dist / MAX_POSSIBLE_SPEED;
  };

  // Priority queue for open set: { f_cost, node_id }
  // f_cost = g_cost (time so far) + h_cost (estimated time remaining)
  std::priority_queue<std::pair<float, long>,
                      std::vector<std::pair<float, long>>,
                      std::greater<std::pair<float, long>>>
      openSet;

  // map<node_id, g_cost_time>
  std::map<long, float> gCost;

  // map<node_id, {came_from_node_id, road_taken_to_get_here}
  std::map<long, std::pair<long, const Road *>> cameFrom;

  // Initialize/Reset
  gCost[startIntersectionId] = 0.0f;
  openSet.push({heuristic(startIntersectionId), startIntersectionId});
  cameFrom[startIntersectionId] = {-1, nullptr};

  while (!openSet.empty()) {
    long currentId = openSet.top().second;
    float currentF = openSet.top().first;
    openSet.pop();

    // If we found a shorter path to this node already, skip (standard priority
    // queue optimization)
    if (gCost.find(currentId) != gCost.end() &&
        currentF > gCost[currentId] + heuristic(currentId)) {
      continue;
    }

    if (currentId == endIntersectionId) {
      // Reconstruct path
      std::vector<const Road *> path;
      long tempId = endIntersectionId;
      while (tempId != startIntersectionId) {
        auto &entry = cameFrom[tempId];
        if (entry.second == nullptr)
          break;
        path.push_back(entry.second);
        tempId = entry.first;
      }
      std::reverse(path.begin(), path.end());
      return path;
    }

    if (outgoingRoads.find(currentId) == outgoingRoads.end()) {
      continue; // Dead end, no outgoing roads
    }

    for (const Road *road : outgoingRoads.at(currentId)) {
      if (road->disabled.load())
        continue; // Disabled roads are excluded from pathfinding.

      long neighborId = road->toIntersectionId;

      // Cost function: cost = road->length / road->speedLimit;
      // Guard against divide by zero (static roads shouldn't be 0 speed)
      float speed = std::max(0.1f, road->speedLimit);
      float traversalTime = road->length / speed;

      float currentG = gCost.count(currentId)
                           ? gCost[currentId]
                           : std::numeric_limits<float>::infinity();
      float tentative_gCost = currentG + traversalTime;

      float neighborG = gCost.count(neighborId)
                            ? gCost[neighborId]
                            : std::numeric_limits<float>::infinity();

      if (tentative_gCost < neighborG) {
        gCost[neighborId] = tentative_gCost;
        float h = heuristic(neighborId);
        float fCost = tentative_gCost + h;
        openSet.push({fCost, neighborId});
        cameFrom[neighborId] = {currentId, road};
      }
    }
  }

  return {}; // No path found
}