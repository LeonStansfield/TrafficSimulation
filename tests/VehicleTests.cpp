#include "Map.hpp"
#include "Pathfinder.hpp"
#include "Vehicle.hpp"
#include "raylib.h"
#include <gtest/gtest.h>

// Test Fixture: Setup minimal map for vehicle placement
class VehicleTest : public ::testing::Test {
protected:
  void SetUp() override {
    map = new Map();

    // Create road from (0,0) to (1000,0)
    Intersection i1;
    i1.id = 1;
    i1.position = {0, 0};
    map->addIntersection(i1);
    Intersection i2;
    i2.id = 2;
    i2.position = {1000, 0};
    map->addIntersection(i2);
    Road r;
    r.fromIntersectionId = 1;
    r.toIntersectionId = 2;
    r.points = {{0, 0}, {1000, 0}};
    r.speedLimit = 20;
    r.lanes = 1;
    r.isOneWay = false;
    r.length = 1000;
    map->addRoad(r);

    // Dummy intersection for non-empty map
    Intersection i3;
    i3.id = 3;
    i3.position = {0, 0};
    map->addIntersection(i3);
  }

  void TearDown() override { delete map; }

  Map *map;
};

// Verify correct initialization and road snapping
TEST_F(VehicleTest, Initialization) {
  Vehicle v({0, 0}, {5, 5}, RED, map, nullptr);

  // Expect snap to (0,0)
  EXPECT_FLOAT_EQ(v.getPosition().x, 0);
  EXPECT_FLOAT_EQ(v.getPosition().y, 0);
}

// Verify update logic runs without crashing on minimal dependencies
TEST_F(VehicleTest, Movement) {
  Vehicle v({0, 0}, {5, 5}, RED, map, nullptr);

  v.update(1.0f);

  // Check valid state update
  EXPECT_GT(v.getAverageSpeed(), -1.0f);
}
