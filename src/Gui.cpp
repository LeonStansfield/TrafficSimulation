#include "Gui.hpp"

Gui::Gui() {}

#include "Engine.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

// Helper to get memory usage in MB
static float getMemoryUsageMB() {
  long rss = 0;
  std::ifstream statm("/proc/self/statm");
  if (statm.is_open()) {
    long ignore;
    statm >> ignore >> rss; // Second value is RSS in pages
    statm.close();
  }
  long pageSize = sysconf(_SC_PAGESIZE);
  return (rss * pageSize) / (1024.0f * 1024.0f);
}

void Gui::draw(const Simulation &simulation, const InputController &input,
               const Renderer &renderer, Engine &engine) {
  const Font &font = renderer.getFont();
  DrawMode currentMode = renderer.getDrawMode();
  SelectionType selectionType = input.getSelectionType();

  // Define style constants
  const int boxX = 20;
  const int boxY = 20;
  const int width = 300;
  const int padding = 15;
  const int lineSpacing = 24;
  const float fontSize = 16.0f;
  const int valueOffset = 140;

  Color panelColor = {220, 220, 225, 230};
  Color borderColor = DARKGRAY;
  Color labelColor = DARKGRAY;
  Color valueColor = BLACK;
  Color accentColor = DARKBLUE;
  Color buttonColor = {200, 200, 205, 255};
  Color buttonHoverColor = {180, 180, 185, 255};

  // Calculate dynamic height
  int contentHeight = 0;

  // System Status section height
  contentHeight += 10 + 20;     // Mode
  contentHeight += lineSpacing; // Pause Status

  // Pause Button Height
  contentHeight += 30 + 10;
  contentHeight += lineSpacing; // [H] help hint

  // Debug Stats
  if (currentMode == DrawMode::DEBUG) {
    contentHeight += 10; // Divider
    contentHeight +=
        lineSpacing * 10; // Config + Stats (Map, Tick, Veh, FPS, FrameTime,
                          // Mem, Roads, Intersections, Zoom, Target)
  }

  // Selection section height
  if (selectionType != SelectionType::NONE) {
    contentHeight += 15;      // Divider
    contentHeight += 10 + 20; // Title
    contentHeight += lineSpacing * 3;
    if (selectionType == SelectionType::ROAD) {
      contentHeight += lineSpacing; // Status row
      contentHeight += lineSpacing; // Keyboard hint
    }
  }

  int totalHeight = padding * 2 + contentHeight;

  // Draw Panel
  DrawRectangle(boxX, boxY, width, totalHeight, panelColor);
  DrawRectangleLines(boxX, boxY, width, totalHeight, borderColor);

  float cursorX = (float)boxX + padding;
  float cursorY = (float)boxY + padding;

  // Helper lambda for using custom font
  auto drawText = [&](const char *text, float x, float y, float size,
                      Color col) {
    DrawTextEx(font, text, {x, y}, size, 1.0f, col);
  };

  // --- System Status ---
  drawText("SIMULATION MODE", cursorX, cursorY, fontSize, labelColor);
  cursorY += 14;

  const char *modeText = "NORMAL";
  Color modeColor = accentColor;

  switch (currentMode) {
  case DrawMode::NORMAL:
    modeText = "NORMAL";
    modeColor = accentColor;
    break;
  case DrawMode::DEBUG:
    modeText = "DEBUG";
    modeColor = ORANGE;
    break;
  case DrawMode::HEATMAP:
    modeText = "HEATMAP";
    modeColor = MAROON;
    break;
  }

  drawText(modeText, cursorX, cursorY, fontSize, modeColor);
  cursorY += 28;

  // Pause Status
  drawText("Status:", cursorX, cursorY, fontSize, labelColor);
  if (engine.getPaused()) {
    drawText("PAUSED", cursorX + valueOffset, cursorY, fontSize, ORANGE);
  } else {
    drawText("RUNNING", cursorX + valueOffset, cursorY, fontSize, DARKGREEN);
  }
  cursorY += lineSpacing + 5;

  // Pause Button
  Rectangle buttonRect = {cursorX, cursorY, (float)width - padding * 2, 30};
  Vector2 mousePos = GetMousePosition();
  bool isHovered = CheckCollisionPointRec(mousePos, buttonRect);

  if (isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    engine.togglePaused(); // Handle click
  }

  DrawRectangleRec(buttonRect, isHovered ? buttonHoverColor : buttonColor);
  DrawRectangleLinesEx(buttonRect, 1.0f, borderColor);

  const char *btnText = engine.getPaused() ? "RESUME" : "PAUSE";
  Vector2 textSize = MeasureTextEx(font, btnText, fontSize, 1.0f);
  drawText(btnText, buttonRect.x + (buttonRect.width - textSize.x) / 2,
           buttonRect.y + (buttonRect.height - textSize.y) / 2, fontSize,
           BLACK);

  cursorY += 30 + 10;

  // Help hint
  drawText("[H] help", cursorX, cursorY, fontSize - 2.0f, {100, 100, 100, 200});
  cursorY += lineSpacing;

  // --- Debug Stats ---
  if (currentMode == DrawMode::DEBUG) {
    DrawLine((int)cursorX, (int)cursorY, boxX + width - padding, (int)cursorY,
             borderColor);
    cursorY += 12;

    const auto &config = engine.getConfig();

    drawText("Map:", cursorX, cursorY, fontSize, labelColor);
    // Truncate map name if too long?
    std::string mapName = config.mapFile;
    if (mapName.length() > 20)
      mapName = "..." + mapName.substr(mapName.length() - 17);
    drawText(mapName.c_str(), cursorX + 60, cursorY, fontSize, valueColor);
    cursorY += lineSpacing;

    drawText("Tick:", cursorX, cursorY, fontSize, labelColor);
    drawText(TextFormat("%ld", simulation.getTickCount()),
             cursorX + valueOffset, cursorY, fontSize, valueColor);
    cursorY += lineSpacing;

    drawText("Vehicles:", cursorX, cursorY, fontSize, labelColor);
    drawText(
        TextFormat("%d / %d", simulation.getVehicleCount(), config.numVehicles),
        cursorX + valueOffset, cursorY, fontSize, valueColor);
    cursorY += lineSpacing;

    drawText("FPS:", cursorX, cursorY, fontSize, labelColor);
    drawText(TextFormat("%d", GetFPS()), cursorX + valueOffset, cursorY,
             fontSize, GREEN);
    cursorY += lineSpacing;

    drawText("Frame Time:", cursorX, cursorY, fontSize, labelColor);
    drawText(TextFormat("%.2f ms", GetFrameTime() * 1000.0f),
             cursorX + valueOffset, cursorY, fontSize, valueColor);
    cursorY += lineSpacing;

    drawText("Memory:", cursorX, cursorY, fontSize, labelColor);
    drawText(TextFormat("%.2f MB", getMemoryUsageMB()), cursorX + valueOffset,
             cursorY, fontSize, valueColor);
    cursorY += lineSpacing;

    // Entities
    const Map *map = simulation.getMap();
    if (map) {
      drawText("Roads:", cursorX, cursorY, fontSize, labelColor);
      drawText(TextFormat("%zu", map->getRoads().size()), cursorX + valueOffset,
               cursorY, fontSize, valueColor);
      cursorY += lineSpacing;

      drawText("Intersections:", cursorX, cursorY, fontSize, labelColor);
      drawText(TextFormat("%zu", map->getIntersections().size()),
               cursorX + valueOffset, cursorY, fontSize, valueColor);
      cursorY += lineSpacing;
    }

    // Camera
    const Camera2D &camera = renderer.getCamera();
    drawText("Zoom:", cursorX, cursorY, fontSize, labelColor);
    drawText(TextFormat("%.2f", camera.zoom), cursorX + valueOffset, cursorY,
             fontSize, valueColor);
    cursorY += lineSpacing;

    drawText("Target:", cursorX, cursorY, fontSize, labelColor);
    drawText(TextFormat("%.0f, %.0f", camera.target.x, camera.target.y),
             cursorX + valueOffset, cursorY, fontSize, valueColor);
    cursorY += lineSpacing;
  }

  // --- Selection Info ---
  if (selectionType != SelectionType::NONE) {
    DrawLine((int)cursorX, (int)cursorY, boxX + width - padding, (int)cursorY,
             borderColor);
    cursorY += 12;

    if (selectionType == SelectionType::VEHICLE) {
      const Vehicle *v =
          dynamic_cast<const Vehicle *>(input.getSelectedVehicle());
      if (v) {
        drawText("SELECTED: VEHICLE", cursorX, cursorY, fontSize, labelColor);
        cursorY += 18;

        drawText("Current Speed:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("%.1f m/s", v->getSpeed()), cursorX + valueOffset,
                 cursorY - 2, fontSize, valueColor);
        cursorY += lineSpacing;

        drawText("Avg Speed:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("%.1f m/s", v->getAverageSpeed()),
                 cursorX + valueOffset, cursorY - 2, fontSize, valueColor);
      }
    } else if (selectionType == SelectionType::INTERSECTION) {
      const Intersection *i = input.getSelectedIntersection();
      if (i) {
        drawText("SELECTED: INTERSECTION", cursorX, cursorY, fontSize,
                 labelColor);
        cursorY += 18;

        drawText("ID:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("#%ld", i->id), cursorX + valueOffset, cursorY - 2,
                 fontSize, valueColor);
        cursorY += lineSpacing;

        drawText("Visits:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("%ld", i->stats.vehiclesVisited.load()),
                 cursorX + valueOffset, cursorY - 2, fontSize, valueColor);
      }
    } else if (selectionType == SelectionType::ROAD) {
      const Road *r = input.getSelectedRoad();
      if (r) {
        drawText("SELECTED: ROAD", cursorX, cursorY, fontSize, labelColor);
        cursorY += 18;

        drawText("Traffic:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("%ld", r->stats.vehiclesPassed.load()),
                 cursorX + valueOffset, cursorY - 2, fontSize, valueColor);
        cursorY += lineSpacing;

        drawText("Avg Speed:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("%.1f m/s", r->stats.getAverageSpeed()),
                 cursorX + valueOffset, cursorY - 2, fontSize, valueColor);
        cursorY += lineSpacing;

        drawText("Type:", cursorX, cursorY, fontSize, labelColor);
        drawText(r->isOneWay ? "One Way" : "Two Way", cursorX + valueOffset,
                 cursorY - 2, fontSize, valueColor);
        cursorY += lineSpacing;

        bool isDisabled = r->disabled.load();
        drawText("Status:", cursorX, cursorY, fontSize, labelColor);
        drawText(isDisabled ? "DISABLED" : "ENABLED",
                 cursorX + valueOffset, cursorY - 2, fontSize,
                 isDisabled ? MAROON : DARKGREEN);
        cursorY += lineSpacing;

        drawText("[E] toggle", cursorX, cursorY, fontSize,
                 {100, 100, 100, 200});
      }
    }
  }

  // --- Help Panel ---
  if (input.getShowHelp()) {
    const int hBoxWidth = 360;
    const int hBoxX = boxX + width + 20;
    const int hBoxY = boxY;

    struct HelpEntry { const char *key; const char *desc; };
    const HelpEntry entries[] = {
      { "WASD / Arrows", "Pan camera"         },
      { "Scroll",        "Zoom in / out"       },
      { "Left click",    "Select element"      },
      { "Q",             "Cycle render mode"   },
      { "E",             "Enable / disable road" },
      { "Space / btn",   "Pause / resume"      },
      { "H",             "Toggle help panel"   },
    };
    const int numEntries = sizeof(entries) / sizeof(entries[0]);

    int hContentHeight = 10 + 20 + 8                  // Title
                       + lineSpacing * numEntries;     // Entries
    int hTotalHeight = padding * 2 + hContentHeight;

    DrawRectangle(hBoxX, hBoxY, hBoxWidth, hTotalHeight, panelColor);
    DrawRectangleLines(hBoxX, hBoxY, hBoxWidth, hTotalHeight, borderColor);

    float hX = hBoxX + padding;
    float hY = hBoxY + padding;

    drawText("CONTROLS", hX, hY, fontSize, accentColor);
    hY += 28;

    for (const auto &e : entries) {
      drawText(e.key,  hX,              hY, fontSize - 1.0f, labelColor);
      drawText(e.desc, hX + 140,        hY, fontSize - 1.0f, valueColor);
      hY += lineSpacing;
    }
  }
}
