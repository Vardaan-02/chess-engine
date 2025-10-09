// compile using : g++ -I../include/chess -o perft.out perft.cpp ../src/chess/*.cpp ../src/chess/movegen/*.cpp

#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include "board.h"
#include "movegen.h"
#include "types.h"
#include "bitboard.h"

// Forward declaration
uint64_t perft(Board& board, int depth);

// Helper to convert move to UCI string
std::string to_uci(chess::Move m) {
    std::string move_str;
    move_str += char('a' + (m.from() % 8));
    move_str += char('1' + (m.from() / 8));
    move_str += char('a' + (m.to() % 8));
    move_str += char('1' + (m.to() / 8));
    if (m.flags() & chess::FLAG_PROMO) {
        switch (m.promo()) {
            case chess::QUEEN:  move_str += 'q'; break;
            case chess::ROOK:   move_str += 'r'; break;
            case chess::BISHOP: move_str += 'b'; break;
            case chess::KNIGHT: move_str += 'n'; break;
            default: break;
        }
    }
    return move_str;
}

// ===================================================================
// CORRECTED PERFT FUNCTION (WITH LEGALITY CHECK)
// ===================================================================
uint64_t perft(Board& board, int depth) {
    if (depth == 0) {
        return 1ULL;
    }

    std::vector<chess::Move> moveList;
    MoveGen::init(board, moveList);
    uint64_t nodes = 0;

    for (const auto& move : moveList) {
        board.make_move(move);

        // FIX: Check if the king of the side that just moved is now in check.
        // The side to move has flipped, so !board.white_to_move is the player who just moved.
        chess::Square king_sq = !board.white_to_move ? board.white_king_sq : board.black_king_sq;

        // If the king is attacked by the new player, the move was illegal.
        if (board.square_attacked(king_sq, board.white_to_move)) {
            board.unmake_move(move);
            continue; // Skip this illegal move
        }

        nodes += perft(board, depth - 1);
        board.unmake_move(move);
    }
    return nodes;
}

// ===================================================================
// NEW "DIVIDE" FUNCTION FOR DEBUGGING
// ===================================================================
void perft_divide(Board& board, int depth) {
    if (depth == 0) return;

    std::vector<chess::Move> moveList;
    MoveGen::init(board, moveList);
    uint64_t total_nodes = 0;

    for (const auto& move : moveList) {
        board.make_move(move);

        // We must also perform the legality check here for the depth 1 moves.
        chess::Square king_sq = !board.white_to_move ? board.white_king_sq : board.black_king_sq;
        if (board.square_attacked(king_sq, board.white_to_move)) {
            board.unmake_move(move);
            continue;
        }

        // Call the corrected perft on the resulting position
        uint64_t nodes = perft(board, depth - 1);
        total_nodes += nodes;
        std::cout << to_uci(move) << ": " << nodes << std::endl;

        board.unmake_move(move);
    }

    std::cout << "\nNodes searched: " << total_nodes << std::endl;
}

// ===================================================================
// MODIFIED MAIN TO RUN THE DIVIDE TEST
// ===================================================================
int main() {
    chess::init(); // Initialize attack tables once

    Board board;
    std::string fen = "8/8/3p4/1Pp4r/1K3p2/6k1/4P1P1/1R6 w - c6 0 3";
    board.set_fen(fen);

    // Call the divide function for depth 5
    perft_divide(board, 1);

    return 0;
}