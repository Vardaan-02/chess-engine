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
std::vector<uint64_t> perft(Board& board, int depth);

// The perft function remains unchanged
std::vector<uint64_t> perft(Board& board, int depth) {
    // {nodes, captures, ep, checks, double_checks}
    if (depth == 0) {
        return {1ULL, 0ULL, 0ULL, 0ULL, 0ULL};
    }

    std::vector<chess::Move> moveList;
    MoveGen::init(board, moveList);

    uint64_t nodes = 0;
    uint64_t captures = 0;
    uint64_t eps = 0;
    uint64_t checks = 0;
    uint64_t double_checks = 0;

    for (const auto& move : moveList) {
        board.make_move(move);

        // Determine which side just moved
        bool sideJustMovedIsWhite = !board.white_to_move;
        chess::Square enemyKingSq = sideJustMovedIsWhite ? board.black_king_sq : board.white_king_sq;

        // Recurse
        std::vector<uint64_t> val = perft(board, depth - 1);

        // Aggregate results
        nodes += val[0];
        captures += val[1];
        eps += val[2];
        checks += val[3];
        double_checks += val[4];

        // --- Count move-specific stats ---
        if (move.flags() == chess::FLAG_CAPTURE){ 
            captures++;
            // board.print_board(0);
        }
        else if (move.flags() == chess::FLAG_EP) eps++;

        // Check if this move delivered a check
        if (board.checker_bitboard) {
            if (board.inDoubleCheck) double_checks++;
            else checks++;
        }

        board.unmake_move(move);
    }

    return {nodes, captures, eps, checks, double_checks};
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
        // {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        //  {20, 400, 8902, 197281, 4865609, 119060324}, "Start Position"},

        // {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        //  {48, 2039, 97862, 4085603, 193690690}, "Kiwipete"},

        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
         {14, 191, 2812, 43238, 674624, 11030083, 178633661}, "Complex Position"},

        // {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        //  {6, 264, 9467, 422333, 15833292}, "Position 4"},

        // {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        //  {44, 1486, 62379, 2103487, 89941194}, "Position 5"},

        // {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        //  {46, 2079, 89890, 3894594, 164075551}, "Position 6"}

        //  {"8/2p5/3p4/Kr6/5R2/4P1k1/6P1/8 w - - 0 1",
        //  {7}, "Position 6"}
    };

    bool all_tests_passed = true;
    chess::init(); // Initialize attack tables once

    for (auto& test : tests) {
        Board board;
        board.set_fen(test.fen);
        board.print_board(0);
        
        std::cout << "------------------------------------------\n";
        std::cout << "🔍 Testing: " << test.name << "\n";
        std::cout << "   FEN: " << test.fen << "\n";
        std::cout << "------------------------------------------\n";

        bool current_test_passed = true;
        std::vector<uint64_t> old = {0,0,0,0,0};
        for (size_t depth = 1; depth <= test.expected.size(); ++depth) {
            // Start the timer for this specific depth
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // Calculate perft nodes
            std::vector<uint64_t> vec = perft(board, (int)depth);
            uint64_t nodes = vec[0] - old[0];
            uint64_t captures = vec[1] - old[1];
            uint64_t eps = vec[2] - old[2];
            uint64_t checks = vec[3] - old[3];
            uint64_t double_checks = vec[4] - old[4];

            old[1] = captures; old[2] = eps; old[3] = checks; old[4] = double_checks;
            // Stop the timer
            auto end_time = std::chrono::high_resolution_clock::now();
            
            // Calculate duration and nodes per second (NPS)
            std::chrono::duration<double> elapsed = end_time - start_time;
            double nps = (elapsed.count() > 0) ? nodes / elapsed.count() : 0;
            
            // Check if the result is correct
            bool correct = (nodes == test.expected[depth - 1]);

            // Print detailed results for the current depth
            std::cout << "  perft(" << depth << ") = " << nodes 
                      << " | captures: " << captures << " | eps: " << eps << " | checks: " << checks << "| double_checks: " << double_checks
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