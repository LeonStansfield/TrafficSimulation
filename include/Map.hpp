#pragma once

#include "raylib.h"
#include <vector>
#include <map>
#include <iostream>

// A struct to hold road information
struct Road {
    std::vector<Vector2> points;
};

class Map {
private:
    std::map<long, Vector2> nodes;
    std::vector<Road> roads;
    float minLat, maxLat, minLon, maxLon;


public:
    Map(const char* filename);
    void draw();
};