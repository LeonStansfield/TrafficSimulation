#include "InputController.hpp"
#include "Quadtree.hpp"
#include "raymath.h"

InputController::InputController()
    : selectionType(SelectionType::NONE), selectedVehicle(nullptr),
      selectedIntersection(nullptr), selectedRoad(nullptr) {}

void InputController::handleInput(Camera2D &camera, Simulation &simulation) {
  // Camera controls
  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    camera.target.y -= 10.0f / camera.zoom;
  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    camera.target.y += 10.0f / camera.zoom;
  if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    camera.target.x -= 10.0f / camera.zoom;
  if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    camera.target.x += 10.0f / camera.zoom;

  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    camera.offset = GetMousePosition();
    camera.target = mouseWorldPos;
    const float zoomIncrement = 0.125f;
    camera.zoom += (wheel * zoomIncrement);
    if (camera.zoom < zoomIncrement)
      camera.zoom = zoomIncrement;
  }

  // Interaction / Selection Logic
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

    selectionType = SelectionType::NONE;
    selectedVehicle = nullptr;
    selectedIntersection = nullptr;
    selectedRoad = nullptr;

    const Map *map = simulation.getMap();
    const Quadtree *quadtree = simulation.getQuadtree();

    if (!quadtree || !map)
      return;

    // 1. Try to select a vehicle
    float searchRadius = 20.0f;
    Rectangle queryBox = {mouseWorldPos.x - searchRadius,
                          mouseWorldPos.y - searchRadius, searchRadius * 2,
                          searchRadius * 2};
    auto nearby = quadtree->query(queryBox);
    float minDist = searchRadius;

    for (auto *v : nearby) {
      float dist = Vector2Distance(v->getPosition(), mouseWorldPos);
      if (dist < minDist) {
        minDist = dist;
        selectedVehicle = v;
        selectionType = SelectionType::VEHICLE;
      }
    }

    // 2. Try to select an intersection
    if (selectionType == SelectionType::NONE) {
      const Intersection *closestInt =
          map->getClosestIntersection(mouseWorldPos);
      if (closestInt) {
        float dist = Vector2Distance(closestInt->position, mouseWorldPos);
        if (dist < 20.0f) {
          selectedIntersection = closestInt;
          selectionType = SelectionType::INTERSECTION;
        }
      }
    }

    // 3. Try to select a road
    if (selectionType == SelectionType::NONE) {
      const Road *closestRoad = map->getClosestRoad(mouseWorldPos);
      if (closestRoad && !closestRoad->points.empty()) {
        bool inRange = false;
        for (const auto &p : closestRoad->points) {
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
}
