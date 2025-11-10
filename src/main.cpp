#include "Engine.hpp"
#include "Vehicle.hpp"
#include "Map.hpp"
#include <memory>
#include <iostream>
#include <string>
#include <algorithm>

// Helper function to convert string to bool
bool stringToBool(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return (str == "true" || str == "1");
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <PathToOSMFile> <NumberOfVehicles> [FastSimulation (bool)] [SimulationTicks (int)]" << std::endl;
        std::cerr << "  Example (Real-time): " << argv[0] << " data/MyMap.osm 500" << std::endl;
        std::cerr << "  Example (Fast Sim):  " << argv[0] << " data/MyMap.osm 500 true 50000" << std::endl;
        return 1;
    }

    std::string mapFile = argv[1];
    int numVehicles = 100;
    bool fastMode = false;
    int numTicks = 18000;

    // NumberOfVehicles
    try {
        numVehicles = std::stoi(argv[2]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid number for NumberOfVehicles. Using default: " << numVehicles << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "NumberOfVehicles out of range. Using default: " << numVehicles << std::endl;
    }

    // FastMode
    if (argc >= 4) {
        fastMode = stringToBool(argv[3]);
    }

    // SimulationTicks
    if (argc >= 5 && fastMode) {
        try {
            numTicks = std::stoi(argv[4]);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid number for SimulationTicks. Using default: " << numTicks << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "SimulationTicks out of range. Using default: " << numTicks << std::endl;
        }
    }

    // Engine Setup
    std::cout << "Initializing engine..." << std::endl;
    Engine engine(800, 600, "Traffic Simulation");

    std::cout << "Loading map: " << mapFile << std::endl;
    engine.setMap(std::make_unique<Map>(mapFile.c_str()));

    std::cout << "Spawning " << numVehicles << " vehicles..." << std::endl;
    engine.spawnVehicles(numVehicles);


    if (fastMode) {
        std::cout << "Running in Fast Mode for " << numTicks << " ticks." << std::endl;
        engine.runFast(numTicks);
        std::cout << "Fast simulation complete. Continuing with real-time simulation." << std::endl;
    } else {
        std::cout << "Running in Real-Time Mode." << std::endl;
    }

    engine.run();

    return 0;
}