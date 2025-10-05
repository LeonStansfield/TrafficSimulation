#pragma once

#include "Object.hpp"
#include "raylib.h"

class Vehicle : public Object {
private:
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    float speed;
    Color color;

public:
    // Constructor
    Vehicle(Vector2 pos, Vector2 sz, float spd, Color col);

    // Override the virtual functions from the Object base class
    void update() override;
    void draw() override;
};