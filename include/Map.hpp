#pragma once

#include "raylib.h"
#include <vector>
#include <map>

// A struct to hold road information
struct Road {
    std::vector<Vector2> points;
};

class Map {
private:
    std::map<long, Vector2> nodes;
    std::vector<Road> roads;
    // Store the map's boundaries
    float minLat, maxLat, minLon, maxLon;


public:
    Map(const char* filename);
    void draw();
};