#pragma once

#include "Simulation.hpp"
#include "raylib.h"
#include <memory>

class InputController; // Forward declaration

class Renderer {
private:
  int screenWidth;
  int screenHeight;
  Camera2D camera;
  Font font;
  DrawMode currentMode;

public:
  Renderer(int width, int height, const char *title);
  ~Renderer();

  void beginDrawing();
  void endDrawing();

  void resetCamera(const Map *map);
  void setCameraTarget(Vector2 target);
  void cycleDrawMode();

  // Updated to take InputController for selection highlighting
  void drawSimulation(const Simulation &simulation,
                      const InputController &input);

  // Camera Control
  Camera2D &getCamera() { return camera; } // Mutable for InputController
  const Camera2D &getCamera() const {
    return camera;
  } // Const for rendering/GUI

  // Mode Control
  void setDrawMode(DrawMode mode) { currentMode = mode; }
  DrawMode getDrawMode() const { return currentMode; }

  // Resources
  const Font &getFont() const { return font; }
};
