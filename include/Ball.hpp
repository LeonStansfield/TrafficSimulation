#pragma once

#include "Object.hpp"
#include "raylib.h"

class Ball : public Object {
private:
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;

public:
    // Constructor
    Ball(Vector2 pos, Vector2 vel, float r, Color c);

    // Override the virtual functions from the Object base class
    void update() override;
    void draw() override;
};