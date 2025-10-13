// Compile using: g++ -std=c++17 -I../include/chess -I../include -I../include/utils -o evaluation_test.out evaluation_test.cpp ../src/chess/*.cpp ../src/chess/movegen/*.cpp ../src/utils/threadpool.cpp ../src/engine/search.cpp ../src/engine/evaluate.cpp ../src/engine/search/*.cpp -O3 -march=native -flto -funroll-loops

#include <iostream>
#include <vector>
#include <string>
#include "chess/board.h"
#include "engine/search.h"

// Helper function to run and display a single evaluation test
void testEvaluate(std::string fen, const std::string& description) {
    std::cout << "--- Test: " << description << " ---\n";
    std::cout << "    FEN: " << fen << "\n";

    try {
        Board board;
        board.set_fen(fen);
        // The evaluation function in search.cpp calls get_score_from_white_perspective
        int score = Search::evaluate(board); 
        std::cout << "    Evaluation Score: " << score << "\n";
    } catch (const std::exception& e) {
        std::cerr << "    Error evaluating position: " << e.what() << "\n";
    }
    std::cout << std::endl;
}

// Helper to print section headers for better organization
void printSectionHeader(const std::string& title) {
    std::cout << "\n==================================================\n";
    std::cout << "          " << title << "\n";
    std::cout << "==================================================\n\n";
}

