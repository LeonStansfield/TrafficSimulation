#include "Renderer.hpp"
#include "Vehicle.hpp"
#include "rlgl.h"
#include <algorithm>
#include <iostream>

Renderer::Renderer(int width, int height, const char *title)
    : screenWidth(width), screenHeight(height), currentMode(DrawMode::NORMAL) {

  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(screenWidth, screenHeight, title);
  if (!IsWindowReady()) {
    std::cerr << "FATAL: Failed to initialize Raylib window." << std::endl;
    throw std::runtime_error("Failed to initialize window");
  }
  // Default FPS cap; can be overridden later
  SetTargetFPS(60);

  // Font Loading
  font = LoadFontEx("../data/Consolas-Regular.ttf", 32, 0, 0);
  if (font.texture.id == 0) {
    std::cerr << "WARNING: Failed to load custom font. Using default."
              << std::endl;
    font = GetFontDefault();
  } else {
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  }

  camera = {0};
  camera.zoom = 1.0f;
}

Renderer::~Renderer() {
  if (font.texture.id != GetFontDefault().texture.id) {
    UnloadFont(font);
  }
  CloseWindow();
}

void Renderer::beginDrawing() {
  BeginDrawing();
  ClearBackground(LIGHTGRAY);
}

void Renderer::endDrawing() { EndDrawing(); }

void Renderer::cycleDrawMode() {
  if (currentMode == DrawMode::NORMAL) {
    currentMode = DrawMode::HEATMAP;
  } else if (currentMode == DrawMode::HEATMAP) {
    currentMode = DrawMode::DEBUG;
  } else {
    currentMode = DrawMode::NORMAL;
  }
}

void Renderer::resetCamera(const Map *map) {
  if (!map)
    return;
  camera.target = {map->getWorldWidth() / 2.0f, map->getWorldHeight() / 2.0f};
  camera.offset = {screenWidth / 2.0f, screenHeight / 2.0f};

  float scaleX = static_cast<float>(screenWidth) / map->getWorldWidth();
  float scaleY = static_cast<float>(screenHeight) / map->getWorldHeight();
  camera.zoom = std::min(scaleX, scaleY);
}

#include "InputController.hpp" // Fully include for selection details

void Renderer::drawSimulation(const Simulation &simulation,
                              const InputController &input) {
  const Map *map = simulation.getMap();
  if (!map)
    return;

  BeginMode2D(camera);

  const_cast<Map *>(map)->draw(currentMode);

  // Draw vehicles only if not in Heatmap mode
  if (currentMode != DrawMode::HEATMAP) {
    for (const auto &obj : simulation.getObjects()) {
      obj->draw(currentMode == DrawMode::DEBUG);
    }
  }

  // Draw Selection Highlights
  SelectionType selectionType = input.getSelectionType();
  if (selectionType != SelectionType::NONE) {

    // Highlight Vehicle
    if (selectionType == SelectionType::VEHICLE) {
      const Vehicle *v =
          dynamic_cast<const Vehicle *>(input.getSelectedVehicle());
      if (v) {
        // Draw selection ring
        DrawRing(v->getPosition(), v->getSize().x / 2.0f + 2.0f,
                 v->getSize().x / 2.0f + 5.0f, 0, 360, 0, DARKBLUE);

        // Draw Path
        const std::vector<const Road *> &path = v->getPath();
        int pathIndex = v->getPathIndex();

        // Current road remaining points
        // We can't access currentRoadPoints easily from here without getter,
        // but we can draw the future path roads.

        // Draw lines for the future path
        for (size_t i = pathIndex; i < path.size(); ++i) {
          const Road *r = path[i];
          if (r) {
            for (size_t j = 1; j < r->points.size(); ++j) {
              DrawLineEx(r->points[j - 1], r->points[j], 4.0f,
                         Fade(DARKBLUE, 0.5f));
            }
          }
        }
      }
    }
    // Highlight Road
    else if (selectionType == SelectionType::ROAD) {
      const Road *r = input.getSelectedRoad();
      if (r) {
        Color highlightColor = r->disabled.load() ? Fade(RED, 0.8f) : Fade(BLUE, 0.6f);
        for (size_t j = 1; j < r->points.size(); ++j) {
          DrawLineEx(r->points[j - 1], r->points[j], 6.0f, highlightColor);
        }
      }
    }
    // Highlight Intersection
    else if (selectionType == SelectionType::INTERSECTION) {
      const Intersection *i = input.getSelectedIntersection();
      if (i) {
        DrawCircleV(i->position, 8.0f, MAGENTA);
        DrawRing(i->position, 8.0f, 12.0f, 0, 360, 0, PURPLE);
      }
    }
  }

  // Draw Quadtree debug
  if (currentMode == DrawMode::DEBUG) {
    const Quadtree *qt = simulation.getQuadtree();
    if (qt) {
      const_cast<Quadtree *>(qt)->draw(camera);
    }
  }

  EndMode2D();
}

void Renderer::setCameraTarget(Vector2 target) { camera.target = target; }
