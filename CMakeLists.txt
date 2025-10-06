cmake_minimum_required(VERSION 3.16)
project(ChessEngine LANGUAGES CXX)

# Set a C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add subdirectories. Each contains its own CMakeLists.txt
# that defines targets (libraries or executables).
add_subdirectory(src)

# Add tests and tools if you want them to be built by default
add_subdirectory(tests)
add_subdirectory(tools)
add_subdirectory(benchmarks)