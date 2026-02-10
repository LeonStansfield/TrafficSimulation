## Todo
Features:
 - [x] Implement heatmap rendering of traffic speed
 - [x] Multi-lane behaviour
 - [ ] Traffic lights
 - [x] Roundabouts
 - [ ] New Config File stuff:
    - [ ] Simulation ticks per second
    - [ ] Left/Right hand drive
 - [ ] Unit Tests

Bugs:
 - [ ] Fast simulation mode causes vehicles to end up off-road sometimes when fast mode ends.
 - [x] Refactor code so map rendering and other different rendering types (normal, debug, heatmap) are in their own seperate classes and files.

optimisations and improvements:
 - [x] Parallelize/optimise vehicle behavior
 - [ ] Parallelize Quadtree