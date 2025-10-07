#pragma once

#include "Object.hpp"
#include "raylib.h"
#include "Map.hpp"
#include <vector>

class Vehicle : public Object {
private:
    const Map* map;
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    float speed;
    Color color;
    std::vector<Vector2> path;
    int currentPathIndex;

public:
    Vehicle(const Map* map, Vector2 sz, float spd, Color col);

    void update() override;
    void draw() override;

private:
    void findNewPath();
};