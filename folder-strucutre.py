import shutil
from pathlib import Path

# --- Placeholder Content Definitions ---

CONTENT_TYPES_H = """
#pragma once

/**
 * @file types.h
 * @brief Contains fundamental type definitions for chess logic.
 *
 * Defining core types like Square, Color, Piece, and Move here helps
 * prevent circular dependencies between other headers like board.h and move.h.
 */

#include <cstdint>

// Example type definitions. Adapt as needed for your engine.
using Square = int8_t;
using Color = int8_t;
using PieceType = int8_t;
using Piece = int8_t;
using Move = uint16_t;

enum : Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    // ... and so on for all 64 squares
    NO_SQUARE = 64
};

enum : Color {
    WHITE, BLACK, NO_COLOR
};

// ... add more core definitions here
"""

CONTENT_BITBOARD_H = """
#pragma once

/**
 * @file bitboard.h
 * @brief Contains helper functions for bitboard manipulation.
 *
 * Centralizing these highly-optimized, low-level functions makes them reusable
 * across your move generator, evaluation, and other modules.
 */

#include <cstdint>
#include "types.h"

using Bitboard = uint64_t;

namespace Bitboards {
    // Function to set a bit on a bitboard
    inline void set_bit(Bitboard& bb, Square s) {
        bb |= (1ULL << s);
    }

    // Add other common helpers:
    // - pop_bit(Bitboard& bb)
    // - get_lsb_index(Bitboard bb)
    // - count_bits(Bitboard bb)
    // - print_bitboard(Bitboard bb)
}
"""

CONTENT_OPTIONS_H = """
#pragma once

/**
 * @file options.h
 * @brief Defines a structure for tunable engine parameters.
 *
 * These options can be modified at runtime by the UCI 'setoption' command,
 * allowing for flexible engine configuration without recompiling.
 */

#include <string>
#include <map>

struct EngineOptions {
    int hash_size_mb = 128;
    int thread_count = 1;
    bool own_book = true;
    // Add other UCI options like "Ponder", "Contempt", etc.
};

// A global options object that can be accessed by the engine modules.
// The UCI handler will be responsible for updating it.
extern EngineOptions options;
"""

CONTENT_UCI_H = """
#pragma once

/**
 * @file uci.h
 * @brief Declares the main loop for the Universal Chess Interface (UCI).
 *
 * This module is the "mouth and ears" of the engine, responsible for
 * communicating with a GUI or command line.
 */

namespace uci {
    // The main entry point for the UCI protocol.
    // This function will start a loop to listen for and process commands.
    void loop();
}
"""

CONTENT_UCI_CPP = """
#include "engine/uci.h"
#include "engine/search.h"
#include "chess/board.h"
#include "chess/fen.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>

/**
 * @file uci.cpp
 * @brief Implements the UCI protocol command loop and handlers.
 */

void uci::loop() {
    Board board;
    // ... state variables

    std::string line;
    while (std::getline(std::cin, line)) {
        // Example command parsing
        if (line == "uci") {
            std::cout << "id name MyChessEngine" << std::endl;
            std::cout << "id author YourName" << std::endl;
            // ... send options
            std::cout << "uciok" << std::endl;
        } else if (line == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (line.rfind("position", 0) == 0) {
            // Parse position command
            // fen::set_from_fen(board, ...);
        } else if (line.rfind("go", 0) == 0) {
            // Launch search in a new thread
            // std::thread search_thread(search::start, ...);
            // search_thread.detach();
        } else if (line == "quit") {
            break;
        }
        // ... handle other commands: "stop", "setoption", etc.
    }
}
"""

CONTENT_ROOT_CMAKE = """
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

"""

CONTENT_SRC_CMAKE = """
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
"""

CONTENT_TOOLS_CMAKE = """
# This file defines command-line executables.

# Define the main CLI executable.
add_executable(engine_cli engine_cli.cpp)

# Link the executable against the core engine library.
target_link_libraries(engine_cli PRIVATE chess_engine)

# You can add other tools here as well.
# add_executable(perft_runner perft_runner.cpp)
# target_link_libraries(perft_runner PRIVATE chess_engine)
"""

CONTENT_TESTS_CMAKE = """
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
"""


def create_file(path: Path, content: str):
    """Creates a file with content if it doesn't exist."""
    try:
        if not path.exists():
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content.strip())
            print(f"✅ Created: {path.relative_to(Path.cwd())}")
        else:
            print(f"🟡 Already exists, skipped: {path.relative_to(Path.cwd())}")
    except Exception as e:
        print(f"❌ Error creating {path}: {e}")


def main():
    """Main function to run the refactoring script."""
    project_root = Path.cwd()
    print(f"🚀 Running refactor script in project root: {project_root}\n")

    # Define all files to be created
    files_to_create = {
        "include/chess/types.h": CONTENT_TYPES_H,
        "include/chess/bitboard.h": CONTENT_BITBOARD_H,
        "include/engine/options.h": CONTENT_OPTIONS_H,
        "include/engine/uci.h": CONTENT_UCI_H,
        "src/engine/uci.cpp": CONTENT_UCI_CPP,
        "src/CMakeLists.txt": CONTENT_SRC_CMAKE,
        "tools/CMakeLists.txt": CONTENT_TOOLS_CMAKE,
        "tests/CMakeLists.txt": CONTENT_TESTS_CMAKE,
    }

    print("--- Creating new files and directories ---")
    for rel_path, content in files_to_create.items():
        create_file(project_root / rel_path, content)

    print("\n--- Updating root CMakeLists.txt ---")
    root_cmake_path = project_root / "CMakeLists.txt"
    if root_cmake_path.exists():
        backup_path = project_root / "CMakeLists.txt.bak"
        try:
            shutil.copy(root_cmake_path, backup_path)
            print(f"✅ Backed up existing CMakeLists.txt to {backup_path.name}")
            root_cmake_path.write_text(CONTENT_ROOT_CMAKE.strip())
            print("✅ Replaced root CMakeLists.txt with new modular version.")
        except Exception as e:
            print(f"❌ Error updating root CMakeLists.txt: {e}")
    else:
        create_file(root_cmake_path, CONTENT_ROOT_CMAKE)

    print("\n🎉 Refactoring complete! Your project structure has been upgraded.")
    print("\n--- Next Steps ---")
    print("1. Review the new placeholder files and integrate your logic.")
    print("2. Update the new `src/CMakeLists.txt` to include all your source files.")
    print("3. Re-run CMake to generate your new build files.")


if __name__ == "__main__":
    print("=" * 60)
    print("⚠️  WARNING: This script will modify your project structure. ⚠️")
    print("Please make sure you have backed up your project or are using")
    print("a version control system like Git before proceeding.")
    print("=" * 60)

    answer = input("Do you want to continue? (yes/no): ").lower().strip()
    if answer == 'yes':
        print("")
        main()
    else:
        print("\nAborted by user.")