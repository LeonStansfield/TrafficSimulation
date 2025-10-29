#include "Vehicle.hpp"
#include "raylib.h"
#include "raymath.h"
#include "Map.hpp"
#include "Quadtree.hpp"
#include <cmath>
#include <random>
#include <algorithm>
#include <cfloat>

// Helper function to calculate the signed angle between two vectors
static float GetRelativeAngle(Vector2 v1, Vector2 v2) {
    float dot = v1.x * v2.x + v1.y * v2.y;
    float det = v1.x * v2.y - v1.y * v2.x;
    return atan2(det, dot);
}

Vehicle::Vehicle(Vector2 pos, Vector2 sz, Color col, Map* m)
    : position(pos), size(sz), color(col), originalColor(col), map(m), currentPathIndex(0), road(nullptr), targetPointIndex(0), 
      isWaitingAtJunction(false), state(VehicleState::DRIVING), gen(std::random_device{}()) {

    std::uniform_real_distribution<> speed_dist(13.0f, 31.0f); // Speed range in m/s (approx. 30-70 mph)
    std::uniform_real_distribution<> accel_dist(3.0f, 8.0f); // m/s^2
    std::uniform_real_distribution<> decel_dist(5.0f, 15.0f); // m/s^2
    std::uniform_real_distribution<> turning_dist(3.0f, 5.0f); // Radians per second

    maxSpeed = speed_dist(gen);
    minSpeed = 4.5f; // Approx. 10 mph
    acceleration = accel_dist(gen);
    deceleration = decel_dist(gen);

    // Speed-Dependent Turning
    maxTurningSpeed = turning_dist(gen);
    minTurningSpeed = 1.5f;

    currentSpeed = 0.0f;
    velocity = { 0, 0 };
    direction = { 1, 0 }; // Default direction

    findNewPath();
}

void Vehicle::update() {
    // This is a placeholder to satisfy the override, but the main update logic is in update(Quadtree*)
}

