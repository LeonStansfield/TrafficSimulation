#pragma once

#if defined(_WIN32)
    #define NOGDI             // All GDI defines and routines
    #define NOUSER            // All USER defines and routines
#endif

#include "raylib.h"
#include <vector>
#include <map>
#include <iostream>
#include <limits>
#include <random>

struct Intersection {
    long id;
    Vector2 position;
};

struct Road {
    std::vector<Vector2> points;
};

class Map {
private:
    std::map<long, Vector2> nodes;
    std::map<long, Intersection> intersections;
    std::vector<Road> roads;
    float minLat, maxLat, minLon, maxLon;
    float worldWidth, worldHeight;

public:
    Map(const char* filename);
    void draw();

    float getWorldWidth() const { return worldWidth; }
    float getWorldHeight() const { return worldHeight; }
    Vector2 convertLatLonToWorld(Vector2 latLon);
    const std::vector<Road>& getRoads() const;
    const Road* getClosestRoad(Vector2 position) const;
    const Road* getRandomConnectedRoad(const Road& currentRoad, const Vector2& endPoint, std::mt19937& rng) const;
};