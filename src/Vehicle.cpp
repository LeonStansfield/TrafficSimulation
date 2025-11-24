#include "Vehicle.hpp"
#include "raylib.h"
#include "raymath.h"
#include "Map.hpp"
#include "Quadtree.hpp"
#include <cmath>
#include <random>
#include <algorithm>
#include <cfloat>
#include <iostream>

static float GetRelativeAngle(Vector2 v1, Vector2 v2) {
    float dot = v1.x * v2.x + v1.y * v2.y;
    float det = v1.x * v2.y - v1.y * v2.x;
    return atan2(det, dot);
}

Vehicle::Vehicle(Vector2 pos, Vector2 sz, Color col, Map* m)
    : position(pos), size(sz), color(col), originalColor(col), map(m),
      maxSpeed(20.0f), minSpeed(5.0f), acceleration(5.0f), deceleration(10.0f),
      maxTurningSpeed(2.0f), minTurningSpeed(0.5f), currentSpeed(0.0f),
      direction({1, 0}), accumulatedSpeed(0), speedSamples(0), timeActive(0),
      currentRoad(nullptr), targetIntersectionId(-1), currentRoadPointIndex(0),
      isWaitingAtJunction(false), state(VehicleState::DRIVING)
{
    // Initialize random engine with a random seed
    std::random_device rd;
    gen = std::mt19937(rd());

    std::uniform_real_distribution<> speed_dist(13.0f, 31.0f);
    std::uniform_real_distribution<> accel_dist(3.0f, 8.0f);
    std::uniform_real_distribution<> decel_dist(5.0f, 15.0f);
    std::uniform_real_distribution<> turning_dist(3.0f, 5.0f);
    std::uniform_real_distribution<> factor_dist(0.85f, 1.15f);

    speedFactor = factor_dist(gen);
    maxSpeed = 13.4f;
    minSpeed = 4.5f;
    acceleration = accel_dist(gen);
    deceleration = decel_dist(gen);
    maxTurningSpeed = turning_dist(gen);
    minTurningSpeed = 1.5f;

    currentSpeed = 0.0f;
    velocity = { 0, 0 };
    direction = { 1, 0 };

    // Snap to closest road and select initial direction
    currentRoad = map->getClosestRoad(position);
    if (currentRoad && !currentRoad->points.empty()) {
        // Find the closest point on this road to snap to
        float minDistSq = FLT_MAX;
        int closestIndex = 0;
        for(size_t i = 0; i < currentRoad->points.size(); ++i) {
            float distSq = Vector2DistanceSqr(position, currentRoad->points[i]);
            if (distSq < minDistSq) {
                minDistSq = distSq;
                closestIndex = i;
            }
        }
        position = currentRoad->points[closestIndex];
        
        maxSpeed = currentRoad->speedLimit * speedFactor;

        // Determine initial target intersection
        if (currentRoad->isOneWay) {
            targetIntersectionId = currentRoad->toIntersectionId;
        } else {
            std::uniform_int_distribution<> dist(0, 1);
            targetIntersectionId = (dist(gen) == 0) ? currentRoad->toIntersectionId : currentRoad->fromIntersectionId;
        }

        startFollowingCurrentRoad();
    } else {
        currentSpeed = 0.0f;
        velocity = { 0, 0 };
        state = VehicleState::WAITING_JUNCTION;
        targetIntersectionId = -1;
    }
}

void Vehicle::update(float deltaTime) {
    updateStats(deltaTime);
}

void Vehicle::updateStats(float deltaTime) {
    timeActive += deltaTime;
    accumulatedSpeed += currentSpeed;
    speedSamples++;

    if (currentSpeed > 0.1f) {
        const Road* road = getRoad();
        if (road) {
            road->stats.accumulatedSpeed += currentSpeed;
            road->stats.speedSamples++;
        }
    }
}

