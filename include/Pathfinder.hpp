#pragma once

#include "Map.hpp"
#include <vector>
#include <memory>

class Pathfinder {
private:
    const Map* map; // Non-owning pointer to the map data

public:
    Pathfinder(const Map* map);

    /**
     * @brief Finds the shortest path (list of roads) between two intersection IDs using A*.
     * @param startIntersectionId The ID of the starting intersection.
     * @param endIntersectionId The ID of the destination intersection.
     * @return A vector of const Road pointers representing the path. Empty if no path is found.
     */
    std::vector<const Road*> findPath(long startIntersectionId, long endIntersectionId);
};