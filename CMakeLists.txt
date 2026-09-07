cmake_minimum_required(VERSION 3.28)
project(RfSimulatorNodeBinding LANGUAGES CXX)

# Force whichever compiler found by cmake or cmake-js to use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include C++ project to build RfSimulationLib
add_subdirectory(src/cpp)

# CMAKE_JS_VERSION is automatically defined when building via cmake-js
if(DEFINED CMAKE_JS_VERSION)
    message(STATUS "Configuring Node.js addon with cmake-js...")

    # cmake-js hardcodes an MSVC linker flag when building on Windows
    # When using MinGW/GCC, remove it
    if(MINGW)
        string(REPLACE "/DELAYLOAD:NODE.EXE" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
        # Updated cached linker flags to override command-line injection
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "Linker flags" FORCE)
    endif()

    add_library(rf_simulator SHARED src/cpp/src/napi.cpp)
    
    # Node.js addons must use the .node extension
    set_target_properties(rf_simulator PROPERTIES PREFIX "" SUFFIX ".node")
    
    # Link headers provided by cmake-js
    target_include_directories(rf_simulator PRIVATE ${CMAKE_JS_INC})
    
    # Link RfSimulationLib to N-API object
    target_link_libraries(rf_simulator PRIVATE RfSimulationLib ${CMAKE_JS_LIB})
    
    # Include node-addon-api dynamically
    execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # Strip the quotes returned by Node
    string(REPLACE "\"" "" NODE_ADDON_API_DIR "${NODE_ADDON_API_DIR}")
    
    target_include_directories(rf_simulator PRIVATE ${NODE_ADDON_API_DIR})
    
    # Disable C++ exceptions for N-API safety
    target_compile_definitions(rf_simulator PRIVATE NAPI_DISABLE_CPP_EXCEPTIONS)
endif()