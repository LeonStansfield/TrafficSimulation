#include "Vehicle.hpp"

Vehicle::Vehicle(Vector2 pos, Vector2 sz, float spd, Color col)
    : position(pos), size(sz), speed(spd), color(col) {
    // Initially, the vehicle will move to the right
    velocity = { speed, speed };
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
    DrawRectangleV(position, size, color);
}