#pragma once

#include "Object.hpp"
#include "raylib.h"
#include "Map.hpp"

class Vehicle : public Object {
private:
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    float speed;
    Color color;
    const Road* road;
    int targetPointIndex;

public:
    // Constructor
    Vehicle(Vector2 pos, Vector2 sz, float spd, Color col, const Road* r);

    // Override the virtual functions from the Object base class
    void update() override;
    void draw() override;
};