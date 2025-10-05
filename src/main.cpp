#include "Engine.hpp"
#include "Ball.hpp"
#include <memory>

int main() {
    // Create the engine
    Engine engine(800, 600, "Traffic Simulation Engine");

    // Create game objects and add them to the engine
    engine.addObject(std::make_unique<Ball>(
        Vector2{100, 100}, Vector2{3.0f, 2.5f}, 20.0f, BLUE
    ));
    engine.addObject(std::make_unique<Ball>(
        Vector2{400, 200}, Vector2{-2.0f, 1.5f}, 15.0f, RED
    ));
    engine.addObject(std::make_unique<Ball>(
        Vector2{600, 350}, Vector2{1.0f, -3.5f}, 25.0f, GREEN
    ));

    // Run the main loop
    engine.run();

    return 0;
}