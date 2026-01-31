#pragma once

#include "Map.hpp"
#include "Object.hpp"
#include "Pathfinder.hpp"
#include "Quadtree.hpp"
#include "raylib.h"
#include <memory>
#include <vector>

enum class SelectionType { NONE, VEHICLE, INTERSECTION, ROAD };

class Engine {
private:
  int screenWidth;
  int screenHeight;
  Camera2D camera;
  Font font;

  std::vector<std::unique_ptr<Object>> objects;
  std::unique_ptr<Map> map;
  std::unique_ptr<Quadtree> quadtree;
  std::unique_ptr<Pathfinder> pathfinder;

  DrawMode currentMode;

  // Selection State
  SelectionType selectionType;
  const Object *selectedVehicle;
  const Intersection *selectedIntersection;
  const Road *selectedRoad;

public:
  Engine(int width, int height, const char *title);
  ~Engine();

  void setMap(std::unique_ptr<Map> newMap);
  void addObject(std::unique_ptr<Object> object);
  void run();
  void runFast(int ticks);

private:
  void updateSimulation(float deltaTime);
  void drawUI();

public:
  void spawnVehicles(int count);
};