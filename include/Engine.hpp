#pragma once

#include "Gui.hpp"
#include "InputController.hpp"
#include "Renderer.hpp"
#include "Simulation.hpp"
#include <memory>
#include <vector>

class Engine {
private:
  Simulation simulation;
  Renderer renderer;
  InputController input;
  Gui gui;

public:
  Engine(int width, int height, const char *title);
  ~Engine();

  void setMap(std::unique_ptr<Map> newMap);
  void addObject(std::unique_ptr<Object> object);
  void run();
  void runFast(int ticks);

public:
  void spawnVehicles(int count);
};