#include "Vehicle.hpp"
#include "raylib.h"
#include "raymath.h"
#include "Map.hpp"
#include "Quadtree.hpp"
#include <cmath>
#include <random>
#include <algorithm>

// Helper function to calculate the signed angle between two vectors
static float GetRelativeAngle(Vector2 v1, Vector2 v2) {
    float dot = v1.x * v2.x + v1.y * v2.y;
    float det = v1.x * v2.y - v1.y * v2.x;
    return atan2(det, dot);
}

Vehicle::Vehicle(Vector2 pos, Vector2 sz, Color col, Map* m)
    : position(pos), size(sz), color(col), map(m), currentPathIndex(0), road(nullptr), targetPointIndex(0), gen(std::random_device{}()) {

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

    // Check for nearby vehicles and adjust speed here
    
    bool approachingEndOfRoad = (path.size() - currentPathIndex < 3) && (Vector2Distance(position, path.back()) < 80.0f);
    if (approachingEndOfRoad) {
        targetSpeed = std::min(targetSpeed, 10.0f);
    }

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
        currentPathIndex++;
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

            if (!Vector2Equals(path.front(), endOfCurrentPath)) {
                std::reverse(path.begin(), path.end());
            }
            currentPathIndex = 0;
        } else if (!path.empty()) {
            // This is a DEAD END. Turn around.
            std::reverse(path.begin(), path.end());
            currentPathIndex = 0;
        }
    }

    if (path.empty()) {
        road = nullptr;
    }
}

Vector2 Vehicle::getPosition() const {
    return position;
}