#pragma once

#include "Object.hpp"
#include "raylib.h"
#include "Map.hpp"
#include <vector>
#include <random>

class Vehicle : public Object {
private:
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    Color color;

    // Kinematic Properties
    float maxSpeed;
    float minSpeed;
    float acceleration;
    float deceleration;
    float maxTurningSpeed;
    float minTurningSpeed;
    float currentSpeed;
    Vector2 direction;

    const Road* road;
    int targetPointIndex;
    std::vector<Vector2> path;
    int currentPathIndex;

    Map* map;

    std::mt19937 gen;

public:
    // Constructor
    Vehicle(Vector2 pos, Vector2 sz, Color col, Map* m);

    // Override the virtual functions from the Object base class
    void update() override;
    void draw() override;

private:
    void findNewPath();
};