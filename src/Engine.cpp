#include "Engine.hpp"
#include "Vehicle.hpp"
#include "Quadtree.hpp"
#include <random>
#include <time.h>

Engine::Engine(int width, int height, const char* title)
    : screenWidth(width), screenHeight(height), map(nullptr), debug(false) {
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

        quadtree = std::make_unique<Quadtree>(Rectangle{0, 0, map->getWorldWidth(), map->getWorldHeight()}, 4);
    }
}

void Engine::addObject(std::unique_ptr<Object> object) {
    objects.push_back(std::move(object));
}

void Engine::run() {
    while (!WindowShouldClose()) {
        // Camera controls
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

        if (IsKeyPressed(KEY_Q)) {
            debug = !debug;
        }

        // Rebuild the quadtree
        quadtree->clear();
        for (const auto& obj : objects) {
            Vehicle* v = dynamic_cast<Vehicle*>(obj.get());
            if (v) {
                quadtree->insert(v);
            }
        }

        // Update all objects
        for (const auto& obj : objects) {
            Vehicle* v = dynamic_cast<Vehicle*>(obj.get());
            if (v) {
                v->update(quadtree.get());
            } else {
                obj->update();
            }
        }

        // Draw all objects
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (map) {
            BeginMode2D(camera);
            map->draw();
            for (const auto& obj : objects) {
                obj->draw();
            }
            if (debug) {
                quadtree->draw(camera);
            }
            EndMode2D();
        }

        DrawFPS(10, 10);
        EndDrawing();
    }
}

void Engine::spawnVehicles(int count) {
    if (!map) {
        return;
    }

    const auto& roads = map->getRoads();
    if (roads.empty()) {
        return;
    }

    std::mt19937 rng(time(nullptr));
    std::uniform_int_distribution<int> road_dist(0, roads.size() - 1);

    for (int i = 0; i < count; ++i) {
        const Road& random_road = roads[road_dist(rng)];
        if (random_road.points.size() < 2) continue;

        std::uniform_int_distribution<int> segment_dist(0, random_road.points.size() - 2);
        int segment_index = segment_dist(rng);

        Vector2 p1 = random_road.points[segment_index];
        Vector2 p2 = random_road.points[segment_index + 1];

        std::uniform_real_distribution<float> pos_dist(0.0f, 1.0f);
        float t = pos_dist(rng);
        Vector2 pos = { p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y) };

        addObject(std::make_unique<Vehicle>(pos, Vector2{10, 10}, RED, map.get()));
    }
}