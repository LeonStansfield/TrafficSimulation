#include "Engine.hpp"
#include "Vehicle.hpp"
#include "Quadtree.hpp"
#include <random>
#include <time.h>
#include <string>

Engine::Engine(int width, int height, const char* title)
    : screenWidth(width), screenHeight(height), map(nullptr), debug(false),
      selectionType(SelectionType::NONE), selectedVehicle(nullptr), 
      selectedIntersection(nullptr), selectedRoad(nullptr)
{
    InitWindow(screenWidth, screenHeight, title);
    SetTargetFPS(60);
    
    font = LoadFontEx("../data/Consolas-Regular.ttf", 32, 0, 0);
    
    if (font.texture.id == 0) {
        // Fallback if file not found
        font = GetFontDefault();
    } else {
        // Bilinear filtering makes scaling look better
        SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    }

    camera = { 0 };
    camera.zoom = 1.0f;
}

Engine::~Engine() {
    if (font.texture.id != GetFontDefault().texture.id) {
        UnloadFont(font);
    }
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

        // Interaction / Selection Logic
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            
            selectionType = SelectionType::NONE;
            selectedVehicle = nullptr;
            selectedIntersection = nullptr;
            selectedRoad = nullptr;

            // 1. Try to select a vehicle
            float searchRadius = 20.0f; // tolerance for clicking
            Rectangle queryBox = { 
                mouseWorldPos.x - searchRadius, mouseWorldPos.y - searchRadius, 
                searchRadius * 2, searchRadius * 2 
            };
            auto nearby = quadtree->query(queryBox);
            float minDist = searchRadius;
            
            for (auto* v : nearby) {
                float dist = Vector2Distance(v->getPosition(), mouseWorldPos);
                if (dist < minDist) {
                    minDist = dist;
                    selectedVehicle = v;
                    selectionType = SelectionType::VEHICLE;
                }
            }

            // 2. Try to select an intersection
            if (selectionType == SelectionType::NONE && map) {
                const Intersection* closestInt = map->getClosestIntersection(mouseWorldPos);
                if (closestInt) {
                    float dist = Vector2Distance(closestInt->position, mouseWorldPos);
                    if (dist < 20.0f) { // Tolerance
                        selectedIntersection = closestInt;
                        selectionType = SelectionType::INTERSECTION;
                    }
                }
            }

            // 3. Try to select a road
            if (selectionType == SelectionType::NONE && map) {
                const Road* closestRoad = map->getClosestRoad(mouseWorldPos);
                if (closestRoad && !closestRoad->points.empty()) {
                    // Basic proximity check
                    bool inRange = false;
                    for (const auto& p : closestRoad->points) {
                        if (Vector2Distance(p, mouseWorldPos) < 50.0f) {
                            inRange = true; 
                            break;
                        }
                    }

                    if (inRange) {
                        selectedRoad = closestRoad;
                        selectionType = SelectionType::ROAD;
                    }
                }
            }
        }

        // --- Update logic ---
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
            
            // Highlight selection
            Color highlightColor = Fade(SKYBLUE, 0.5f);

            if (selectionType == SelectionType::ROAD && selectedRoad) {
                for (size_t i = 0; i < selectedRoad->points.size() - 1; ++i) {
                    DrawLineEx(selectedRoad->points[i], selectedRoad->points[i+1], 4.0f, highlightColor);
                }
            } else if (selectionType == SelectionType::INTERSECTION && selectedIntersection) {
                DrawCircleV(selectedIntersection->position, 8.0f, highlightColor);
            } else if (selectionType == SelectionType::VEHICLE && selectedVehicle) {
                 const Vehicle* v = dynamic_cast<const Vehicle*>(selectedVehicle);
                 if (v) DrawCircleV(v->getPosition(), v->getSize().x * 0.7f, highlightColor);
            }

            if (debug) {
                quadtree->draw(camera);
            }
            EndMode2D();
        }

        // Draw UI overlay
        drawUI();

        EndDrawing();
    }
}

