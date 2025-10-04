#pragma once
#include <cstdint>
#include <vector>
#include <utility>
#include "board.h"
#include "move.h"
#include "bitboard_helpers.h"

#define ONE 1ULL
#define NUM_SQUARES 64

// Precomputed leaper moves
extern uint64_t all_king_moves[NUM_SQUARES];
extern uint64_t all_knight_moves[NUM_SQUARES];

// Init tables
void init_leapers();

// ---------------- MoveGen ----------------
class MoveGen {
public:
    // Pawn moves
    static void gen_white_pawn_promotions(int currPawnIndex, int nextPawnIndex, std::vector<MoveType>& moves, int flags, int capturedPiece);
    static void gen_white_pawn_single_push(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_white_pawn_double_push(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_white_pawn_captures(const Chess::Board &B, std::vector<MoveType> &moves);

    static void gen_black_pawn_promotions(int currPawnIndex, int nextPawnIndex, std::vector<MoveType>& moves, int flags, int capturedPiece);
    static void gen_black_pawn_single_push(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_black_pawn_double_push(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_black_pawn_captures(const Chess::Board &B, std::vector<MoveType> &moves);

    static void gen_pawn_moves(const Chess::Board &B, std::vector<MoveType> &moves, bool whiteTurn);

    // Knight moves
    static void gen_white_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_black_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves, bool whiteTurn);

    // King moves
    static void gen_king_moves(const Chess::Board &B, std::vector<MoveType> &moves, bool whiteTurn);
    static void gen_white_king_moves(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_black_king_moves(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_castling_moves(const Chess::Board &B, std::vector<MoveType> &moves, bool whiteTurn);

    // Capturable pieces
    static std::pair<Move::Piece, bool> get_black_capturable_pieces(const Chess::Board& B, uint64_t targetSquare);
    static std::pair<Move::Piece, bool> get_white_capturable_pieces(const Chess::Board& B, uint64_t targetSquare);
};
