# Set the minimum required version of CMake and define the project.
cmake_minimum_required(VERSION 3.14)
project(HagnusCarlsen VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Global Project Configuration ---

# Add the main 'include' directory globally so all sub-projects can find headers.
include_directories(include)

# Enable testing with CTest, CMake's testing framework.
enable_testing()

# --- Build the Core Engine Library ---

# Find all engine source files automatically.
file(GLOB_RECURSE ENGINE_SOURCES "src/chess/*.cpp" "src/engine/*.cpp" "src/utils/*.cpp")

# Create a library named 'engine' from these source files.
# This library will contain all your core chess logic.
add_library(engine ${ENGINE_SOURCES})

# Apply compiler optimizations and flags specifically to the 'engine' library.
target_compile_options(engine PRIVATE -O3 -march=native)
if(UNIX AND NOT APPLE)
    target_link_libraries(engine PRIVATE pthread)
endif()

# --- Build the Main UCI Executable ---

# Create the main executable for the UCI interface.
add_executable(HagnusCarlsen main.cpp)

# Link the main executable against our core 'engine' library.
target_link_libraries(HagnusCarlsen PRIVATE engine)


# --- Build Benchmarks (Optional) ---

# Create an option to allow enabling/disabling benchmark builds.
option(BUILD_BENCHMARKS "Build the benchmark executables" OFF)

if(BUILD_BENCHMARKS)
    message(STATUS "Building benchmarks...")
    # Find all benchmark source files.
    file(GLOB BENCHMARK_FILES "benchmarks/*.cpp")

    # Create a separate executable for each benchmark file.
    foreach(benchmark_file ${BENCHMARK_FILES})
        # Get the filename without the extension to use as the executable name.
        get_filename_component(benchmark_name ${benchmark_file} NAME_WE)
        add_executable(${benchmark_name} ${benchmark_file})
        target_link_libraries(${benchmark_name} PRIVATE engine)
    endforeach()
endif()


# --- Build Tests (Optional) ---

# Create an option to allow enabling/disabling test builds.
option(BUILD_TESTS "Build the test executables" OFF)

if(BUILD_TESTS)
    message(STATUS "Building tests...")
    # Find all test source files.
    file(GLOB TEST_FILES "tests/*.cpp")

    foreach(test_file ${TEST_FILES})
        get_filename_component(test_name ${test_file} NAME_WE)
        add_executable(${test_name} ${test_file})
        target_link_libraries(${test_name} PRIVATE engine)
        # Add the executable to CMake's test runner (CTest).
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()

# --- Output ---

# Print a message after configuration is done.
message(STATUS "Configuration complete. Main executable is 'HagnusCarlsen'.")
message(STATUS "To build tests, use: cmake .. -DBUILD_TESTS=ON")
message(STATUS "To build benchmarks, use: cmake .. -DBUILD_BENCHMARKS=ON")

