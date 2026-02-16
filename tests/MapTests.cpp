#include "Map.hpp"
#include "raylib.h"
#include <gtest/gtest.h>

// Test Fixture: Default Map constructor (0..0 bounds)
// Coordinate conversion tests skipped due to private member access limitations
class MapTest : public ::testing::Test {
protected:
  void SetUp() override { map = new Map(); }

  void TearDown() override { delete map; }

  Map *map;
};

// Verify intersection storage and spatial retrieval
TEST_F(MapTest, AddIntersection) {
  Intersection i;
  i.id = 1;
  i.position = {100, 100};
  map->addIntersection(i);

  const auto *retrieved = map->getClosestIntersection({100, 100});
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->id, 1);
}

// Verify road addition and adjacency list updates
TEST_F(MapTest, AddRoad) {
  Road r;
  r.fromIntersectionId = 1;
  r.toIntersectionId = 2;
  r.points = {{0, 0}, {100, 0}};
  map->addRoad(r);

  const auto &roads = map->getRoads();
  EXPECT_EQ(roads.size(), 1);

  const auto &outgoing = map->getOutgoingRoads();
  EXPECT_TRUE(outgoing.find(1) != outgoing.end());
}
