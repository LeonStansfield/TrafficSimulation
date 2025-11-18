#include "Vehicle.hpp"
#include "raylib.h"
#include "raymath.h"
#include "Map.hpp"
#include "Quadtree.hpp"
#include <cmath>
#include <random>
#include <algorithm>
#include <cfloat>

static float GetRelativeAngle(Vector2 v1, Vector2 v2) {
    float dot = v1.x * v2.x + v1.y * v2.y;
    float det = v1.x * v2.y - v1.y * v2.x;
    return atan2(det, dot);
}

Vehicle::Vehicle(Vector2 pos, Vector2 sz, Color col, Map* m, Pathfinder* pf)
    : position(pos), size(sz), color(col), originalColor(col), map(m), pathfinder(pf),
      currentPathRoadIndex(0), currentRoadPointIndex(0), 
      isWaitingAtJunction(false), state(VehicleState::DRIVING), gen(std::random_device{}()),
      destinationIntersectionId(-1),
      accumulatedSpeed(0.0), speedSamples(0), timeActive(0.0f)
{

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

    // Snap to closest road and find first path
    const Road* startRoad = map->getClosestRoad(position);
    if (startRoad && !startRoad->points.empty()) {
        // Find the closest point on this new path to snap to
        float minDistSq = FLT_MAX;
        int closestIndex = 0;
        for(int i = 0; i < startRoad->points.size(); ++i) {
            float distSq = Vector2DistanceSqr(position, startRoad->points[i]);
            if (distSq < minDistSq) {
                minDistSq = distSq;
                closestIndex = i;
            }
        }
        // Snap the vehicle to the path
        position = startRoad->points[closestIndex];

        // Set up a path
        requestNewPath();

    } else {
        // No road found, become inert
        currentSpeed = 0.0f;
        velocity = { 0, 0 };
        state = VehicleState::WAITING_JUNCTION;
    }
}

void Vehicle::update(float deltaTime) {
    // This is a placeholder to satisfy the override
    updateStats(deltaTime);
}

void Vehicle::updateStats(float deltaTime) {
    // Update internal vehicle stats
    timeActive += deltaTime;
    accumulatedSpeed += currentSpeed;
    speedSamples++;

    // Update road stats (if moving)
    if (currentSpeed > 0.1f) {
        const Road* currentRoad = getRoad();
        if (currentRoad) {
            currentRoad->stats.accumulatedSpeed += currentSpeed;
            currentRoad->stats.speedSamples++;
        }
    }
}

