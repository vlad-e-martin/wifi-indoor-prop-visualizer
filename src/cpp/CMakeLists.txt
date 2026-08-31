cmake_minimum_required(VERSION 3.28)
project(RfSimulation VERSION 1.0 LANGUAGES CXX)

# Require C++20 for <numbers> and <optional>
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Fetch Eigen 3.4.0
FetchContent_Declare(
    eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG        5.0.0
)
FetchContent_MakeAvailable(eigen)

# Fetch nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
)
FetchContent_MakeAvailable(json)

# Explicitly define which source files the compiler should build 
add_library(RfSimulationLib
    src/ReflectionEngine.cpp
    src/SimulationEngine.cpp
    # src/IndoorSimulator.cpp
)

# Tell the compiler where the include/ folder is
target_include_directories(RfSimulationLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link the external headers to our library
target_link_libraries(RfSimulationLib PUBLIC 
    Eigen3::Eigen 
    nlohmann_json::nlohmann_json
)

enable_testing()

# Fetch Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.18.0
)
# For Windows, prevent gtest from overriding the static/shared CRT settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(tests)