int main() {
    std::cout << "========== CHESS ENGINE EVALUATION TEST SUITE ==========\n";

    // ##################################################################
    // # 1. Sanity and Basic Symmetry Tests
    // ##################################################################
    printSectionHeader("Sanity and Symmetry Tests");

    testEvaluate(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "Initial Position: Should be close to 0."
    );
    testEvaluate(
        "r1bqkbnr/pp1ppppp/2n5/2p5/2P5/2N5/PP1PPPPP/R1BQKBNR w KQkq - 2 3",
        "Symmetrical English Opening: Should be close to 0."
    );

    // ##################################################################
    // # 2. Material Balance Tests
    // ##################################################################
    printSectionHeader("Material Balance Tests");

    testEvaluate(
        "rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "White is up a Queen: Should be a very large positive score."
    );
    testEvaluate(
        "1nbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "White is up a Rook: Should be a large positive score."
    );
     testEvaluate(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN1 b KQkq - 1 1",
        "Black is up a Rook: Should be a large negative score."
    );
    testEvaluate(
        "rnbqkb1r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "White is up a Knight: Should be a positive score."
    );
    testEvaluate(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1",
        "black is up a Pawn: Should be a negetive score (around 100 centipawns)."
    );

    // ##################################################################
    // # 3. Piece-Square Table (PST) Tests
    // ##################################################################
    printSectionHeader("Piece-Square Table (PST) Tests");
    
    testEvaluate(
        "8/8/8/3N4/8/8/8/8 w - - 0 1",
        "Centralized White Knight: Score should be positive from PST."
    );
    testEvaluate(
        "N7/8/8/8/8/8/8/8 w - - 0 1",
        "White Knight on the Rim ('a' file): Should be penalized by PST."
    );
    testEvaluate(
        "8/6b1/8/8/8/8/8/8 b - - 0 1",
        "Black Bishop on long diagonal: Should be a good score for black (negative total)."
    );
    testEvaluate(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        "Rooks on open files: Should be positive due to rook placement."
    );
    testEvaluate(
        "8/8/8/8/8/3K4/8/k7 w - - 0 1",
        "Active King in Endgame: White king is centralized, should give an advantage."
    );

    // ##################################################################
    // # 4. Pawn Structure Tests
    // ##################################################################
    printSectionHeader("Pawn Structure Tests");

    // Note: Your current engine doesn't explicitly evaluate these.
    // When you add these features, these tests will become useful.
    testEvaluate(
        "8/k7/8/3p4/8/3P4/8/K7 w - - 0 1",
        "Isolated Pawns (d3 for White, d5 for Black): May result in penalties for both."
    );
    testEvaluate(
        "8/k7/8/3p4/3P4/8/8/K7 w - - 0 1",
        "Doubled Pawns (d4, d5 for White): Should be a penalty for White."
    );
    testEvaluate(
        "8/k5P1/8/8/8/8/8/K7 w - - 0 1",
        "Passed Pawn (g7 for White): Should be a massive advantage for White."
    );

    // ##################################################################
    // # 5. King Safety Tests
    // ##################################################################
    printSectionHeader("King Safety Tests");

    testEvaluate(
        "rnbq1rk1/ppp2ppp/5n2/3p4/1b1P4/2N2N2/PPPBBPPP/R2Q1RK1 b - - 5 8",
        "Good King Safety for both sides: King safety scores should be minimal."
    );
    testEvaluate(
        "4k3/8/8/8/8/8/PPPPPPPP/RNBQK2R w KQ - 0 1",
        "Exposed White King (No pawn shield on g/h): Should apply a penalty to White."
    );
    testEvaluate(
        "r2q1rk1/ppp1nppp/3p1b2/8/3P3N/6P1/PP2PP1P/R1BQR1K1 b - - 0 14",
        "King under Attack (White's H4 knight attacks g6): Should increase White's attack score."
    );
     testEvaluate(
        "rnb1kbnr/pppp1p1p/6p1/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 1 3",
        "Direct Queen Attack (Scholar's Mate threat): White's attack score should be high."
    );

    // ##################################################################
    // # 6. Specific Feature Tests (Bishop Pair, etc.)
    // ##################################################################
    printSectionHeader("Specific Feature Tests");

    testEvaluate(
        "r1b1kbnr/pppp1ppp/2n5/8/8/2N5/PPPPPPPP/R1BQKBNR w KQkq - 2 4",
        "Black has Bishop Pair: Should be a small bonus for Black (negative score)."
    );
     testEvaluate(
        "rnbqk1nr/pppp1ppp/8/8/1b2P3/2N5/PPP2PPP/R1BQKBNR b KQkq - 2 5",
        "After Bxc3, White gets the Bishop Pair: Position before trade is equal."
    );

    // ##################################################################
    // # 7. Tapered Evaluation (Middlegame vs. Endgame)
    // ##################################################################
    printSectionHeader("Tapered Evaluation (MG vs EG)");

    testEvaluate(
        "8/4k3/8/8/8/8/4N3/4K3 w - - 0 1",
        "Knight in Endgame: K+N vs K is a theoretical draw. Score should be near 0."
    );
    testEvaluate(
        "8/4k3/8/8/8/8/4R3/4K3 w - - 0 1",
        "Rook in Endgame: K+R vs K is a win. Score should be high for White, reflecting Rook's strength in endgame."
    );
    testEvaluate(
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1",
        "Middlegame Phase: Game phase value should be at its max."
    );
    testEvaluate(
        "8/k7/8/8/8/8/8/K7 w - - 0 1",
        "Endgame Phase: Game phase value should be at its minimum (0)."
    );


    // ##################################################################
    // # 8. General Positions from Games
    // ##################################################################
    printSectionHeader("Complex Middlegame Positions");

    testEvaluate(
        "r1bqk2r/pp2bppp/2n1pn2/2p5/2P5/2N2N2/PP1PBPPP/R1BQR1K1 b kq - 5 8",
        "Quiet Catalan Opening: A balanced and complex position, score should be near 0."
    );
    testEvaluate(
        "r3r1k1/pp1q1pp1/1np1bn1p/3p4/3P1B2/P1N1PN2/1PQ2PPP/2R1R1K1 b - - 3 17",
        "Strategic Middlegame: White has space, Black has solid pieces. Should be roughly equal."
    );
    testEvaluate(
        "2q1rr1k/1p1b2p1/p2p3p/P1pP2n1/2P1Pp2/2NBNP2/1P4PP/R5K1 w - - 0 29",
        "Complex Late Middlegame: Black has attacking chances, score should likely be negative."
    );

    std::cout << "\n========== ALL TESTS COMPLETED ==========\n";
    return 0;
}