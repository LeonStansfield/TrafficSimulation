#include "raylib.h"
#include <vector>
#include <memory>

/**
 * @class Object
 * @brief An abstract base class for all game objects.
 */
class Object {
public:
    virtual ~Object() = default;

    // Virtual functions that all derived classes must implement.
    virtual void update() = 0;
    virtual void draw() = 0;
};

/**
 * @class BouncingBall
 * @brief A simple example object that inherits from Object.
 */
class BouncingBall : public Object {
private:
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;

public:
    BouncingBall(Vector2 pos, Vector2 vel, float r, Color c)
        : position(pos), velocity(vel), radius(r), color(c) {}

    void update() override {
        position.x += velocity.x;
        position.y += velocity.y;

        if ((position.x + radius) >= GetScreenWidth() || (position.x - radius) <= 0) {
            velocity.x *= -1;
        }
        if ((position.y + radius) >= GetScreenHeight() || (position.y - radius) <= 0) {
            velocity.y *= -1;
        }
    }

    void draw() override {
        DrawCircleV(position, radius, color);
    }
};

/**
 * @class Engine
 * @brief Manages the main simulation loop and holds all objects.
 */
class Engine {
private:
    std::vector<std::unique_ptr<Object>> objects;
    int screenWidth;
    int screenHeight;

public:
    Engine(int width, int height, const char* title)
        : screenWidth(width), screenHeight(height) {
        InitWindow(screenWidth, screenHeight, title);
        SetTargetFPS(60);
    }

    ~Engine() {
        CloseWindow();
    }

    void addObject(std::unique_ptr<Object> object) {
        objects.push_back(std::move(object));
    }

    // Main loop
    void run() {
        while (!WindowShouldClose()) {
            // Update all objects
            for (const auto& obj : objects) {
                obj->update();
            }

            // Draw all objects
            BeginDrawing();
            ClearBackground(DARKGRAY);

            for (const auto& obj : objects) {
                obj->draw();
            }

            DrawFPS(10, 10);

            EndDrawing();
        }
    }
};

int main() {
    // Create engine
    Engine engine(800, 600, "Traffic Simulation Engine");

    // Create game objects and add them to the engine
    engine.addObject(std::make_unique<BouncingBall>(
        Vector2{100, 100}, Vector2{3.0f, 2.5f}, 20.0f, BLUE
    ));
    engine.addObject(std::make_unique<BouncingBall>(
        Vector2{400, 200}, Vector2{-2.0f, 1.5f}, 15.0f, RED
    ));
    engine.addObject(std::make_unique<BouncingBall>(
        Vector2{600, 350}, Vector2{1.0f, -3.5f}, 25.0f, GREEN
    ));

    // Run the main loop
    engine.run();

    return 0;
}