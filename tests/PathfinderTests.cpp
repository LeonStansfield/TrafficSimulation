#include "Map.hpp"
#include "Pathfinder.hpp"
#include "raylib.h"
#include <gtest/gtest.h>

// Test Fixture: Sets up a known graph topology (1-2-3, 1->4)
class PathfinderTest : public ::testing::Test {
protected:
  void SetUp() override {
    map = new Map();

    // Add Intersections
    Intersection i1;
    i1.id = 1;
    i1.position = {0, 0};
    map->addIntersection(i1);
    Intersection i2;
    i2.id = 2;
    i2.position = {100, 0};
    map->addIntersection(i2);
    Intersection i3;
    i3.id = 3;
    i3.position = {200, 0};
    map->addIntersection(i3);
    Intersection i4;
    i4.id = 4;
    i4.position = {0, 100};
    map->addIntersection(i4);

    // Road 1->2 (Bidirectional)
    Road r12;
    r12.fromIntersectionId = 1;
    r12.toIntersectionId = 2;
    r12.length = 100;
    r12.points = {{0, 0}, {100, 0}};
    r12.speedLimit = 10;
    r12.isOneWay = false;
    map->addRoad(r12);

    // Road 2->3 (Bidirectional)
    Road r23;
    r23.fromIntersectionId = 2;
    r23.toIntersectionId = 3;
    r23.length = 100;
    r23.points = {{100, 0}, {200, 0}};
    r23.speedLimit = 10;
    r23.isOneWay = false;
    map->addRoad(r23);

    // Road 1->4 (One-way)
    Road r14;
    r14.fromIntersectionId = 1;
    r14.toIntersectionId = 4;
    r14.length = 100;
    r14.points = {{0, 0}, {0, 100}};
    r14.speedLimit = 10;
    r14.isOneWay = true;
    map->addRoad(r14);

    pathfinder = new Pathfinder(map);
  }

  void TearDown() override {
    delete pathfinder;
    delete map;
  }

  Map *map;
  Pathfinder *pathfinder;
};

// Verify path between adjacent intersections
TEST_F(PathfinderTest, FindPathDirect) {
  std::vector<const Road *> path = pathfinder->findPath(1, 2);
  ASSERT_EQ(path.size(), 1);
  EXPECT_EQ(path[0]->toIntersectionId, 2);
}

// Verify multi-step path (1 -> 2 -> 3)
TEST_F(PathfinderTest, FindPathMultiStep) {
  std::vector<const Road *> path = pathfinder->findPath(1, 3);
  ASSERT_EQ(path.size(), 2);
  EXPECT_EQ(path[0]->toIntersectionId, 2);
  EXPECT_EQ(path[1]->toIntersectionId, 3);
}

// Verify no path against one-way road direction
TEST_F(PathfinderTest, NoPath) {
  std::vector<const Road *> path = pathfinder->findPath(4, 1);
  EXPECT_TRUE(path.empty());
}

// Verify behavior with invalid intersection ID
TEST_F(PathfinderTest, InvalidIntersection) {
  std::vector<const Road *> path = pathfinder->findPath(1, 99);
  EXPECT_TRUE(path.empty());
}

// Verify disabled road is excluded from pathfinding
TEST_F(PathfinderTest, DisabledRoadExcluded) {
  // Disable road 1->2; path from 1 to 2 should now be impossible.
  const auto &roads = map->getRoads();
  for (const auto &r : roads) {
    if (r.fromIntersectionId == 1 && r.toIntersectionId == 2) {
      map->setRoadEnabled(&r, false);
      break;
    }
  }
  std::vector<const Road *> path = pathfinder->findPath(1, 2);
  EXPECT_TRUE(path.empty());
}

// Verify re-enabling a road restores pathfinding through it
TEST_F(PathfinderTest, ReenabledRoadAvailable) {
  const auto &roads = map->getRoads();
  const Road *target = nullptr;
  for (const auto &r : roads) {
    if (r.fromIntersectionId == 1 && r.toIntersectionId == 2) {
      target = &r;
      break;
    }
  }
  ASSERT_NE(target, nullptr);
  map->setRoadEnabled(target, false);
  map->setRoadEnabled(target, true);
  std::vector<const Road *> path = pathfinder->findPath(1, 2);
  EXPECT_FALSE(path.empty());
}
