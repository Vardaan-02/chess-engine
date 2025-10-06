# This file defines test executables.

# Enable testing for the project
enable_testing()

# Example for a single test runner.
# You might use a testing framework like GTest or Catch2 here.
add_executable(run_tests
    board_tests.cpp
    movegen_tests.cpp
)

# Link tests against the core engine library
target_link_libraries(run_tests PRIVATE chess_engine)

# Add a test to CTest
add_test(NAME AllTests COMMAND run_tests)