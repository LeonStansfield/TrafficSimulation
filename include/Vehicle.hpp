#pragma once

#include "Object.hpp"
#include "raylib.h"
#include "Map.hpp"
#include "Pathfinder.hpp"
#include <vector>
#include <random>

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
    float minSpeed;
    float acceleration;
    float deceleration;
    float maxTurningSpeed;
    float minTurningSpeed;
    float currentSpeed;
    Vector2 direction;

    // Stats Tracking
    double accumulatedSpeed;
    long speedSamples;
    float timeActive;

    Pathfinder* pathfinder;
    long destinationIntersectionId;
    std::vector<const Road*> currentPath;
    int currentPathRoadIndex;
    
    std::vector<Vector2> currentRoadPoints;
    int currentRoadPointIndex;
    
    bool isWaitingAtJunction;
    Map* map;
    std::mt19937 gen;
    VehicleState state;

public:
    Vehicle(Vector2 pos, Vector2 sz, Color col, Map* m, Pathfinder* pf);

    void update(Quadtree* quadtree, float deltaTime);
    void update(float deltaTime) override;
    void draw(bool debug) override;

    Vector2 getPosition() const;
    Vector2 getDirection() const;
    float getSpeed() const;
    float getAverageSpeed() const;
    Vector2 getSize() const;
    
    const Road* getRoad() const;
    
    // Path visualization getters
    const std::vector<const Road*>& getPath() const;
    int getPathIndex() const;

private:
    void requestNewPath();
    void startFollowingCurrentRoad();
    void updateStats(float deltaTime);
};