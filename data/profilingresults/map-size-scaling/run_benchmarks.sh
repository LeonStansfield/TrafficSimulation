#!/bin/bash

# Ensure we are in the project root
cd ../../../

mkdir -p data/profilingresults/map-size-scaling

echo "Starting Map Size Scaling Benchmarks..."

# We benchmark scaling from 1000 up to 15000 roads in steps of 1000
for i in {1000..7000..1000}
do
    echo "Running Benchmark: Max $i Roads"
    ./build/TrafficSimulator -OSMFile data/maps/BristolCity.pbf -NumberOfVehicles 200 -FastSimulation false -SimulationTicks 1000 -MaxRoadsToLoad $i -BenchmarkOutput data/profilingresults/map-size-scaling/benchmark_$i.csv
    echo "Done."
done

echo "All benchmarks completed. Results saved in data/profilingresults/map-size-scaling/"
