# This file defines the core engine library target.

# Gather all source files for the library.
# Add your .cpp files from src/chess, src/engine, and src/utils here.
file(GLOB_RECURSE ENGINE_SOURCES
    "chess/*.cpp"
    "engine/*.cpp"
    "utils/*.cpp"
)

# Define the library
add_library(chess_engine ${ENGINE_SOURCES})

# Specify that the library needs access to the public include directory.
# 'PUBLIC' means that any target linking against chess_engine will also
# inherit this include directory.
target_include_directories(chess_engine
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)