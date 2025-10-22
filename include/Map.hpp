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
    long fromIntersectionId; // ID of the intersection it starts at
    long toIntersectionId;   // ID of the intersection it ends at
    std::vector<Vector2> points; // World-space points defining the path
    bool isOneWay;           // Was this road originally a one-way?
};

class Map {
private:
    std::map<long, Vector2> nodes;
    std::map<long, Intersection> intersections;
    std::vector<Road> roads;
    // Adjacency list for the directed graph
    std::map<long, std::vector<const Road*>> outgoingRoads; 

    std::map<std::pair<long, long>, std::vector<Vector2>> baseRoadSegments;
    std::map<std::pair<long, long>, bool> baseSegmentOneWay;

    float minLat, maxLat, minLon, maxLon;
    float worldWidth, worldHeight;

public:
    Map(const char* filename);
    void draw(bool debug);

    float getWorldWidth() const { return worldWidth; }
    float getWorldHeight() const { return worldHeight; }
    Vector2 convertLatLonToWorld(Vector2 latLon);
    const std::vector<Road>& getRoads() const;
    const Road* getClosestRoad(Vector2 position) const;
    const Road* getRandomOutgoingRoad(long fromIntersectionId, std::mt19937& rng) const;
};