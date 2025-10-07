#pragma once

#if defined(_WIN32)
    #define NOGDI
    #define NOUSER
#endif

#include "raylib.h"
#include <vector>
#include <map>
#include <iostream>

struct Road {
    std::vector<Vector2> points;
};

class Map {
private:
    std::map<long, Vector2> nodes;
    std::vector<Road> roads;
    float minLat, maxLat, minLon, maxLon;
    float worldWidth, worldHeight;

public:
    Map(const char* filename);
    void draw();

    float getWorldWidth() const { return worldWidth; }
    float getWorldHeight() const { return worldHeight; }
    Vector2 convertLatLonToWorld(Vector2 latLon) const;
    std::vector<Vector2> getRandomRoad() const;
};