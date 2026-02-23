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

  bool isPaused = false;

  struct ConfigInfo {
    std::string mapFile;
    int numVehicles;
    std::string benchmarkOutput;
    bool useQuadtree;
  } config;

public:
  Engine(int width, int height, const char *title, std::string mapFile,
         int numVehicles, bool useQuadtree = true);
  ~Engine();

  bool getPaused() const { return isPaused; }
  void setPaused(bool paused) { isPaused = paused; }
  void togglePaused() { isPaused = !isPaused; }

  const ConfigInfo &getConfig() const { return config; }

  void setMap(std::unique_ptr<Map> newMap);
  void addObject(std::unique_ptr<Object> object);
  void run(int maxTicks = -1);
  void runFast(int ticks);
  void setBenchmarkOutput(const std::string &path) {
    config.benchmarkOutput = path;
  }

public:
  void spawnVehicles(int count);
};