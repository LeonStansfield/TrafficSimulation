#include "Engine.hpp"

Engine::Engine(int width, int height, const char* title)
    : screenWidth(width), screenHeight(height), map(nullptr) {
    InitWindow(screenWidth, screenHeight, title);
    SetTargetFPS(60);
}

Engine::~Engine() {
    CloseWindow();
}

// Add this new method
void Engine::setMap(std::unique_ptr<Map> newMap) {
    map = std::move(newMap);
}

void Engine::addObject(std::unique_ptr<Object> object) {
    objects.push_back(std::move(object));
}

void Engine::run() {
    while (!WindowShouldClose()) {
        // Update all objects
        for (const auto& obj : objects) {
            obj->update();
        }

        // Draw all objects
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Draw the map if it exists
        if (map) {
            map->draw();
        }

        for (const auto& obj : objects) {
            obj->draw();
        }

        DrawFPS(10, 10);
        EndDrawing();
    }
}