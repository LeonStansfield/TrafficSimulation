#include "Vehicle.hpp"
#include "Map.hpp"
#include <raymath.h>

Vehicle::Vehicle(const Map* map, Vector2 sz, float spd, Color col)
    : map(map), size(sz), speed(spd), color(col), currentPathIndex(0) {
    findNewPath();
}

void Vehicle::findNewPath() {
    if (map) {
        path = map->getRandomRoad();
        if (!path.empty()) {
            position = map->convertLatLonToWorld(path[0]);
            currentPathIndex = 1;
        }
    }
}

void Vehicle::update() {
    if (path.size() < 2 || currentPathIndex >= path.size()) {
        findNewPath();
        return;
    }

    Vector2 targetPosition = map->convertLatLonToWorld(path[currentPathIndex]);
    Vector2 direction = Vector2Subtract(targetPosition, position);
    direction = Vector2Normalize(direction);

    velocity = { direction.x * speed, direction.y * speed };

    position.x += velocity.x;
    position.y += velocity.y;

    if (Vector2Distance(position, targetPosition) < 5.0f) {
        currentPathIndex++;
        if (currentPathIndex >= path.size()) {
            findNewPath();
        }
    }
}

void Vehicle::draw() {
    DrawRectangleV(position, size, color);
}