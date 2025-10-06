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