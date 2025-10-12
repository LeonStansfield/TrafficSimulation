#include "Vehicle.hpp"
#include "raylib.h"
#include "raymath.h"
#include "Map.hpp"
#include <cmath>
#include <random>
#include <algorithm>
#include <algorithm>

Vehicle::Vehicle(Vector2 pos, Vector2 sz, Color col, Map* m)
    : position(pos), size(sz), color(col), map(m), currentPathIndex(0), road(nullptr), targetPointIndex(0), gen(std::random_device{}()) {

    std::uniform_real_distribution<> speed_dist(30.0f, 80.0f);     // Speed in meters/sec
    std::uniform_real_distribution<> accel_dist(5.0f, 15.0f);      // m/s^2
    std::uniform_real_distribution<> decel_dist(10.0f, 25.0f);     // m/s^2
    std::uniform_real_distribution<> turning_dist(2.0f, 4.0f);     // Radians per second

    maxSpeed = speed_dist(gen);
    acceleration = accel_dist(gen);
    deceleration = decel_dist(gen);
    turningSpeed = turning_dist(gen);
    currentSpeed = 0.0f;
    velocity = { 0, 0 };
    direction = { 1, 0 }; // Default direction

    findNewPath();
}

void Vehicle::update() {
    if (path.empty() || currentPathIndex >= path.size()) {
        findNewPath();
        if (path.empty() || currentPathIndex >= path.size()) { // Re-check after finding new path
            velocity = {0, 0};
            currentSpeed = 0;
            return;
        }
    }

    Vector2 target = path[currentPathIndex];
    Vector2 toTarget = Vector2Subtract(target, position);
    float distanceToTarget = Vector2Length(toTarget);

    // Dynamic arrival radius, bigger for faster vehicles to prevent overshooting
    float arrivalRadius = std::max(15.0f, currentSpeed * 0.4f);

    if (distanceToTarget < arrivalRadius) {
        currentPathIndex++;
        // If we've reached the end of the current road segment, find a new one
        if (currentPathIndex >= path.size()) {
            findNewPath();
            // If findNewPath fails, exit to avoid accessing invalid memory
            if (path.empty() || currentPathIndex >= path.size()) {
                velocity = {0, 0};
                currentSpeed = 0;
                return;
            }
        }
        // Update target for this frame to avoid a 1-frame lag
        target = path[currentPathIndex];
        toTarget = Vector2Subtract(target, position);
    }

    Vector2 desiredDirection = Vector2Normalize(toTarget);

    // Steering logic
    float angle = atan2(desiredDirection.y, desiredDirection.x) - atan2(direction.y, direction.x);
    if (angle > PI) angle -= 2 * PI;
    if (angle < -PI) angle += 2 * PI;

    float turn = turningSpeed * GetFrameTime();
    angle = std::clamp(angle, -turn, turn);

    float currentAngle = atan2(direction.y, direction.x);
    currentAngle += angle;
    direction = { cos(currentAngle), sin(currentAngle) };

    // Check for sharp turns by comparing current direction with desired direction
    float dot = (direction.x * desiredDirection.x) + (direction.y * desiredDirection.y);
    bool sharpTurnAhead = (dot < 0.8f); // dot < 0.8 is roughly > 36 degree turn

    // Check if we're approaching the final point on our current path
    bool approachingEndOfRoad = (currentPathIndex == path.size() - 1) && (distanceToTarget < 100.0f);

    if (sharpTurnAhead || approachingEndOfRoad) {
        // Slow down for corners or junctions
        currentSpeed -= deceleration * GetFrameTime();
        if (currentSpeed < 15.0f) currentSpeed = 15.0f; // Minimum cornering speed
    } else {
        // Accelerate to max speed
        if (currentSpeed < maxSpeed) {
            currentSpeed += acceleration * GetFrameTime();
            if (currentSpeed > maxSpeed) currentSpeed = maxSpeed;
        }
    }

    velocity = { direction.x * currentSpeed, direction.y * currentSpeed };
    position.x += velocity.x * GetFrameTime();
    position.y += velocity.y * GetFrameTime();
}

void Vehicle::draw() {
    DrawCircleV(position, size.x / 2, color);
    // Draw a line to show direction
    DrawLineEx(position, Vector2Add(position, Vector2Scale(direction, 15.0f)), 2.0f, BLACK);
}

void Vehicle::findNewPath() {
    if (!map) return;

    if (!road) { // First time finding a path
        road = map->getClosestRoad(position);
        if (road && !road->points.empty()) {
            path = road->points;
            float distToStart = Vector2DistanceSqr(position, path.front());
            float distToEnd = Vector2DistanceSqr(position, path.back());
            if (distToEnd < distToStart) {
                std::reverse(path.begin(), path.end());
            }
            currentPathIndex = 0;
        }
    } else { // Already on a road, find the next one
        const Vector2 endOfCurrentPath = path.back();
        const Road* nextRoad = map->getRandomConnectedRoad(*road, endOfCurrentPath, gen);

        if (nextRoad && !nextRoad->points.empty()) {
            road = nextRoad;
            path = road->points;

            // Check if we need to reverse the new path
            if (!Vector2Equals(path.front(), endOfCurrentPath)) {
                // The new path must end where the old one ended. Reverse it.
                std::reverse(path.begin(), path.end());
            }
            currentPathIndex = 0;
        } else if (!path.empty()) {
            // This is a DEAD END. Turn around.
            std::reverse(path.begin(), path.end());
            currentPathIndex = 0;
        }
    }

    if (path.empty() || currentPathIndex >= path.size()) {
        road = nullptr;
        path.clear();
    }
}