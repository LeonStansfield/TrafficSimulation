#include "Engine.hpp"
#include "Vehicle.hpp"
#include "Quadtree.hpp"
#include <random>
#include <time.h>

Engine::Engine(int width, int height, const char* title)
    : screenWidth(width), screenHeight(height), map(nullptr), debug(false){
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
        pathfinder = std::make_unique<Pathfinder>(map.get());
    }
}

void Engine::addObject(std::unique_ptr<Object> object) {
    objects.push_back(std::move(object));
}

void Engine::updateSimulation(float deltaTime) {
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
            v->update(quadtree.get(), deltaTime);
        } else {
            obj->update(deltaTime);
        }
    }
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

        // --- Call the refactored update logic ---
        updateSimulation(GetFrameTime());

        // Draw all objects
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (map) {
            BeginMode2D(camera);
            map->draw(debug);
            for (const auto& obj : objects) {
                obj->draw(debug);
            }
            if (debug) {
                quadtree->draw(camera);
            }
            EndMode2D();
        }

        DrawFPS(10, 10);
        DrawText(debug ? "DEBUG VIEW" : "NORMAL VIEW", 10, 30, 20, DARKGREEN);
        EndDrawing();
    }
}

void Engine::runFast(int ticks) {
    if (!map) {
         std::cerr << "Map is not set. Aborting fast run." << std::endl;
         return;
    }

    // Use a fixed time step, e.g., 60 FPS
    const float FIXED_DELTA_TIME = 1.0f / 60.0f;

    for (int i = 0; i < ticks; ++i) {
        if (i % (ticks / 100) == 0) {
             std::cout << "\rSimulation progress: " << (i * 100 / ticks) << "%" << std::flush;
        }
        updateSimulation(FIXED_DELTA_TIME);
    }
    std::cout << "\rSimulation progress: 100%." << std::endl;
}

void Engine::spawnVehicles(int count) {
    if (!map || !pathfinder) {
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

        addObject(std::make_unique<Vehicle>(pos, Vector2{10, 10}, RED, map.get(), pathfinder.get()));
    }
}