## Todo
Features:
 - Implement heatmap rendering of traffic speed
 - Multi-lane behaviour
 - Traffic lights
 - Roundabouts

Bugs:
 - Intersections are not counting vehicles passing them in their stats.
 - Fast simulation mode causes vehicles to end up off-road sometimes when fast mode ends.
 - Refactor code so map rendering and other different rendering types (normal, debug, heatmap) are in their own seperate classes and files.

optimisations and improvements:
 - Parallelize/optimise vehicle behavior
 - Parallelize Quadtree