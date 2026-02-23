#pragma once

#include "Map.hpp"
#include "Object.hpp"
#include "Pathfinder.hpp"
#include "Quadtree.hpp"
#include "ThreadPool.hpp"
#include <memory>
#include <vector>

// Forward decl
class Renderer;

class Simulation {
private:
  std::vector<std::unique_ptr<Object>> objects;
  std::unique_ptr<Map> map;
  std::unique_ptr<Quadtree> quadtree;
  std::unique_ptr<Pathfinder> pathfinder;
  std::unique_ptr<ThreadPool> threadPool;

public:
  Simulation();
  ~Simulation();

  void setMap(std::unique_ptr<Map> newMap);
  void update(float deltaTime);
  void spawnVehicles(int count);
  void addObject(std::unique_ptr<Object> object);

  long getTickCount() const { return tickCount; }
  int getVehicleCount() const {
    return objects.size();
  } // Approximate, includes all objects

private:
  long tickCount = 0;
  bool useQuadtree = true;

public:
  void setUseQuadtree(bool useIt) { useQuadtree = useIt; }
  bool getUseQuadtree() const { return useQuadtree; }

  // Getters for Renderer / Input
  const Map *getMap() const { return map.get(); }
  const Quadtree *getQuadtree() const { return quadtree.get(); }
  const std::vector<std::unique_ptr<Object>> &getObjects() const {
    return objects;
  }

  // Mutable getters for internal updates if needed, though const preferred for
  // external
  Map *getMapMutable() { return map.get(); }
  Pathfinder *getPathfinder() { return pathfinder.get(); }
};
