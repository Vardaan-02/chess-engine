// compile using : g++ -I../include/chess -o perft_test.out perft_test.cpp ../src/chess/*.cpp ../src/chess/movegen/*.cpp

#include <iostream>
#include <vector>
#include <chrono>
#include "board.h"
#include "movegen.h"
#include "types.h"
#include "bitboard.h"

// Forward declaration of the main perft function
uint64_t perft(Board& board, int depth);

// A helper function to run perft and print results for a specific FEN and depth.
void run_perft_test(const std::string& fen, int max_depth) {
    // Initialize the pre-computed attack tables
    Board board;
    board.set_fen(const_cast<std::string&>(fen));

    chess::init();

    std::cout << "------------------------------------------\n";
    std::cout << "Position: " << fen << "\n";
    std::cout << "------------------------------------------\n";

    for (int depth = 1; depth <= max_depth; ++depth) {
        auto start_time = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perft(board, depth);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> elapsed = end_time - start_time;
        double nps = (elapsed.count() > 0) ? nodes / elapsed.count() : 0;

        std::cout << "perft(" << depth << ") = " << nodes 
                  << " | Time: " << std::fixed << elapsed.count() << "s"
                  << " | NPS: " << (uint64_t)nps << "\n";
    }
    std::cout << "\n";
}

/**
 * @brief The core recursive perft function.
 * * This function is the heart of the performance test. It explores the move
 * tree to a specified depth and counts the number of leaf nodes (final positions).
 * This is the ultimate test of the correctness of the make_move and unmake_move
 * functions.
 * * @param board The current board state.
 * @param depth The remaining depth to search.
 * @return The number of leaf nodes found.
 */
uint64_t perft(Board& board, int depth) {
    // Base case: If we've reached the desired depth, we've found one leaf node.
    if (depth == 0) {
        return 1ULL;
    }

    std::vector<chess::Move> moveList;
    // Generate all pseudo-legal moves for the current position.
    MoveGen::init(board, moveList);

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
        
        if (!board.square_attacked(king_sq, board.white_to_move)) {
            // board.print_board();
            nodes += perft(board, depth - 1);
        }

        // Unmake the move to restore the board to its original state
        board.unmake_move(move);
    }

    return nodes;
}

int main() {
    // Standard test cases with known correct results.
    // Use these to verify your move generator.

    // Test Case 1: Starting Position
    // Expected: 1=20, 2=400, 3=8902, 4=197281, 5=4865609
    std::string start_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Board newBoard; newBoard.set_fen(start_pos);
    newBoard.print_board();
    run_perft_test(start_pos, 4);

    // Board board;
    // board.set_fen(const_cast<std::string&>(start_pos));
    // uint64_t attacks = chess::get_orthogonal_slider_attacks(chess::A1, board.occupied);
    // chess::print_bitboard(attacks);
    // attacks = chess::get_diagonal_slider_attacks(chess::C1, board.occupied);
    // chess::print_bitboard(attacks);

    // Test Case 2: "Kiwipete" - Good for testing complex captures and checks
    // Expected: 1=48, 2=2039, 3=97862, 4=4085603
    // std::string kiwipete_pos = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    // run_perft_test(kiwipete_pos, 4);

    // Test Case 3: Position with castling, en passant, and promotion potential
    // Expected: 1=14, 2=191, 3=2812, 4=43238, 5=674624
    // std::string pos3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    // run_perft_test(pos3, 5);

    return 0;
}