void Vehicle::update(Quadtree* quadtree, float deltaTime) {
    updateStats(deltaTime);

    // Check if we are at the end of the current road
    if (currentRoadPoints.empty() || currentRoadPointIndex >= currentRoadPoints.size() - 1) {
        
        if (!isWaitingAtJunction) {
            isWaitingAtJunction = true;
            state = VehicleState::WAITING_JUNCTION;
        }

        // At a junction - mark intersection as visited
        if (targetIntersectionId != -1) {
            const auto& intersections = map->getIntersections();
            if (intersections.find(targetIntersectionId) != intersections.end()) {
                intersections.at(targetIntersectionId).stats.vehiclesVisited++;
            }
        }

        // Select a random road to follow
        selectRandomRoad();
        
        if (currentRoadPoints.empty()) {
            velocity = {0, 0}; 
            currentSpeed = 0; 
            return;
        }
        
        isWaitingAtJunction = false;
        return;

    } else if (currentRoadPoints.empty()) {
        selectRandomRoad();
        if (currentRoadPoints.empty()) {
            velocity = {0, 0}; 
            currentSpeed = 0; 
            return;
        }
    }

    // Look-Ahead & Speed Control
    float lookAheadDistance = std::max(50.0f, currentSpeed * 3.0f);
    float totalAngleChange = 0.0f;
    float distanceTraveled = 0.0f;

    for (size_t i = currentRoadPointIndex; i < currentRoadPoints.size() - 2 && distanceTraveled < lookAheadDistance; ++i) {
        Vector2 p1 = currentRoadPoints[i];
        Vector2 p2 = currentRoadPoints[i+1];
        Vector2 p3 = currentRoadPoints[i+2];

        Vector2 segment1 = Vector2Normalize(Vector2Subtract(p2, p1));
        Vector2 segment2 = Vector2Normalize(Vector2Subtract(p3, p2));

        totalAngleChange += std::abs(GetRelativeAngle(segment1, segment2));
        distanceTraveled += Vector2Distance(p1, p2);
    }

    float curvature = (distanceTraveled > 0) ? totalAngleChange / distanceTraveled : 0;
    float curvatureFactor = std::clamp(curvature * 10.0f, 0.0f, 1.0f);
    float targetSpeed = maxSpeed - (maxSpeed - minSpeed) * curvatureFactor;

    if (currentSpeed > targetSpeed) {
        state = VehicleState::BRAKING;
    } else {
        state = VehicleState::DRIVING;
    }

    // Collision Avoidance
    float minGap = 8.0f; 
    float desiredTimeGap = 1.5f;
    float vehicleLookAhead = std::max(minGap * 5.0f, currentSpeed * (desiredTimeGap + 1.0f));
    
    Rectangle queryBox = { 
        position.x - vehicleLookAhead / 2.0f, 
        position.y - vehicleLookAhead / 2.0f, 
        vehicleLookAhead, 
        vehicleLookAhead 
    };
    auto nearby = quadtree->query(queryBox);
    Vehicle* leadVehicle = nullptr;
    float closestDistSqr = FLT_MAX;

    for (Vehicle* other : nearby) {
        if (other == this) continue;

        Vector2 toOther = Vector2Subtract(other->getPosition(), position);
        float distSqr = Vector2LengthSqr(toOther);
        if (distSqr == 0.0f || distSqr > vehicleLookAhead * vehicleLookAhead) continue;

        Vector2 otherDir = Vector2Normalize(toOther);
        float dot = Vector2DotProduct(direction, otherDir);

        if (dot > 0.9 && distSqr < closestDistSqr) {
            Vector2 otherVehicleDirection = other->getDirection();
            float directionDot = Vector2DotProduct(direction, otherVehicleDirection);
            if (directionDot > 0.5f) {
                closestDistSqr = distSqr;
                leadVehicle = other;
            }
        }
    }

    if (leadVehicle) {
        float distance = sqrt(closestDistSqr) - (leadVehicle->getSize().x / 2.0f) - (size.x / 2.0f);
        float safeDistance = currentSpeed * desiredTimeGap + minGap;

        if (distance < safeDistance) {
            float brakeSpeed = leadVehicle->getSpeed() + (distance - safeDistance) / desiredTimeGap;
            targetSpeed = std::min(targetSpeed, std::max(0.0f, brakeSpeed));
            state = VehicleState::BRAKING; 
        }
        
        if (distance < minGap) {
            targetSpeed = std::min(targetSpeed, 0.0f);
            state = VehicleState::BRAKING;
        }
    }

    // Junction Logic
    if (isWaitingAtJunction) {
        targetSpeed = 0.0f;
        state = VehicleState::WAITING_JUNCTION;
    } 
    else if (!currentRoadPoints.empty() && currentRoadPointIndex >= currentRoadPoints.size() - 2) {
        float distanceToJunction = Vector2Distance(position, currentRoadPoints.back());
        float junctionApproachDistance = std::max(30.0f, currentSpeed * 2.5f); 
        
        if (distanceToJunction < junctionApproachDistance) {
            float junctionSlowSpeed = 8.0f;
            targetSpeed = std::min(targetSpeed, junctionSlowSpeed);
            if (currentSpeed > targetSpeed) {
                state = VehicleState::BRAKING;
            }
        }
    }

    // Adjust Current Speed
    if (currentSpeed < targetSpeed) {
        currentSpeed += acceleration * deltaTime;
        if (currentSpeed > targetSpeed) currentSpeed = targetSpeed;
    } else if (currentSpeed > targetSpeed) {
        currentSpeed -= deceleration * deltaTime;
        if (currentSpeed < targetSpeed) currentSpeed = targetSpeed;
    }
    currentSpeed = std::clamp(currentSpeed, 0.0f, maxSpeed);

    // Dynamic Turning Speed
    float speedT = (maxSpeed > minSpeed) ? (currentSpeed - minSpeed) / (maxSpeed - minSpeed) : 0.0f;
    float currentTurningSpeed = maxTurningSpeed - speedT * (maxTurningSpeed - minTurningSpeed);

    // Steering and Movement
    Vector2 target = currentRoadPoints[currentRoadPointIndex];
    Vector2 toTarget = Vector2Subtract(target, position);
    float distanceToTarget = Vector2Length(toTarget);

    float arrivalRadius = 15.0f;
    if (distanceToTarget < arrivalRadius && currentRoadPointIndex < currentRoadPoints.size() - 1) {
        if (!isWaitingAtJunction || distanceToTarget < 5.0f) {
             currentRoadPointIndex++;
        }
        target = currentRoadPoints[currentRoadPointIndex];
        toTarget = Vector2Subtract(target, position);
    }

    Vector2 desiredDirection = Vector2Normalize(toTarget);
    float angle = atan2(desiredDirection.y, desiredDirection.x) - atan2(direction.y, direction.x);
    if (angle > PI) angle -= 2 * PI;
    if (angle < -PI) angle += 2 * PI;

    float turn = currentTurningSpeed * deltaTime;
    angle = std::clamp(angle, -turn, turn);

    float currentAngle = atan2(direction.y, direction.x);
    currentAngle += angle;
    direction = { cosf(currentAngle), sinf(currentAngle) };

    velocity = Vector2Scale(direction, currentSpeed);
    position = Vector2Add(position, Vector2Scale(velocity, deltaTime));
}

