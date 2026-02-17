#include "Engine.hpp"
#include "Benchmarker.hpp"
#include <iostream>
#include <string>

Engine::Engine(int width, int height, const char *title, std::string mapFile,
               int numVehicles)
    : renderer(width, height, title) {
  config.mapFile = mapFile;
  config.numVehicles = numVehicles;
}

Engine::~Engine() {}

void Engine::setMap(std::unique_ptr<Map> newMap) {

  simulation.setMap(std::move(newMap));
  renderer.resetCamera(simulation.getMap());
}

void Engine::addObject(std::unique_ptr<Object> object) {
  simulation.addObject(std::move(object));
}

void Engine::run() {
  while (!WindowShouldClose()) {

    // Input Handling
    input.handleInput(renderer.getCamera(), simulation);

    if (IsKeyPressed(KEY_SPACE)) {
      isPaused = !isPaused;
    }

    if (IsKeyPressed(KEY_R)) {
      renderer.resetCamera(simulation.getMap());
    }

    if (IsKeyPressed(KEY_Q)) {
      renderer.cycleDrawMode();
    }

    // Update
    if (!isPaused) {
      simulation.update(GetFrameTime());
    }

    // Draw
    renderer.beginDrawing();

    renderer.drawSimulation(simulation, input);

    // We pass *this (Engine) to Gui so it can access/modify pause state and get
    // config
    gui.draw(simulation, input, renderer, *this);

    renderer.endDrawing();
  }
}

void Engine::runFast(int ticks) {
  if (!simulation.getMap()) {
    std::cerr << "Map is not set. Aborting fast run." << std::endl;
    return;
  }

  if (!config.benchmarkOutput.empty()) {
      Benchmarker::Get().BeginSession(config.benchmarkOutput);
  }

  const float FIXED_DELTA_TIME = 1.0f / 60.0f;

  for (int i = 0; i < ticks; ++i) {
    if (i % (ticks / 100) == 0) {
      std::cout << "\rSimulation progress: " << (i * 100 / ticks) << "%"
                << std::flush;
    }
    
    Benchmarker::Get().Start("TotalTick");
    simulation.update(FIXED_DELTA_TIME);
    Benchmarker::Get().Stop("TotalTick");
    
    Benchmarker::Get().WriteFrame(i);
  }
  std::cout << "\rSimulation progress: 100%." << std::endl;
  
  Benchmarker::Get().EndSession();
}

void Engine::spawnVehicles(int count) { simulation.spawnVehicles(count); }