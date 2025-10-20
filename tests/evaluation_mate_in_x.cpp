// Compile using: g++ -std=c++17 -I../include/chess -I../include -I../include/utils \
  -o evaluation_mate_in_x.out evaluation_mate_in_x.cpp \
  ../src/chess/*.cpp ../src/chess/movegen/*.cpp ../src/utils/*.cpp \
  ../src/engine/*.cpp ../src/engine/evaluate/*.cpp ../src/engine/search/*.cpp \
  -O3 -march=native -flto=auto -funroll-loops -pipe

#include <iostream>
#include <vector>
#include <string>
#include "chess/board.h"
#include "engine/search.h"
#include "chess/zobrist.h"

// Helper to print section headers for better organization
void printSectionHeader(const std::string& title) {
    std::cout << "\n==================================================\n";
    std::cout << "          " << title << "\n";
    std::cout << "==================================================\n\n";
}

/**
 * @brief Runs a search on a given position to a specified depth and prints the results.
 *
 * @param search_agent The search object to use.
 * @param fen The FEN string of the position to test.
 * @param depth The depth to search to.
 * @param description A description of the test case.
 * @param stockfish_eval The benchmark evaluation from Stockfish (or known puzzle solution).
 */
void testSearchAndEvaluate(Search& search_agent, std::string fen, int depth, const std::string& description, const std::string& stockfish_eval) {
    std::cout << "--- Test: " << description << " ---\n";
    std::cout << "    FEN: " << fen << "\n";
    std::cout << "    Searching to depth: " << depth << "\n";
    std::cout << "    Expected Evaluation: " << stockfish_eval << "\n\n";

    Board board;
    try {
        board.set_fen(fen);
        board.print_board();

        // The start_search function will print "info" lines containing the score
        // during its operation. We don't need to capture a return value for the score itself.
        std::cout << "    Engine thinking...\n";
        chess::Move best_move = search_agent.start_search(board, depth, 0, 0, 0, 0, 0);

        std::string move_str = util::move_to_string(best_move);
         if (best_move.flags() & chess::FLAG_PROMO) {
            switch (chess::type_of((chess::Piece)best_move.promo())) {
                case chess::QUEEN:  move_str += 'q'; break;
                case chess::ROOK:   move_str += 'r'; break;
                case chess::BISHOP: move_str += 'b'; break;
                case chess::KNIGHT: move_str += 'n'; break;
                default: break;
            }
        }

        std::cout << "\n    --- Search Complete ---\n";
        std::cout << "    Best move found: " << move_str << "\n";
        std::cout << "    Your Engine's final evaluation is the 'score mate' value in the last 'info' line above.\n";
        std::cout << "    Compare this with the expected evaluation to gauge performance.\n";

    } catch (const std::exception& e) {
        std::cerr << "    Error during test: " << e.what() << "\n";
    }
    std::cout << std::endl;
    search_agent.TT.clear();
}

