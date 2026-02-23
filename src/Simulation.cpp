#include "Simulation.hpp"
#include "Benchmarker.hpp"
#include "Vehicle.hpp"
#include <iostream>
#include <random>
#include <time.h>

Simulation::Simulation()
    : map(nullptr), quadtree(nullptr), pathfinder(nullptr) {
  unsigned int threads = std::thread::hardware_concurrency();
  if (threads == 0)
    threads = 4;
  threadPool = std::make_unique<ThreadPool>(threads);
}

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
  tickCount++;
  if (!quadtree)
    return;

  // Rebuild the quadtree (Single threaded)
  Benchmarker::Get().Start("Quadtree");
  quadtree->clear();
  for (const auto &obj : objects) {
    Vehicle *v = dynamic_cast<Vehicle *>(obj.get());
    if (v) {
      quadtree->insert(v);
    }
  }
  Benchmarker::Get().Stop("Quadtree");

  // Parallel Update Loop
  size_t count = objects.size();
  if (count == 0)
    return;

  Benchmarker::Get().Start("Vehicles");
  // Simple heuristic: don't thread if very few objects
  if (count < 100) {
    for (const auto &obj : objects) {
      Vehicle *v = dynamic_cast<Vehicle *>(obj.get());
      if (v) {
        v->update(useQuadtree ? quadtree.get() : nullptr, objects, deltaTime);
      } else {
        obj->update(deltaTime);
      }
    }
    Benchmarker::Get().Stop("Vehicles");
    return;
  }

  // Determine number of threads to use (hardware concurrency or default to 4)
  unsigned int numThreads = std::thread::hardware_concurrency();
  if (numThreads == 0)
    numThreads = 4;

  // Calculate chunk size to distribute work evenly
  size_t chunkSize = (count + numThreads - 1) / numThreads; // Ceiling division

  // Store futures to track task completion
  std::vector<std::future<void>> futures;
  futures.reserve(numThreads);

  for (unsigned int i = 0; i < numThreads; ++i) {
    size_t start = i * chunkSize;
    size_t end = std::min(start + chunkSize, count);

    if (start >= end)
      break;

    // Enqueue a lambda to the thread pool.
    // This lambda captures 'this' to access objects/quadtree, and the range
    // [start, end)
    futures.emplace_back(threadPool->enqueue([this, start, end, deltaTime] {
      for (size_t j = start; j < end; ++j) {
        const auto &obj = this->objects[j];
        Vehicle *v = dynamic_cast<Vehicle *>(obj.get());
        if (v) {
          // Safe to read quadtree (read-only) and update vehicle (exclusive to
          // this thread)
          v->update(this->useQuadtree ? this->quadtree.get() : nullptr,
                    this->objects, deltaTime);
        } else {
          obj->update(deltaTime);
        }
      }
    }));
  }

  // Wait for all worker threads to finish their chunks before proceeding
  // (Synchronization)
  for (auto &f : futures) {
    f.get();
  }
  Benchmarker::Get().Stop("Vehicles");
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
