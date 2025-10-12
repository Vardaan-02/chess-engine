// compile using : g++ -I../include/chess -o perft_test.out perft_test.cpp ../src/chess/*.cpp ../src/chess/movegen/*.cpp

#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip> // For std::fixed and std::setprecision
#include "board.h"
#include "movegen.h"
#include "types.h"
#include "bitboard.h"

// Forward declaration of the main perft function
uint64_t perft(Board& board, int depth);

// The perft function remains unchanged
uint64_t perft(Board& board, int depth) {
    // Base case: If we've reached the desired depth, we've found one leaf node.
    if (depth == 0) {
        return 1ULL;
    }

    std::vector<chess::Move> moveList;
    // Generate all pseudo-legal moves for the current position.
    MoveGen::init(board, moveList, false);

    uint64_t nodes = 0;

    // Iterate through all generated moves
    for (const auto& move : moveList) {
        // Make the move on the board
        uint64_t curPin = board.pinned;
        uint64_t check = board.checks;
        chess::Square currKingSq = (board.white_to_move) ? board.white_king_sq : board.black_king_sq;
        board.make_move(move);

        // After making a move, the king of the side that just moved should
        // not be in check by the opponent. If it is, the move was illegal.
        chess::Square king_sq = board.white_to_move 
            ? board.black_king_sq // Black just moved, check their king
            : board.white_king_sq; // White just moved, check their king
        
        if(curPin || check || move.from() == (int)(currKingSq) || move.flags() == chess::FLAG_EP)
        {
            if(!board.square_attacked(king_sq,board.white_to_move))
            nodes += perft(board, depth - 1);
        }
        else nodes += perft(board, depth - 1);


        // Unmake the move to restore the board to its original state
        board.unmake_move(move);
    }

    return nodes;
}

// ===================================================================
// MODIFIED MAIN FUNCTION
// ===================================================================
int main() {
    struct TestCase {
        std::string fen;
        std::vector<uint64_t> expected;
        std::string name;
    };

    std::vector<TestCase> tests = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
         {20, 400, 8902, 197281, 4865609, 119060324}, "Start Position"},

        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
         {48, 2039, 97862, 4085603, 193690690}, "Kiwipete"},

        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
         {14, 191, 2812, 43238, 674624, 11030083, 178633661}, "Complex Position"},

        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
         {6, 264, 9467, 422333, 15833292}, "Position 4"},

        {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
         {44, 1486, 62379, 2103487, 89941194}, "Position 5"},

        {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
         {46, 2079, 89890, 3894594, 164075551}, "Position 6"}
    };

    bool all_tests_passed = true;
    chess::init(); // Initialize attack tables once

    for (auto& test : tests) {
        Board board;
        board.set_fen(test.fen);
        
        std::cout << "------------------------------------------\n";
        std::cout << "🔍 Testing: " << test.name << "\n";
        std::cout << "   FEN: " << test.fen << "\n";
        std::cout << "------------------------------------------\n";

        bool current_test_passed = true;

        for (size_t depth = 1; depth <= test.expected.size(); ++depth) {
            // Start the timer for this specific depth
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // Calculate perft nodes
            uint64_t nodes = perft(board, (int)depth);
            
            // Stop the timer
            auto end_time = std::chrono::high_resolution_clock::now();
            
            // Calculate duration and nodes per second (NPS)
            std::chrono::duration<double> elapsed = end_time - start_time;
            double nps = (elapsed.count() > 0) ? nodes / elapsed.count() : 0;
            
            // Check if the result is correct
            bool correct = (nodes == test.expected[depth - 1]);

            // Print detailed results for the current depth
            std::cout << "  perft(" << depth << ") = " << nodes 
                      << " | Time: " << std::fixed << std::setprecision(3) << elapsed.count() << "s"
                      << " | NPS: " << (uint64_t)nps
                      << " | Status: " << (correct ? "✅ Passed" : "❌ FAIL") << "\n";

            if (!correct) {
                std::cout << "  Expected: " << test.expected[depth - 1] << "\n";
                all_tests_passed = false;
                current_test_passed = false;
                break; // Stop testing this position after a failure
            }
        }
        
        if (!current_test_passed) {
            std::cout << "\n🔴 Test case FAILED.\n";
        }
        std::cout << "\n";
    }

    std::cout << "==========================================\n";
    if (all_tests_passed) {
        std::cout << "🎉 All perft tests passed successfully! 🟢\n";
    } else {
        std::cout << "❌ Some perft tests failed. 🔴\n";
    }
    std::cout << "==========================================\n";

    return 0;
}