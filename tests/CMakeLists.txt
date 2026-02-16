include(GoogleTest)

add_executable(TrafficSimulatorTests
    QuadtreeTests.cpp
    PathfinderTests.cpp
    MapTests.cpp
    VehicleTests.cpp
)

target_link_libraries(TrafficSimulatorTests PRIVATE
    TrafficSimulatorLib
    gtest_main
)

gtest_discover_tests(TrafficSimulatorTests)