void Vehicle::draw(bool debug) {
    color = originalColor;

    if (debug) {
        switch (state) {
            case VehicleState::DRIVING:
                color = GREEN;
                break;
            case VehicleState::BRAKING:
                color = RED;
                break;
            case VehicleState::WAITING_JUNCTION:
                color = ORANGE;
                break;
        }
    }

    DrawCircleV(position, size.x / 2, color);
    DrawLineEx(position, Vector2Add(position, Vector2Scale(direction, 15.0f)), 2.0f, BLACK);
}

void Vehicle::selectRandomRoad() {
    if (!map) {
        currentRoadPoints.clear();
        return;
    }

    // Get outgoing roads from current intersection
    const auto& outgoingRoads = map->getOutgoingRoads();
    
    if (targetIntersectionId == -1) {
        // No target intersection, try to find one from current position
        const Road* road = map->getClosestRoad(position);
        if (road) {
            targetIntersectionId = road->toIntersectionId;
        } else {
            currentRoadPoints.clear();
            return;
        }
    }

    // Get roads from current intersection
    auto it = outgoingRoads.find(targetIntersectionId);
    if (it == outgoingRoads.end() || it->second.empty()) {
        // No outgoing roads, try to find any nearby road
        const Road* road = map->getClosestRoad(position);
        if (road) {
            currentRoad = road;
            targetIntersectionId = road->toIntersectionId;
            startFollowingCurrentRoad();
        } else {
            currentRoadPoints.clear();
            state = VehicleState::WAITING_JUNCTION;
            currentSpeed = 0.0f;
        }
        return;
    }

    // Randomly select one of the outgoing roads
    const auto& roads = it->second;
    std::uniform_int_distribution<> dist(0, roads.size() - 1);
    currentRoad = roads[dist(gen)];
    
    // Update target intersection for next junction
    targetIntersectionId = currentRoad->toIntersectionId;
    
    startFollowingCurrentRoad();
}

void Vehicle::startFollowingCurrentRoad() {
    if (!currentRoad) {
        currentRoadPoints.clear();
        currentRoadPointIndex = 0;
        return;
    }

    // Update max speed for this road
    maxSpeed = currentRoad->speedLimit * speedFactor;

    // Update Road Stats
    currentRoad->stats.vehiclesPassed++;

    currentRoadPoints = currentRoad->points;
    currentRoadPointIndex = 0;
}

Vector2 Vehicle::getPosition() const {
    return position;
}

Vector2 Vehicle::getDirection() const {
    return direction;
}

float Vehicle::getSpeed() const {
    return currentSpeed;
}

float Vehicle::getAverageSpeed() const {
    return (speedSamples > 0) ? static_cast<float>(accumulatedSpeed / speedSamples) : 0.0f;
}

Vector2 Vehicle::getSize() const {
    return size;
}

const Road* Vehicle::getRoad() const {
    return currentRoad;
}