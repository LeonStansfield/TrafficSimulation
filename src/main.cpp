#include "Engine.hpp"
#include "Map.hpp"
#include "Vehicle.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

// Helper function to convert string to bool
bool stringToBool(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return (str == "true" || str == "1");
}

// Parses a simple key = value config file
bool parseConfigFile(const std::string &path, std::string &mapFile,
                     int &numVehicles, bool &fastMode, int &numTicks) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open config file: " << path << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(file, line)) {
    size_t commentPos = line.find('#');
    if (commentPos != std::string::npos) {
      line = line.substr(0, commentPos);
    }

    size_t separatorPos = line.find('=');
    if (separatorPos == std::string::npos) {
      continue;
    }

    std::string key = line.substr(0, separatorPos);
    std::string value = line.substr(separatorPos + 1);

    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);

    if (key.empty() || value.empty()) {
      continue;
    }

    try {
      if (key == "OSMFile") {
        mapFile = value;
      } else if (key == "NumberOfVehicles") {
        numVehicles = std::stoi(value);
      } else if (key == "FastSimulation") {
        fastMode = stringToBool(value);
      } else if (key == "SimulationTicks") {
        numTicks = std::stoi(value);
      }
    } catch (const std::exception &e) {
      std::cerr << "Warning: Failed to parse config line: " << line << " ("
                << e.what() << ")" << std::endl;
    }
  }
  return true;
}

// Parses key-value pairs from command line, e.g., -key value
std::map<std::string, std::string> parseCmdLine(int argc, char *argv[]) {
  std::map<std::string, std::string> args;
  for (int i = 1; i < argc; ++i) {
    std::string key = argv[i];
    if (key.rfind('-', 0) == 0) { // Check if it starts with '-'
      if (i + 1 < argc) {
        std::string value = argv[i + 1];
        if (value.rfind('-', 0) != 0) { // Check if next arg is not another key
          args[key] = value;
          i++; // Skip the value
          continue;
        }
      }
      std::cerr << "Warning: Flag " << key
                << " is missing a value or value starts with '-'." << std::endl;
    }
  }
  return args;
}

void printUsage(const char *progName) {
  std::cerr << "Usage: " << progName << " [options]" << std::endl;
  std::cerr << "Options:" << std::endl;
  std::cerr
      << "  -ConfigFile <path>        Path to configuration file. (Loads first)"
      << std::endl;
  std::cerr << "  -OSMFile <path>           Path to .osm file. (Required)"
            << std::endl;
  std::cerr << "  -NumberOfVehicles <int>   Number of vehicles to spawn."
            << std::endl;
  std::cerr << "  -FastSimulation <bool>    Enable fast-forward mode."
            << std::endl;
  std::cerr << "  -SimulationTicks <int>    Number of ticks for fast-forward."
            << std::endl;
  std::cerr << std::endl;
  std::cerr << "Example (Config): " << progName << " -ConfigFile config.ini"
            << std::endl;
  std::cerr << "Example (Manual): " << progName
            << " -OSMFile data/MyMap.osm -NumberOfVehicles 500 -FastSimulation "
               "true -SimulationTicks 50000"
            << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage(argv[0]);
    return 1;
  }

  std::string mapFile = "";
  int numVehicles = 100;
  bool fastMode = false;
  int numTicks = 18000;

  std::map<std::string, std::string> args = parseCmdLine(argc, argv);

  // Load from Config File if specified
  if (args.count("-ConfigFile")) {
    std::cout << "Loading configuration from: " << args["-ConfigFile"]
              << std::endl;
    if (!parseConfigFile(args["-ConfigFile"], mapFile, numVehicles, fastMode,
                         numTicks)) {
      return 1; // Error already printed by parser
    }
  }

  // Load from Command Line (config)
  try {
    if (args.count("-OSMFile")) {
      mapFile = args["-OSMFile"];
    }
    if (args.count("-NumberOfVehicles")) {
      numVehicles = std::stoi(args["-NumberOfVehicles"]);
    }
    if (args.count("-FastSimulation")) {
      fastMode = stringToBool(args["-FastSimulation"]);
    }
    if (args.count("-SimulationTicks")) {
      numTicks = std::stoi(args["-SimulationTicks"]);
    }
  } catch (const std::exception &e) {
    std::cerr << "Error parsing command-line arguments: " << e.what()
              << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // 4. Validate
  if (mapFile.empty()) {
    std::cerr << "Error: No OSMFile specified in config or arguments."
              << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // Engine Setup
  std::cout << "--- Simulation Settings ---" << std::endl;
  std::cout << "Map: " << mapFile << std::endl;
  std::cout << "Vehicles: " << numVehicles << std::endl;
  std::cout << "Fast Mode: " << (fastMode ? "True" : "False") << std::endl;
  if (fastMode) {
    std::cout << "Ticks: " << numTicks << std::endl;
  }
  std::cout << "---------------------------" << std::endl;

  try {
    std::cout << "Initializing engine..." << std::endl;
    Engine engine(1280, 720, "Traffic Simulation", mapFile, numVehicles);

    std::cout << "Loading map..." << std::endl;
    engine.setMap(std::make_unique<Map>(mapFile.c_str()));

    std::cout << "Spawning vehicles..." << std::endl;
    engine.spawnVehicles(numVehicles);

    if (fastMode) {
      std::cout << "Running in Fast Mode for " << numTicks << " ticks."
                << std::endl;
      engine.runFast(numTicks);
      std::cout
          << "Fast simulation complete. Continuing with real-time simulation."
          << std::endl;
    } else {
      std::cout << "Running in Real-Time Mode." << std::endl;
    }

    engine.run();
  } catch (const std::exception &e) {
    std::cerr << "FATAL ERROR: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}