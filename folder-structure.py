import os

folders = [
    "docs",
    "include/chess",
    "include/engine",
    "include/utils",
    "src/chess",
    "src/engine",
    "src/utils",
    "tests",
    "benchmarks",
    "tools",
    "data",
]

files = {
    "CMakeLists.txt": "",
    "main.cpp": "// Entry point for the chess engine\n\nint main() {\n    return 0;\n}\n",
    "docs/architecture.md": "# Chess Engine Architecture\n\nDesign notes and diagrams go here.\n",

    # Chess headers
    "include/chess/board.h": "#pragma once\n\nclass Board {\npublic:\n    void print();\n};\n",
    "include/chess/move.h": "#pragma once\n\nclass Move {\n    // Move representation\n};\n",
    "include/chess/movegen.h": "#pragma once\n\nclass MoveGen {\n    // Move generation logic\n};\n",
    "include/chess/fen.h": "#pragma once\n\nnamespace FEN {\n    // FEN parsing/serialization\n}\n",
    "include/chess/zobrist.h": "#pragma once\n\nnamespace Zobrist {\n    // Hashing implementation\n}\n",
    "include/chess/evaluate.h": "#pragma once\n\nclass Evaluate {\n    // Evaluation function\n};\n",

    # Engine headers
    "include/engine/search.h": "#pragma once\n\nclass Search {\n    // Minimax / Alpha-Beta\n};\n",
    "include/engine/transposition.h": "#pragma once\n\nclass TranspositionTable {\n    // TT logic\n};\n",
    "include/engine/time.h": "#pragma once\n\nclass TimeManager {\n    // Time controls\n};\n",

    # Utils headers
    "include/utils/logger.h": "#pragma once\n\nnamespace Logger {\n    void log(const char* msg);\n}\n",
    "include/utils/config.h": "#pragma once\n\nnamespace Config {\n    // Engine constants & config\n}\n",

    # Tests
    "tests/board_tests.cpp": "#include <cassert>\n\nint main() {\n    // TODO: Unit tests for Board\n    return 0;\n}\n",
    "tests/movegen_tests.cpp": "#include <cassert>\n\nint main() {\n    // TODO: Unit tests for MoveGen\n    return 0;\n}\n",

    # Benchmarks
    "benchmarks/perft.cpp": "// Perft benchmark tool\nint main() { return 0; }\n",
    "benchmarks/search_benchmark.cpp": "// Search performance tests\nint main() { return 0; }\n",

    # Tools
    "tools/perft_runner.cpp": "// CLI tool for perft testing\nint main() { return 0; }\n",
    "tools/fen_parser.cpp": "// CLI tool for FEN parsing\nint main() { return 0; }\n",
    "tools/engine_cli.cpp": "// Simple UCI CLI\nint main() { return 0; }\n",

    # Data
    "data/openings.epd": "# Opening positions placeholder\n",
    "data/test_positions.fen": "# Test positions placeholder\n",
}

# Auto-generate matching .cpp for headers
def make_src_files():
    src_files = {}
    for path in files.keys():
        if path.startswith("include/") and path.endswith(".h"):
            src_path = path.replace("include/", "src/").replace(".h", ".cpp")
            header_name = os.path.basename(path)
            src_files[src_path] = f'#include "{header_name}"\n\n// Implementation goes here\n'
    return src_files

def make_structure():
    for folder in folders:
        os.makedirs(folder, exist_ok=True)
        print(f"[+] Created folder: {folder}")

    all_files = {**files, **make_src_files()}

    for filepath, content in all_files.items():
        with open(filepath, "w") as f:
            f.write(content)
        print(f"[+] Created file: {filepath}")

if __name__ == "__main__":
    make_structure()
    print("\n🚀 Chess Engine project structure initialized successfully with src implementations!")
