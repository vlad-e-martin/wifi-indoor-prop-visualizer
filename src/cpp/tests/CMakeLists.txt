add_executable(RfSimulationTests
    ReflectionEngineTests.cpp
    SimulationEngineTests.cpp)
target_link_libraries(RfSimulationTests PRIVATE 
    RfSimulationLib 
    GTest::gtest_main 
    Eigen3::Eigen
)

# Automatically discover and register gtest macros with CTest
include(GoogleTest)
gtest_discover_tests(RfSimulationTests)