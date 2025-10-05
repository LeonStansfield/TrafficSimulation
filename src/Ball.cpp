#include "Ball.hpp"

Ball::Ball(Vector2 pos, Vector2 vel, float r, Color c)
    : position(pos), velocity(vel), radius(r), color(c) {}

void Ball::update() {
    position.x += velocity.x;
    position.y += velocity.y;

    // Simple wall bouncing logic
    if ((position.x + radius) >= GetScreenWidth() || (position.x - radius) <= 0) {
        velocity.x *= -1;
    }
    if ((position.y + radius) >= GetScreenHeight() || (position.y - radius) <= 0) {
        velocity.y *= -1;
    }
}

void Ball::draw() {
    DrawCircleV(position, radius, color);
}