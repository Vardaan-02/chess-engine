// Compile using: \
                g++ -std=c++17 -I../include/chess -I../include -I../include/utils \
                -o evaluation_test.out evaluation_test.cpp ../src/chess/*.cpp ../src/chess/movegen/*.cpp \
                ../src/utils/*.cpp ../src/engine/*.cpp ../src/engine/evaluate/*.cpp ../src/engine/search/*.cpp \
                -O3 -march=native -flto -funroll-loops

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
 * @param stockfish_eval The benchmark evaluation from Stockfish.
 */
void testSearchAndEvaluate(Search& search_agent, std::string fen, int depth, const std::string& description, const std::string& stockfish_eval) {
    std::cout << "--- Test: " << description << " ---\n";
    std::cout << "    FEN: " << fen << "\n";
    std::cout << "    Searching to depth: " << depth << "\n";
    std::cout << "    Stockfish 16 (d=20) Evaluation: " << stockfish_eval << "\n\n";

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
        std::cout << "    Your Engine's final evaluation is the 'score cp' value in the last 'info' line above.\n";
        std::cout << "    Compare this with the Stockfish evaluation to gauge performance.\n";

    } catch (const std::exception& e) {
        std::cerr << "    Error during test: " << e.what() << "\n";
    }
    std::cout << std::endl;
}

int main() {
    // --- One-time initialization ---
    Zobrist::init_zobrist_keys(); 
    chess::init();
    Search search_agent(512); // Create a single search agent to use for all tests
    
    std::cout << "========== CHESS ENGINE SEARCH & EVALUATION TEST SUITE ==========\n";

    // --- Original Test Cases ---
    
    printSectionHeader("Opening Position");
    testSearchAndEvaluate(
        search_agent,
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        12, // Search depth
        "Initial Position",
        "+0.2"
    );

    printSectionHeader("Tactical Position (White to move)");
    testSearchAndEvaluate(
        search_agent,
        "r1bqkbnr/ppp2ppp/2np4/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 4",
        12, // Search depth
        "Italian Game - White has a slight advantage",
        "+0.4"
    );

    printSectionHeader("Middlegame Position (Black to move)");
    testSearchAndEvaluate(
        search_agent,
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP2PPP/R2Q1RK1 b - - 2 9",
        12, // Search depth
        "Complex middlegame, requires careful calculation",
        "0.0"
    );
    
    printSectionHeader("Endgame Position (White to move)");
     testSearchAndEvaluate(
        search_agent,
        "8/8/8/p1k5/P7/8/1K6/8 w - - 0 1",
        12, // Search depth
        "King and Pawn Endgame - Should be a draw",
        "0.0"
    );

    // --- Expanded Test Cases ---

    printSectionHeader("Tactics: Forks and Double Attacks");
    testSearchAndEvaluate(search_agent, "r1b3k1/pp3ppp/5n2/2p1q3/2P5/3B4/PP3PPP/R2Q2K1 w - - 0 16", 12, "Queen Fork", "+3.5");
    testSearchAndEvaluate(search_agent, "2r3k1/3n1pp1/3Qrn2/8/5B1q/2pP4/P1R1PPB1/5RK1 w - - 0 1", 12, "Double Attack", "+2.1");

    printSectionHeader("Tactics: Pins and Skewers");
    testSearchAndEvaluate(search_agent, "6rk/1bpq1p1p/p3pb2/8/PnP1P3/3P1Q2/1P3PPP/R1B2RK1 w - - 0 1", 12, "Cross-Pin", "+6.1");
    testSearchAndEvaluate(search_agent, "6k1/p4p2/1p2p3/4P3/2P3r1/1P1K4/P7/3R4 b - - 1 35", 12, "Skewer", "-1.5");
    
    printSectionHeader("Tactics: Discovered Attacks");
    testSearchAndEvaluate(search_agent, "3n4/3k1p1p/B2P2p1/8/2P2P2/P5PP/2P1r3/1R4K1 b - - 0 1", 12, "Discovered Attack", "-3.9");

    printSectionHeader("Mate Finders");
    testSearchAndEvaluate(search_agent, "3r1rk1/p1q2p1p/bp2p1p1/5nNQ/2P3N1/8/PB3PPP/1BR3K1 b - - 0 1", 12, "Mate in 3", "Black to play, #3");
    testSearchAndEvaluate(search_agent, "8/2p1Np2/k2P2p1/2P5/2K2b2/1R6/r7/8 b - - 0 1", 14, "Mate in 4", "Black to play, #4");
    testSearchAndEvaluate(search_agent, "8/8/8/8/8/3Q4/1K6/k7 w - - 0 1", 16, "Mate in 6", "White to play, #6");

    printSectionHeader("Positional and Strategic Tests");
    testSearchAndEvaluate(search_agent, "3r3k/pq1r1ppp/1pb1pn2/3n4/3P4/1BN1BN2/PP1Q1PPP/2R1R1K1 b - -", 12, "Isolated Queen Pawn", "+0.2");
    testSearchAndEvaluate(search_agent, "6k1/p2p3p/5p1q/1Q6/1Q6/2Pp2P1/PP3P1P/6K1 w - -", 12, "Exposed King", "+5.4");
    testSearchAndEvaluate(search_agent, "6k1/pp4p1/1b1pNp1p/3Pp3/2P1P3/8/PP3PPP/6K1 b - -", 12, "Knight Outpost", "+1.8");
    testSearchAndEvaluate(search_agent, "r1b1r1k1/1p3pbp/p1qp1np1/2p1n3/P1P1P3/1PN1B1NP/3QBPP1/2RR2K1 b - -", 12, "Quiet Position (Prophylaxis)", "+0.1");

    printSectionHeader("Theoretical Endgames");
    testSearchAndEvaluate(search_agent, "4K3/4P1k1/8/8/8/8/7R/5r2 w - - 0 1", 16, "Lucena Position (Win)", "+6.5");
    testSearchAndEvaluate(search_agent, "8/4k3/8/4p3/8/3K4/3R4/r7 b - - 0 1", 16, "Philidor Position (Draw)", "0.00");
    testSearchAndEvaluate(search_agent, "8/8/8/8/1k6/8/p1K5/R7 w - - 0 1", 16, "Saavedra Position (Win)", "+M6");
    
    printSectionHeader("Engine Stumpers");
    testSearchAndEvaluate(search_agent, "k1r2b2/1r6/3p1p2/p1pPpPp1/PpP1P1Pp/1P5P/1R6/2K3B1 w - - 0 1", 12, "Completely Blocked Position", "0.00");
    testSearchAndEvaluate(search_agent, "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -", 16, "Deep Zugzwang", "-2.1 (White to move loses)");


    std::cout << "\n========== ALL TESTS COMPLETED ==========\n";
    return 0;
}