void Engine::drawUI() {
    // Define style constants
    const int boxX = 20;
    const int boxY = 20;
    const int width = 260;
    const int padding = 8;
    const int lineSpacing = 24;
    const float fontSize = 16.0f;

    Color panelColor = { 20, 25, 30, 230 }; // Deep dark blue-gray
    Color borderColor = { 60, 70, 80, 255 }; // Subtle border
    Color labelColor = GRAY;
    Color valueColor = RAYWHITE;
    Color accentColor = SKYBLUE;

    // Calculate dynamic height
    int contentHeight = 0;
    
    // System Status section height
    contentHeight += 10 + 20; // Label + Value

    // Selection section height
    if (selectionType != SelectionType::NONE) {
        contentHeight += 15; // Spacer
        contentHeight += 10 + 20; // Header
        contentHeight += lineSpacing * 3; // Lines
    }

    int totalHeight = padding * 2 + contentHeight;

    // Draw Panel
    DrawRectangle(boxX, boxY, width, totalHeight, panelColor);
    DrawRectangleLines(boxX, boxY, width, totalHeight, borderColor);
    
    float cursorX = (float)boxX + padding;
    float cursorY = (float)boxY + padding;

    // Helper lambda for using custom font
    auto drawText = [&](const char* text, float x, float y, float size, Color col) {
        DrawTextEx(font, text, {x, y}, size, 1.0f, col);
    };

    // --- System Status ---
    drawText("View mode", cursorX, cursorY, fontSize, labelColor);
    cursorY += 14;
    drawText(debug ? "DEBUG" : "NORMAL", cursorX, cursorY, fontSize, debug ? ORANGE : accentColor);
    cursorY += 28;

    // --- Selection Info ---
    if (selectionType != SelectionType::NONE) {
        DrawLine((int)cursorX, (int)cursorY, boxX + width - padding, (int)cursorY, borderColor);
        cursorY += 12;

        if (selectionType == SelectionType::VEHICLE && selectedVehicle) {
            const Vehicle* v = dynamic_cast<const Vehicle*>(selectedVehicle);
            drawText("SELECTED: VEHICLE", cursorX, cursorY, fontSize, labelColor); cursorY += 18;
            
            drawText("Speed:", cursorX, cursorY, fontSize, labelColor);
            drawText(TextFormat("%.1f m/s", v->getSpeed()), cursorX + 100, cursorY - 2, fontSize, valueColor); 
            cursorY += lineSpacing;

            drawText("Avg Speed:", cursorX, cursorY, fontSize, labelColor);
            drawText(TextFormat("%.1f m/s", v->getAverageSpeed()), cursorX + 100, cursorY - 2, fontSize, valueColor);
        }
        else if (selectionType == SelectionType::INTERSECTION && selectedIntersection) {
            drawText("SELECTED: INTERSECTION", cursorX, cursorY, fontSize, labelColor); cursorY += 18;
            
            drawText("ID:", cursorX, cursorY, fontSize, labelColor);
            drawText(TextFormat("#%ld", selectedIntersection->id), cursorX + 100, cursorY - 2, fontSize, valueColor);
            cursorY += lineSpacing;

            drawText("Visits:", cursorX, cursorY, fontSize, labelColor);
            drawText(TextFormat("%ld", selectedIntersection->stats.vehiclesVisited), cursorX + 100, cursorY - 2, fontSize, valueColor);
        }
        else if (selectionType == SelectionType::ROAD && selectedRoad) {
            drawText("SELECTED: ROAD", cursorX, cursorY, fontSize, labelColor); cursorY += 18;

            drawText("Traffic:", cursorX, cursorY, fontSize, labelColor);
            drawText(TextFormat("%ld", selectedRoad->stats.vehiclesPassed), cursorX + 100, cursorY - 2, fontSize, valueColor);
            cursorY += lineSpacing;

            drawText("Avg Speed:", cursorX, cursorY, fontSize, labelColor);
            drawText(TextFormat("%.1f m/s", selectedRoad->stats.getAverageSpeed()), cursorX + 100, cursorY - 2, fontSize, valueColor);
            cursorY += lineSpacing;
            
            drawText("Type:", cursorX, cursorY, fontSize, labelColor);
            drawText(selectedRoad->isOneWay ? "One Way" : "Two Way", cursorX + 100, cursorY - 2, fontSize, valueColor);
        }
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