#pragma once

#include "Map.hpp"
#include <vector>
#include <deque>
#include <random>

class Pathfinder {
public:
    // Returns a list of intersection IDs representing the path from start to end
    // Returns an empty vector if no path is found
    static std::deque<long> findPath(const Map* map, long startIntersection, long endIntersection);
};
