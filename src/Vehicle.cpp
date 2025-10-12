#include "Vehicle.hpp"
#include <cmath>

Vehicle::Vehicle(Vector2 pos, Vector2 sz, float spd, Color col, const Road* r)
    : position(pos), size(sz), speed(spd), color(col), road(r), targetPointIndex(1) {
    velocity = { 0, 0 };
}

void Vehicle::update() {
    // Move the vehicle
    // position.x += velocity.x;
    // position.y += velocity.y;

    // // A simple reset when the vehicle goes off-screen
    // if (position.x > GetScreenWidth()) {
    //     position.x = -size.x;
    // }
}

void Vehicle::draw() {
    // Draw vehicle as small circle for simplicity
    DrawCircleV(position, size.x / 2, color);
}