void Vehicle::update(Quadtree* quadtree) {
    if (path.empty() || currentPathIndex >= path.size() - 1) {
        findNewPath();
        if (path.empty()) { // If still no path, stop the vehicle
            velocity = {0, 0};
            currentSpeed = 0;
            return;
        }
        // If findNewPath() succeeded but path is *still* too small, return
        if (path.size() < 2) {
             velocity = {0, 0};
             currentSpeed = 0;
             return;
        }
    }

    // Look-Ahead for Curvature Detection
    float lookAheadDistance = std::max(50.0f, currentSpeed * 3.0f);
    float totalAngleChange = 0.0f;
    float distanceTraveled = 0.0f;

    for (size_t i = currentPathIndex; i < path.size() - 2 && distanceTraveled < lookAheadDistance; ++i) {
        Vector2 p1 = path[i];
        Vector2 p2 = path[i+1];
        Vector2 p3 = path[i+2];

        Vector2 segment1 = Vector2Normalize(Vector2Subtract(p2, p1));
        Vector2 segment2 = Vector2Normalize(Vector2Subtract(p3, p2));

        totalAngleChange += std::abs(GetRelativeAngle(segment1, segment2));
        distanceTraveled += Vector2Distance(p1, p2);
    }

    // Determine Target Speed Based on Curvature
    float curvature = (distanceTraveled > 0) ? totalAngleChange / distanceTraveled : 0;
    float curvatureFactor = std::clamp(curvature * 10.0f, 0.0f, 1.0f);

    float targetSpeed = maxSpeed - (maxSpeed - minSpeed) * curvatureFactor;

    // State Setting
    // Start by assuming we are driving, or braking for curvature
    if (currentSpeed > targetSpeed) {
        state = VehicleState::BRAKING;
    } else {
        state = VehicleState::DRIVING;
    }

    // Begin Collision Avoidance
    float minGap = 8.0f; // Min distance between cars
    float desiredTimeGap = 1.5f; // 1.5 seconds of travel time
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

        // Check if the other vehicle is strongly in front of us
        if (dot > 0.9 && distSqr < closestDistSqr) {
            
            // Check if they are also going (roughly) the same direction as us.
            Vector2 otherVehicleDirection = other->getDirection();
            float directionDot = Vector2DotProduct(direction, otherVehicleDirection);

            // If directionDot > 0.5, they are going the same direction.
            if (directionDot > 0.5f) {
                closestDistSqr = distSqr;
                leadVehicle = other;
            }
        }
    }

    if (leadVehicle) {
        // Calculate the gap from our front bumper to their-rear bumper
        float distance = sqrt(closestDistSqr) - (leadVehicle->getSize().x / 2.0f) - (size.x / 2.0f);
        float safeDistance = currentSpeed * desiredTimeGap + minGap;

        if (distance < safeDistance) {
            // Adjust target speed based on lead vehicle
            float brakeSpeed = leadVehicle->getSpeed() + (distance - safeDistance) / desiredTimeGap;
            targetSpeed = std::min(targetSpeed, std::max(0.0f, brakeSpeed));
            state = VehicleState::BRAKING; // Override state
        }
        
        if (distance < minGap) {
            targetSpeed = std::min(targetSpeed, 0.0f); // Hard brake
            state = VehicleState::BRAKING; // Override state
        }
    }

    // Junction logic will go here but ignore for now

    // Adjust Current Speed
    if (currentSpeed < targetSpeed) {
        currentSpeed += acceleration * GetFrameTime();
        if (currentSpeed > targetSpeed) currentSpeed = targetSpeed;
    } else if (currentSpeed > targetSpeed) {
        currentSpeed -= deceleration * GetFrameTime();
        if (currentSpeed < targetSpeed) currentSpeed = targetSpeed;
    }
    currentSpeed = std::clamp(currentSpeed, 0.0f, maxSpeed);

    // Dynamic Turning Speed based on Vehicle Speed
    float speedT = (maxSpeed > minSpeed) ? (currentSpeed - minSpeed) / (maxSpeed - minSpeed) : 0.0f;
    float currentTurningSpeed = maxTurningSpeed - speedT * (maxTurningSpeed - minTurningSpeed);

    // Steering and Movement
    Vector2 target = path[currentPathIndex];
    Vector2 toTarget = Vector2Subtract(target, position);
    float distanceToTarget = Vector2Length(toTarget);

    float arrivalRadius = std::max(15.0f, currentSpeed * 0.5f);
    if (distanceToTarget < arrivalRadius && currentPathIndex < path.size() - 1) {
        // Only advance path index if we are NOT waiting at a junction
        if (!isWaitingAtJunction || distanceToTarget < 5.0f) {
             currentPathIndex++;
        }
        target = path[currentPathIndex];
        toTarget = Vector2Subtract(target, position);
    }

    Vector2 desiredDirection = Vector2Normalize(toTarget);
    float angle = atan2(desiredDirection.y, desiredDirection.x) - atan2(direction.y, direction.x);
    if (angle > PI) angle -= 2 * PI;
    if (angle < -PI) angle += 2 * PI;

    float turn = currentTurningSpeed * GetFrameTime();
    angle = std::clamp(angle, -turn, turn);

    float currentAngle = atan2(direction.y, direction.x);
    currentAngle += angle;
    direction = { cosf(currentAngle), sinf(currentAngle) };

    velocity = Vector2Scale(direction, currentSpeed);
    position = Vector2Add(position, Vector2Scale(velocity, GetFrameTime()));
}

void Vehicle::draw(bool debug) {
    color = originalColor; // Default color

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
    // Draw a line to show direction
    DrawLineEx(position, Vector2Add(position, Vector2Scale(direction, 15.0f)), 2.0f, BLACK);
}

void Vehicle::findNewPath() {
    if (!map) return;
    
    isWaitingAtJunction = false; // Reset waiting state when finding new path

    if (!road) { // First time finding a path
        road = map->getClosestRoad(position);
        if (road && !road->points.empty()) {
            path = road->points;
            
            // Find the closest point on this new path to snap to
            float minDistSq = FLT_MAX;
            int closestIndex = 0;
            for(int i = 0; i < path.size(); ++i) {
                float distSq = Vector2DistanceSqr(position, path[i]);
                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    closestIndex = i;
                }
            }
            // Set the path index to the closest point found.
            // The vehicle will now drive from this point to the end.
            currentPathIndex = closestIndex;
            // Snap the vehicle to the path
            if (currentPathIndex < path.size()) {
                 position = path[currentPathIndex]; 
            }
        }
    } else { // Already on a road, find the next one
        long endOfRoadIntersectionId = road->toIntersectionId;
        const Road* nextRoad = map->getRandomOutgoingRoad(endOfRoadIntersectionId, gen);

        if (nextRoad && !nextRoad->points.empty()) {
            road = nextRoad;
            path = road->points;
            // We ALWAYS start at the beginning of the new road
            currentPathIndex = 0;
        } else {
            // This is a DEAD END. Clear the path.
            path.clear();
            road = nullptr;
        }
    }

    if (path.empty()) {
        road = nullptr;
    }
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

Vector2 Vehicle::getSize() const {
    return size;
}