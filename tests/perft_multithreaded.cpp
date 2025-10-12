// ===================================================================
// Thread-Pooled Performance Test (Perft) for Chess Engine
//
// Description:
// This program calculates the number of legal moves from a given chess
// position up to a certain depth. It uses a thread pool to parallelize
// the search at the root, significantly improving performance on
// multi-core processors. The results are validated against known values
// for several standard test positions.
//
// How it works:
// 1. A ThreadPool class is implemented to manage a set of worker threads.
// 2. The main function sets up a series of test cases (FEN strings).
// 3. For each test, the `perft_threaded` function is called.
// 4. `perft_threaded` generates all legal moves from the root position.
// 5. Each of these moves, along with the resulting board state, is
//    submitted as a task to the thread pool.
// 6. Each worker thread takes a task and runs a recursive `perft` search
//    for the remaining depth.
// 7. The main thread collects the results from all tasks and sums them
//    up to get the final node count.
//
// ===================================================================


// USE TO COMPILE
// g++ -std=c++17 -I../include/chess -I../include/utils -o perft_multithreaded.out perft_multithreaded.cpp ../src/chess/*.cpp ../src/chess/movegen/*.cpp ../src/utils/threadpool.cpp -O3 -march=native -flto -funroll-loops

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <future>
#include <numeric>

#include "board.h"
#include "movegen.h"
#include "types.h"
#include "bitboard.h"
#include "threadpool.h"



// The recursive perft function executed by worker threads.
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
        board.make_move(move);

        // After making a move, the king of the side that just moved should
        // not be in check by the opponent. If it is, the move was illegal.
        chess::Square king_sq = board.white_to_move
            ? board.black_king_sq // Black just moved, check their king
            : board.white_king_sq; // White just moved, check their king

        // square_attacked checks if the opponent is attacking the king's square.
        if (!board.square_attacked(king_sq, board.white_to_move)) {
            nodes += perft(board, depth - 1);
        }

        // Unmake the move to restore the board to its original state for the next iteration.
        board.unmake_move(move);
    }
    return nodes;
}

// Top-level function to divide perft work among threads.
uint64_t perft_threaded(Board& root_board, int depth, ThreadPool& pool) {
    if (depth == 0) return 1ULL;

    std::vector<chess::Move> moveList;
    MoveGen::init(root_board, moveList, false);
    std::vector<std::future<uint64_t>> futures;
    uint64_t total_nodes = 0;

    // Iterate through root moves and create tasks for the thread pool
    for (const auto& move : moveList) {
        Board board_copy = root_board;
        board_copy.make_move(move);

        chess::Square king_sq = board_copy.white_to_move ? board_copy.black_king_sq : board_copy.white_king_sq;
        if (!board_copy.square_attacked(king_sq, board_copy.white_to_move)) {
             // If the move is legal, enqueue the rest of the search.
             // We pass board_copy by value to ensure each thread has its own instance.
            futures.emplace_back(
                pool.enqueue([board_copy, depth]() mutable {
                    return perft(board_copy, depth - 1);
                })
            );
        }
    }

    // Collect results from all futures
    for (auto &future : futures) {
        total_nodes += future.get();
    }
    return total_nodes;
}

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

    chess::init(); // Initialize attack tables once
    
    // Determine the optimal number of threads to use
    unsigned int num_threads = std::max(1u, std::thread::hardware_concurrency());
    ThreadPool pool(num_threads);
    std::cout << "==========================================\n";
    std::cout << "🚀 Starting Perft Test Suite\n";
    std::cout << "   Using " << num_threads << " worker threads.\n";
    std::cout << "==========================================\n\n";

    bool all_tests_passed = true;

    for (auto& test : tests) {
        Board board;
        board.set_fen(test.fen);
        std::cout << "------------------------------------------\n";
        std::cout << "🔍 Testing: " << test.name << "\n";
        std::cout << "   FEN: " << test.fen << "\n";
        std::cout << "------------------------------------------\n";

        bool current_test_passed = true;

        for (size_t depth = 1; depth <= test.expected.size(); ++depth) {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // For shallow depths, using the overhead of the thread pool isn't efficient.
            uint64_t nodes = (depth <= 3)
                ? perft(board, (int)depth)
                : perft_threaded(board, (int)depth, pool);
            
            auto end_time = std::chrono::high_resolution_clock::now();
            
            std::chrono::duration<double> elapsed = end_time - start_time;
            double nps = (elapsed.count() > 0) ? nodes / elapsed.count() : 0;
            
            bool correct = (nodes == test.expected[depth - 1]);

            std::cout << "  perft(" << depth << ") = " << nodes 
                      << " | Time: " << std::fixed << std::setprecision(3) << elapsed.count() << "s"
                      << " | NPS: " << static_cast<uint64_t>(nps)
                      << " | Status: " << (correct ? "✅ Passed" : "❌ FAIL") << "\n";

            if (!correct) {
                std::cout << "  Expected: " << test.expected[depth - 1] << "\n";
                all_tests_passed = false;
                current_test_passed = false;
                break; 
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

