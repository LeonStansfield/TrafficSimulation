#include "Quadtree.hpp"
#include "Vehicle.hpp"
#include "raylib.h"
#include <gtest/gtest.h>

// Helper to create a dummy vehicle isolated from Map logic
Vehicle *createVehicle(float x, float y) {
  return new Vehicle({x, y}, {10, 10}, RED, nullptr, nullptr);
}

// Test Fixture: Sets up a 100x100 Quadtree with capacity 4
class QuadtreeTest : public ::testing::Test {
protected:
  void SetUp() override {
    boundary = {0, 0, 100, 100};
    quadtree = new Quadtree(boundary, 4);
  }

  void TearDown() override {
    delete quadtree;
    for (auto v : vehicles) {
      delete v;
    }
  }

  Rectangle boundary;
  Quadtree *quadtree;
  std::vector<Vehicle *> vehicles;

  // Adds vehicle to tracking list and inserts into quadtree
  void addVehicle(float x, float y) {
    Vehicle *v = createVehicle(x, y);
    vehicles.push_back(v);
    quadtree->insert(v);
  }
};

// Verify basic insertion and range query
TEST_F(QuadtreeTest, InsertAndQuery) {
  addVehicle(10, 10);
  addVehicle(20, 20);
  addVehicle(80, 80); // Outside query range

  Rectangle range = {0, 0, 50, 50};
  auto results = quadtree->query(range);

  EXPECT_EQ(results.size(), 2);
}

// Ensure insertion fails if object is out of bounds
TEST_F(QuadtreeTest, OutOfBoundsInsert) {
  Vehicle *v = createVehicle(150, 150);
  bool inserted = quadtree->insert(v);

  EXPECT_FALSE(inserted);
  delete v;
}

// Test subdivision when capacity (4) is exceeded
TEST_F(QuadtreeTest, CapacitySubdivision) {
  addVehicle(10, 10);
  addVehicle(12, 12);
  addVehicle(14, 14);
  addVehicle(16, 16);
  addVehicle(18, 18); // Triggers split

  Rectangle range = {0, 0, 50, 50};
  auto results = quadtree->query(range);

  EXPECT_EQ(results.size(), 5);
}

// Test query overlapping quadrant boundaries
TEST_F(QuadtreeTest, BoundaryQuery) {
  addVehicle(50, 50);

  Rectangle range = {40, 40, 20, 20};
  auto results = quadtree->query(range);
  EXPECT_EQ(results.size(), 1);
}

// Verify clear functionality
TEST_F(QuadtreeTest, Clear) {
  addVehicle(10, 10);
  quadtree->clear();

  Rectangle range = {0, 0, 100, 100};
  auto results = quadtree->query(range);
  EXPECT_TRUE(results.empty());
}