void Vehicle::update(Quadtree* quadtree, float deltaTime) {
    updateStats(deltaTime);

    // Path & Junction Logic
    // Check if we are at the end of the current road
    if (currentRoadPoints.empty() || currentRoadPointIndex >= currentRoadPoints.size() - 1) {
        
        if (!isWaitingAtJunction) {
            isWaitingAtJunction = true;
            state = VehicleState::WAITING_JUNCTION;
        }

        // We are at a junction, decide what to do
        if (currentPath.empty() || currentPathRoadIndex >= currentPath.size() - 1) {
            // We are at the end of our path (destination)
            // Mark the intersection as visited
            if (destinationIntersectionId != -1) {
                const auto& intersections = map->getIntersections();
                if (intersections.find(destinationIntersectionId) != intersections.end()) {
                    intersections.at(destinationIntersectionId).stats.vehiclesVisited++;
                }
            }

            requestNewPath();
            if (currentRoadPoints.empty()) { // No new path found
                velocity = {0, 0}; currentSpeed = 0; return;
            }
            return; // New path is set, process next frame
        
        } else {
            // We are at an intermediate junction, check if safe to proceed
            const Road* nextRoadToJoin = currentPath[currentPathRoadIndex + 1];

            bool safeToProceed = true;
            float checkRadius = 30.0f; 
            Vector2 junctionPos = currentRoadPoints.back();
            
            Rectangle queryBox = { 
                junctionPos.x - checkRadius, 
                junctionPos.y - checkRadius, 
                checkRadius * 2.0f, 
                checkRadius * 2.0f 
            };
            auto nearby = quadtree->query(queryBox);

            for (Vehicle* other : nearby) {
                if (other == this) continue;
                if (other->getRoad() == nextRoadToJoin && other->getSpeed() > 1.0f) {
                    safeToProceed = false;
                    break;
                }
            }

            if (safeToProceed) {
                isWaitingAtJunction = false;
                currentPathRoadIndex++; // Advance to the next road in our path
                startFollowingCurrentRoad();
                
                if (currentRoadPoints.empty()) { 
                    velocity = {0, 0}; currentSpeed = 0; return;
                }
            }
        }

    } else if (currentRoadPoints.empty()) {
        // Path is empty
        requestNewPath();
        if (currentRoadPoints.empty()) { // If still no path, stop
            velocity = {0, 0}; currentSpeed = 0; return;
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

    // Determine Target Speed Based on Curvature
    float curvature = (distanceTraveled > 0) ? totalAngleChange / distanceTraveled : 0;
    float curvatureFactor = std::clamp(curvature * 10.0f, 0.0f, 1.0f);
    float targetSpeed = maxSpeed - (maxSpeed - minSpeed) * curvatureFactor;

    // State Setting
    if (currentSpeed > targetSpeed) {
        state = VehicleState::BRAKING;
    } else {
        state = VehicleState::DRIVING;
    }

    // Begin Collision Avoidance
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
        // We are approaching the junction
        float distanceToJunction = Vector2Distance(position, currentRoadPoints.back());
        
        float junctionApproachDistance = std::max(30.0f, currentSpeed * 2.5f); 
        
        if (distanceToJunction < junctionApproachDistance) {
            float junctionSlowSpeed = 8.0f; // ~18 mph
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

    // Dynamic Turning Speed based on Vehicle Speed
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
    // Draw a line to show direction
    DrawLineEx(position, Vector2Add(position, Vector2Scale(direction, 15.0f)), 2.0f, BLACK);
}

void Vehicle::startFollowingCurrentRoad() {
    if (currentPath.empty() || currentPathRoadIndex >= currentPath.size()) {
        currentRoadPoints.clear();
        currentRoadPointIndex = 0;
        requestNewPath(); // Path complete, get a new one
        return;
    }

    const Road* roadToFollow = currentPath[currentPathRoadIndex];
    
    // Update Road Stats
    roadToFollow->stats.vehiclesPassed++;

    currentRoadPoints = roadToFollow->points;
    currentRoadPointIndex = 0; // Always start at the beginning of the new road
}

void Vehicle::requestNewPath() {
    if (!map || !pathfinder) {
        currentRoadPoints.clear();
        currentPath.clear();
        return;
    }

    isWaitingAtJunction = false;
    currentPath.clear();
    currentRoadPoints.clear();
    currentPathRoadIndex = 0;
    currentRoadPointIndex = 0;

    // Get start intersection
    const Road* currentRoad = map->getClosestRoad(position);
    if (!currentRoad) {
        state = VehicleState::WAITING_JUNCTION;
        currentSpeed = 0.0f;
        return;
    }
    long startIntersectionId = currentRoad->toIntersectionId;
    if (startIntersectionId == 0) startIntersectionId = currentRoad->fromIntersectionId;

    // Get new random destination
    destinationIntersectionId = -1;
    while (destinationIntersectionId == -1 || destinationIntersectionId == startIntersectionId) {
        destinationIntersectionId = map->getRandomIntersectionId(gen);
    }
    
    currentPath = pathfinder->findPath(startIntersectionId, destinationIntersectionId);

    if (!currentPath.empty()) {
        startFollowingCurrentRoad();
    } else {
        // A* path was NOT found (disconnected graph).
        // Attempt to find any local outgoing road from the start intersection
        const auto& outgoingRoads = map->getOutgoingRoads();
        const Road* nextRoad = nullptr;
        if (outgoingRoads.find(startIntersectionId) != outgoingRoads.end()) {
            const auto& roadsFromHere = outgoingRoads.at(startIntersectionId);
            if (!roadsFromHere.empty()) {
                nextRoad = roadsFromHere[0]; // Pick the first available road
            }
        }

        if (nextRoad) {
            currentPath.push_back(nextRoad);
            startFollowingCurrentRoad();
        } else {
            // Dead end
            state = VehicleState::WAITING_JUNCTION;
            currentSpeed = 0.0f;
        }
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

float Vehicle::getAverageSpeed() const {
    return (speedSamples > 0) ? static_cast<float>(accumulatedSpeed / speedSamples) : 0.0f;
}

Vector2 Vehicle::getSize() const {
    return size;
}

const Road* Vehicle::getRoad() const {
    if (currentPath.empty() || currentPathRoadIndex >= currentPath.size()) {
        return nullptr;
    }
    return currentPath[currentPathRoadIndex];
}

const std::vector<const Road*>& Vehicle::getPath() const {
    return currentPath;
}

int Vehicle::getPathIndex() const {
    return currentPathRoadIndex;
}