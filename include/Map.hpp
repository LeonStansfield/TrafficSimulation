#pragma once

#if defined(_WIN32)
#define NOGDI  // All GDI defines and routines
#define NOUSER // All USER defines and routines
#endif

#include "raylib.h"
#include "raymath.h"
#include <atomic>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <vector>

struct RoadStats {
  mutable std::atomic<long> vehiclesPassed{0};
  mutable std::atomic<double> accumulatedSpeed{0.0};
  mutable std::atomic<long> speedSamples{0};

  RoadStats() = default;

  RoadStats(const RoadStats &other) {
    vehiclesPassed.store(other.vehiclesPassed.load());
    accumulatedSpeed.store(other.accumulatedSpeed.load());
    speedSamples.store(other.speedSamples.load());
  }

  // Assignment operator required for vector operations
  RoadStats &operator=(const RoadStats &other) {
    if (this != &other) {
      vehiclesPassed.store(other.vehiclesPassed.load());
      accumulatedSpeed.store(other.accumulatedSpeed.load());
      speedSamples.store(other.speedSamples.load());
    }
    return *this;
  }

  float getAverageSpeed() const {
    long samples = speedSamples.load();
    return (samples > 0) ? static_cast<float>(accumulatedSpeed.load() / samples)
                         : 0.0f;
  }

  void addSpeed(double speed) {
    double current = accumulatedSpeed.load();
    while (!accumulatedSpeed.compare_exchange_weak(current, current + speed)) {
    }
  }
};

struct IntersectionStats {
  mutable std::atomic<long> vehiclesVisited{0};

  IntersectionStats() = default;

  IntersectionStats(const IntersectionStats &other) {
    vehiclesVisited.store(other.vehiclesVisited.load());
  }

  IntersectionStats &operator=(const IntersectionStats &other) {
    if (this != &other) {
      vehiclesVisited.store(other.vehiclesVisited.load());
    }
    return *this;
  }
};

struct Intersection {
  long id;
  Vector2 position;
  mutable IntersectionStats stats;
};

struct Road {
  long fromIntersectionId;
  long toIntersectionId;
  std::vector<Vector2> points;
  bool isOneWay;
  float length;
  float speedLimit;
  int lanes;
  bool isRoundabout;
  mutable RoadStats stats;
};

enum class DrawMode { NORMAL, HEATMAP, DEBUG };

class Map {
private:
  std::map<long, Vector2> nodes;
  std::map<long, Intersection> intersections;
  std::vector<Road> roads;
  std::map<long, std::vector<const Road *>> outgoingRoads;
  std::map<long, std::vector<const Road *>> incomingRoads;

  std::map<std::pair<long, long>, std::vector<Vector2>> baseRoadSegments;
  std::map<std::pair<long, long>, bool> baseSegmentOneWay;

  float minLat, maxLat, minLon, maxLon;
  float worldWidth, worldHeight;

public:
  Map();
  Map(const char *filename, int maxRoads = 0);
  void addIntersection(const Intersection &intersection);
  void addRoad(const Road &road);
  void draw(DrawMode mode);

  float getWorldWidth() const { return worldWidth; }
  float getWorldHeight() const { return worldHeight; }
  Vector2 convertLatLonToWorld(Vector2 latLon);

  const std::vector<Road> &getRoads() const;
  const Road *getClosestRoad(Vector2 position) const;
  const Intersection *getClosestIntersection(Vector2 position) const;

  long getRandomIntersectionId(std::mt19937 &rng) const;
  const std::map<long, Intersection> &getIntersections() const {
    return intersections;
  }
  const std::map<long, std::vector<const Road *>> &getOutgoingRoads() const {
    return outgoingRoads;
  }
  const std::map<long, std::vector<const Road *>> &getIncomingRoads() const {
    return incomingRoads;
  }
};