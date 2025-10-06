# This file defines command-line executables.

# Define the main CLI executable.
add_executable(engine_cli engine_cli.cpp)

# Link the executable against the core engine library.
target_link_libraries(engine_cli PRIVATE chess_engine)

# You can add other tools here as well.
# add_executable(perft_runner perft_runner.cpp)
# target_link_libraries(perft_runner PRIVATE chess_engine)