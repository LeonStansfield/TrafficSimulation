#pragma once

#include "Simulation.hpp"
#include "raylib.h"
#include <memory>

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

  void drawSimulation(const Simulation &simulation);

  // Camera Control
  Camera2D &getCamera() { return camera; } // Mutable for InputController
  const Camera2D &getCamera() const {
    return camera;
  } // Const for rendering/GUI
  void setCameraTarget(Vector2 target);
  void resetCamera(const Map *map);

  // Mode Control
  void setDrawMode(DrawMode mode) { currentMode = mode; }
  DrawMode getDrawMode() const { return currentMode; }
  void cycleDrawMode();

  // Resources
  const Font &getFont() const { return font; }
};
