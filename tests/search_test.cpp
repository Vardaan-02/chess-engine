// Compile using: g++ -std=c++17 -I../include/chess -I../include -I../include/utils -o search_test.out search_test.cpp ../src/chess/*.cpp ../src/chess/movegen/*.cpp ../src/utils/threadpool.cpp ../src/engine/*.cpp ../src/engine/search/*.cpp -O3 -march=native -flto -funroll-loops

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "chess/board.h"
#include "engine/search.h"

struct TestCase {
    std::string fen;
    int depth;
    std::string best_move_str;
};

void run_test(Search& search, const TestCase& tc) {
    Board b;
    std::string fen_str = tc.fen;
    b.set_fen(fen_str);

    auto start = std::chrono::high_resolution_clock::now();
    //Have to give time left for white, black and increments in ms, the engine takes time/25th or 15 seconds whichever is smaller
    chess::Move bm = search.start_search(b, tc.depth, 10, 10*60*1000,10*60*1000,1000,1000);
    auto end = std::chrono::high_resolution_clock::now();

    std::string found_move_str = util::move_to_string(bm);
    bool passed = (found_move_str == tc.best_move_str);

    std::chrono::duration<double> diff = end - start;
    double nps = (diff.count() > 0) ? search.nodes_searched / diff.count() : 0;

    std::cout << "FEN: " << tc.fen << std::endl;
    b.print_board();
    std::cout << "Depth: " << tc.depth << ", Expected: " << tc.best_move_str << ", Found: " << found_move_str << std::endl;
    std::cout << "Time: " << std::fixed << diff.count() << "s, Nodes: " << search.nodes_searched << ", NPS: " << (long long)nps << std::endl;
    std::cout << "Result: " << (passed ? "PASSED ✅" : "FAILED ❌") << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
}

int main() {
    std::cout << "Running engine tests..." << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

    std::vector<TestCase> tests = {
        // Mate in 1. Requires depth 1. (Legal's Mate pattern)
        // {"r1bqkbnr/p1pp1ppp/1p6/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 2 4", 8, "f3f7"},

        // Mate in 1. Requires depth 1.
        // {"6k1/3br3/1p1p2p1/p1pP4/PPPb2r1/3B4/8/3R1K2 b - - 0 49", 8, "g4g1"},

        // // Mate in 2, depth 4
        // {"6k1/3b4/1p1p2p1/p1pPbr2/P1P3K1/1P6/4r3/3R4 b - - 1 51", 8, "e2f2"}, //e2f2 then f5f2

        // Winning knight.
        // {"rnbqkbnr/p1pppppp/8/1p6/2N1P3/8/PPPP1PPP/R1BQKBNR b KQkq - 0 1", 6, "b5c4"},

        // Passed Pawn.
        // {"8/5ppp/1P5k/8/8/6P1/5PKP/8 w - - 0 1", 7, "b6b7"},

        //rook takes bishop kyu ho raha ;-;
        // {"6k1/p4pp1/8/1p2P2P/4R3/2Br3P/1Pr2PK1/8 b - - 0 34", 60, "a7a5"},

        //Resigns due to illegal move
        // {"8/7p/p5p1/5k2/3P1p2/2N1p3/1p6/7K b - - 1 56",1,"b2b1"}

        //starting position
        // {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 8, "e2e4"}
    };

    Search search_agent(128);
    int passed_count = 0;
    int total_tests = tests.size();

    chess::init(); // Initialize attack tables once

    for (const auto& test : tests) {
        // This is a simplified test runner; for a real one, you'd reset the search_agent
        // or ensure stats like nodes_searched are cleared before each run.
        run_test(search_agent, test);
        // A more advanced version would count passed tests and print a summary.
    }

    std::cout << "Test run finished." << std::endl;

    return 0;
}