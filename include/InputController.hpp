#pragma once

#include "Map.hpp"
#include "Simulation.hpp"
#include "Vehicle.hpp"
#include "raylib.h"

enum class SelectionType { NONE, VEHICLE, INTERSECTION, ROAD };

class InputController {
public:
  SelectionType selectionType;
  const Object *selectedVehicle;
  const Intersection *selectedIntersection;
  const Road *selectedRoad;
  bool showHelp;

public:
  InputController();

  void handleInput(Camera2D &camera, Simulation &simulation);

  // Getters for GUI
  SelectionType getSelectionType() const { return selectionType; }
  const Object *getSelectedVehicle() const { return selectedVehicle; }
  const Intersection *getSelectedIntersection() const {
    return selectedIntersection;
  }
  const Road *getSelectedRoad() const { return selectedRoad; }
  bool getShowHelp() const { return showHelp; }
};
