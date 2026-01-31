#include "Simulation.hpp"
#include "Vehicle.hpp"
#include <iostream>
#include <random>
#include <time.h>

Simulation::Simulation()
    : map(nullptr), quadtree(nullptr), pathfinder(nullptr) {}

Simulation::~Simulation() {}

void Simulation::setMap(std::unique_ptr<Map> newMap) {
  map = std::move(newMap);
  if (map) {
    quadtree = std::make_unique<Quadtree>(
        Rectangle{0, 0, map->getWorldWidth(), map->getWorldHeight()}, 4);
    pathfinder = std::make_unique<Pathfinder>(map.get());
  }
}

void Simulation::addObject(std::unique_ptr<Object> object) {
  objects.push_back(std::move(object));
}

void Simulation::update(float deltaTime) {
  if (!quadtree)
    return;

  // Rebuild the quadtree
  quadtree->clear();
  for (const auto &obj : objects) {
    Vehicle *v = dynamic_cast<Vehicle *>(obj.get());
    if (v) {
      quadtree->insert(v);
    }
  }

  // Update all objects
  for (const auto &obj : objects) {
    Vehicle *v = dynamic_cast<Vehicle *>(obj.get());
    if (v) {
      v->update(quadtree.get(), deltaTime);
    } else {
      obj->update(deltaTime);
    }
  }
}

void Simulation::spawnVehicles(int count) {
  if (!map || !pathfinder) {
    return;
  }

  const auto &roads = map->getRoads();
  if (roads.empty()) {
    return;
  }

  std::mt19937 rng(time(nullptr));
  std::uniform_int_distribution<int> road_dist(0, roads.size() - 1);

  for (int i = 0; i < count; ++i) {
    const Road &random_road = roads[road_dist(rng)];
    if (random_road.points.size() < 2)
      continue;

    std::uniform_int_distribution<int> segment_dist(
        0, random_road.points.size() - 2);
    int segment_index = segment_dist(rng);

    Vector2 p1 = random_road.points[segment_index];
    Vector2 p2 = random_road.points[segment_index + 1];

    std::uniform_real_distribution<float> pos_dist(0.0f, 1.0f);
    float t = pos_dist(rng);
    Vector2 pos = {p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y)};

    addObject(std::make_unique<Vehicle>(pos, Vector2{10, 10}, RED, map.get(),
                                        pathfinder.get()));
  }
}
