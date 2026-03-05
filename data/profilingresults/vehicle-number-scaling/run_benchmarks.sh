#!/bin/bash

# Ensure we are in the project root (assuming script is run from data/profilingresults)
# Go up two levels to reach project root
cd ../../

mkdir -p data/profilingresults

echo "Starting Benchmarks..."

for i in {100..1500..100}
do
    echo "Running Benchmark: $i Vehicles"
    ./build/TrafficSimulator -OSMFile data/maps/Berkely.osm -NumberOfVehicles $i -FastSimulation false -SimulationTicks 1200 -BenchmarkOutput data/profilingresults/benchmark_$i.csv
    echo "Done."
done

echo "All benchmarks completed. Results saved in data/profilingresults/"