int main() {
    // --- One-time initialization ---
    Zobrist::init_zobrist_keys(); 
    chess::init();
    Search search_agent(512); // Create a single search agent to use for all tests
    
    std::cout << "========== CHESS ENGINE SEARCH & EVALUATION TEST SUITE ==========\n";

    // --- Mate in 2 Puzzles ---
    printSectionHeader("Mate in 2 Puzzles");
    testSearchAndEvaluate(search_agent, "1k1r4/pp1b1R2/3Q2p1/2p5/2B5/8/PPP2B2/2K5 w - - 1 1", 4, "M2 Puzzle #1", "#2");
    testSearchAndEvaluate(search_agent, "3r1k2/4npp1/1p5p/p1p2N2/2P5/1P6/P4PPP/4R1K1 w - - 0 1", 4, "M2 Puzzle #2", "#2");
    testSearchAndEvaluate(search_agent, "2r1r3/pp1k2pp/8/2bN4/8/8/PPP2PPP/2KRR3 w - - 0 1", 4, "M2 Puzzle #3", "#2");
    testSearchAndEvaluate(search_agent, "r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq - 0 13", 4, "M2 Puzzle #4", "#2");
    testSearchAndEvaluate(search_agent, "1R6/1brk2p1/p2p1p1p/P2PpP2/4P1P1/4K3/7P/8 w - - 15 53", 4, "M2 Puzzle #5", "#2");
    testSearchAndEvaluate(search_agent, "8/6pk/p1p4p/1p5r/1P1R4/P3q2P/6P1/3Q3K w - - 0 42", 4, "M2 Puzzle #6", "#2");

    // --- Mate in 3 Puzzles ---
    // printSectionHeader("Mate in 3 Puzzles");
    // testSearchAndEvaluate(search_agent, "r5k1/pp1b1p1p/3p1Np1/q1p5/2PnP3/3B4/PP4PP/R2Q2K1 b - - 1 18", 6, "M3 Puzzle #1", "#3");
    // testSearchAndEvaluate(search_agent, "1r2k2r/p4p1p/2p2p2/4p3/q7/2Q5/PP3PPP/1K1R3R w k - 0 19", 6, "M3 Puzzle #2", "#3");
    // testSearchAndEvaluate(search_agent, "8/2p3N1/pk1p4/1p2p3/4P3/2P5/PP1K4/8 w - - 2 46", 6, "M3 Puzzle #3", "#3");
    // testSearchAndEvaluate(search_agent, "4r1k1/pp3p1p/2p3p1/4b3/3P4/BqP2N2/3R1PPP/6K1 w - - 0 28", 6, "M3 Puzzle #4", "#3");
    // testSearchAndEvaluate(search_agent, "r1b1r1k1/1pp2pp1/p1n1p2p/8/2B1N3/4P3/Pq3PPP/2RQR1K1 w - - 2 18", 6, "M3 Puzzle #5", "#3");
    // testSearchAndEvaluate(search_agent, "3k4/1p6/p1p5/P2p3K/1P1P4/8/8/8 w - - 1 60", 6, "M3 Puzzle #6", "#3");

    // --- Mate in 4 Puzzles ---
    // printSectionHeader("Mate in 4 Puzzles");
    // testSearchAndEvaluate(search_agent, "7k/p7/1p6/8/P7/8/r7/5R1K w - - 1 54", 8, "M4 Puzzle #1", "#4");
    // testSearchAndEvaluate(search_agent, "8/8/k1p5/1p6/pP6/P7/K7/8 w - - 2 64", 8, "M4 Puzzle #2", "#4");
    // testSearchAndEvaluate(search_agent, "k7/8/8/8/8/3R4/p7/7K w - - 0 85", 8, "M4 Puzzle #3", "#4");
    // testSearchAndEvaluate(search_agent, "5r1k/p2Q4/1p1P2p1/4b2p/1P2Pq2/P5P1/5R1P/6K1 b - - 0 46", 8, "M4 Puzzle #4", "#4");
    // testSearchAndEvaluate(search_agent, "8/2p5/8/kp6/1p2N3/pP6/P1K5/8 w - - 10 57", 8, "M4 Puzzle #5", "#4");
    // testSearchAndEvaluate(search_agent, "1R3R2/6pk/1p5p/5p1P/5P2/4n3/8/6K1 w - - 3 52", 8, "M4 Puzzle #6", "#4");

    // --- Mate in 5 Puzzles ---
    // printSectionHeader("Mate in 5 Puzzles");
    // testSearchAndEvaluate(search_agent, "8/8/8/8/k7/p1p1p3/P1P1P3/K7 w - - 0 54", 10, "M5 Puzzle #1", "#5");
    // testSearchAndEvaluate(search_agent, "1k6/1p6/p7/P7/8/8/1p1p2K1/8 w - - 0 59", 10, "M5 Puzzle #2", "#5");
    // testSearchAndEvaluate(search_agent, "8/k7/8/p1p5/P1P5/8/8/K7 w - - 2 73", 10, "M5 Puzzle #3", "#5");
    // testSearchAndEvaluate(search_agent, "8/8/8/p1k5/1p6/1P6/P1K5/8 w - - 2 61", 10, "M5 Puzzle #4", "#5");
    // testSearchAndEvaluate(search_agent, "8/6p1/5p2/5k2/5p2/8/4K3/8 w - - 0 52", 10, "M5 Puzzle #5", "#5");
    // testSearchAndEvaluate(search_agent, "2r3k1/1b5p/p2B2p1/1p2Qp2/2q5/2P2P2/P5PP/4R2K w - - 4 33", 10, "M5 Puzzle #6", "#5");
    
    // --- Mate in 6 Puzzles ---
    // printSectionHeader("Mate in 6 Puzzles");
    // testSearchAndEvaluate(search_agent, "6k1/8/8/8/8/8/P1R2P2/K7 w - - 1 55", 12, "M6 Puzzle #1", "#6");
    // testSearchAndEvaluate(search_agent, "8/8/p1k5/P7/8/8/1K6/8 w - - 1 51", 12, "M6 Puzzle #2", "#6");
    // testSearchAndEvaluate(search_agent, "8/8/8/3p1k2/3P1P2/8/4K3/8 w - - 0 51", 12, "M6 Puzzle #3", "#6");
    // testSearchAndEvaluate(search_agent, "k7/pp6/8/8/8/8/P7/K7 w - - 0 54", 12, "M6 Puzzle #4", "#6");
    // testSearchAndEvaluate(search_agent, "6k1/8/8/K7/8/8/2p5/8 w - - 0 71", 12, "M6 Puzzle #5", "#6");
    // testSearchAndEvaluate(search_agent, "8/4k3/8/8/8/8/p1K5/8 w - - 0 69", 12, "M6 Puzzle #6", "#6");

    std::cout << "\n========== ALL TESTS COMPLETED ==========\n";
    return 0;
}