#include "Engine.hpp"

Engine::Engine(int width, int height, const char* title)
    : screenWidth(width), screenHeight(height), map(nullptr) {
    InitWindow(screenWidth, screenHeight, title);
    SetTargetFPS(60);

    camera = { 0 };
    camera.zoom = 1.0f;
}

Engine::~Engine() {
    CloseWindow();
}

void Engine::setMap(std::unique_ptr<Map> newMap) {
    map = std::move(newMap);
    if (map) {
        camera.target = { map->getWorldWidth() / 2.0f, map->getWorldHeight() / 2.0f };
        camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };

        float scaleX = static_cast<float>(screenWidth) / map->getWorldWidth();
        float scaleY = static_cast<float>(screenHeight) / map->getWorldHeight();
        camera.zoom = std::min(scaleX, scaleY);
    }
}

void Engine::addObject(std::unique_ptr<Object> object) {
    objects.push_back(std::move(object));
}

void Engine::run() {
    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) camera.target.y -= 10.0f / camera.zoom;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) camera.target.y += 10.0f / camera.zoom;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) camera.target.x -= 10.0f / camera.zoom;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) camera.target.x += 10.0f / camera.zoom;

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.offset = GetMousePosition();
            camera.target = mouseWorldPos;
            const float zoomIncrement = 0.125f;
            camera.zoom += (wheel * zoomIncrement);
            if (camera.zoom < zoomIncrement) camera.zoom = zoomIncrement;
        }

        if (IsKeyPressed(KEY_R)) {
            if (map) {
                camera.target = { map->getWorldWidth() / 2.0f, map->getWorldHeight() / 2.0f };
                camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
                float scaleX = static_cast<float>(screenWidth) / map->getWorldWidth();
                float scaleY = static_cast<float>(screenHeight) / map->getWorldHeight();
                camera.zoom = std::min(scaleX, scaleY);
            }
        }

        for (const auto& obj : objects) {
            obj->update();
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (map) {
            BeginMode2D(camera);
            map->draw();
            for (const auto& obj : objects) {
                obj->draw();
            }
            EndMode2D();
        }

        DrawFPS(10, 10);
        EndDrawing();
    }
}