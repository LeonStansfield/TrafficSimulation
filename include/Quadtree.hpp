#pragma once

#include "Vehicle.hpp"
#include "raylib.h"
#include <memory>
#include <vector>

class Quadtree {
private:
  Rectangle boundary;
  int capacity;
  std::vector<Vehicle *> vehicles;
  bool divided;

  std::unique_ptr<Quadtree> northwest;
  std::unique_ptr<Quadtree> northeast;
  std::unique_ptr<Quadtree> southwest;
  std::unique_ptr<Quadtree> southeast;

  void subdivide();

public:
  Quadtree(Rectangle boundary, int capacity);
  ~Quadtree();

  bool insert(Vehicle *vehicle);
  std::vector<Vehicle *> query(Rectangle range) const;
  void clear();
  void draw(const Camera2D &camera) const;
};