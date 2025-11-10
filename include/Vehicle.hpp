#pragma once

#include "Object.hpp"
#include "raylib.h"
#include "Map.hpp"
#include <vector>
#include <random>

class Quadtree;

// Define the state machine enum
enum class VehicleState {
    DRIVING,
    BRAKING,
    WAITING_JUNCTION
};

class Vehicle : public Object {
private:
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    Color color;
    Color originalColor;

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

    bool isWaitingAtJunction;
    const Road* nextRoadToJoin;

    Map* map;

    std::mt19937 gen;

    VehicleState state;

public:
    // Constructor
    Vehicle(Vector2 pos, Vector2 sz, Color col, Map* m);

    // Override the virtual functions from the Object base class
    void update(Quadtree* quadtree);
    void update() override;
    void draw(bool debug) override;

    Vector2 getPosition() const;
    Vector2 getDirection() const;
    float getSpeed() const;
    Vector2 getSize() const;
    const Road* getRoad() const { return road; }

private:
    void findNewPath();
};