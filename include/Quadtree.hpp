#pragma once

#include "raylib.h"
#include "Vehicle.hpp"
#include <vector>
#include <memory>

class Quadtree {
private:
    Rectangle boundary;
    int capacity;
    std::vector<Vehicle*> vehicles;
    bool divided;

    std::unique_ptr<Quadtree> northwest;
    std::unique_ptr<Quadtree> northeast;
    std::unique_ptr<Quadtree> southwest;
    std::unique_ptr<Quadtree> southeast;

    void subdivide();

public:
    Quadtree(Rectangle boundary, int capacity);
    ~Quadtree();

    bool insert(Vehicle* vehicle);
    std::vector<Vehicle*> query(Rectangle range);
    void clear();
    void draw(const Camera2D& camera);
};