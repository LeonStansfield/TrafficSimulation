## Todo
Features:
 - [x] Implement heatmap rendering of traffic speed
 - [x] Multi-lane behaviour
 - [ ] Traffic lights
 - [x] Roundabouts
 - [ ] Disable/enable roads
 - [ ] Press H for help in UI, which shows all controls
 - [ ] New Config File stuff:
    - [ ] Simulation ticks per second
    - [ ] Left/Right hand drive
    - [x] MaxRoadsToLoad limit for scaling benchmark
 - [x] Unit Tests
 - [x] Benchmarking mode, with data exporting to CSV.

Bugs:
 - [ ] Fast simulation mode causes vehicles to end up off-road sometimes when fast mode ends.
 - [ ] Exponential time complexity in vehicle spawning?
 - [x] Refactor code so map rendering and other different rendering types (normal, debug, heatmap) are in their own seperate classes and files.

optimisations and improvements:
 - [x] Parallelize/optimise vehicle behavior
 - [ ] Parallelize Quadtree

Benchmarks:
 - [x] Comparison of computation time of different numbers of vehicles WITH and WITHOUT quadtree
 - [ ] Comparison of computation time on different thread counts
 - [ ] Comparison of computation time of renderer on different map sizes WITH and WITHOUT render batching!!!

Report:
 - [ ] Label new screenshots with captions
 - [ ] Find new diagram to demonstrate emergent behaviour
 - [ ] Complete map scaling benchmarks
 - [ ] Implement this in production/ evaluation section
 - [ ] Write conclusion section.