#pragma once

#include "Object.hpp"
#include "raylib.h"
#include "Map.hpp"
#include <vector>
#include <random>
#include <cstdint>

class Quadtree;

// State machine enum
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
    float speedFactor; // Multiplier for road speed limit (e.g. 0.9 to 1.1)
    float minSpeed;
    float acceleration;
    float deceleration;
    float maxTurningSpeed;
    float minTurningSpeed;
    float currentSpeed;
    Vector2 direction;

    // Stats Tracking
    double accumulatedSpeed;
    int64_t speedSamples;
    float timeActive;

    // Current road tracking
    const Road* currentRoad;
    int64_t targetIntersectionId;
    
    std::vector<Vector2> currentRoadPoints;
    int currentRoadPointIndex;
    
    bool isWaitingAtJunction;
    Map* map;
    std::mt19937 gen;
    VehicleState state;

public:
    Vehicle(Vector2 pos, Vector2 sz, Color col, Map* m);

    void update(Quadtree* quadtree, float deltaTime);
    void update(float deltaTime) override;
    void draw(bool debug) override;

    Vector2 getPosition() const;
    Vector2 getDirection() const;
    float getSpeed() const;
    float getAverageSpeed() const;
    Vector2 getSize() const;
    
    const Road* getRoad() const;
    bool isActive() const { return true; }

private:
    void selectRandomRoad();
    void startFollowingCurrentRoad();
    void updateStats(float deltaTime);
};