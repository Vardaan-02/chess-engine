#pragma once
#include <cstdint>
#include <vector>
#include "board.h"
#include "move.h"
#include "bitboard_helpers.h"

// ---------------- Constants ----------------
constexpr int NUM_SQUARES = 64;

// ---------------- Precomputed attack tables ----------------
extern uint64_t all_king_moves[NUM_SQUARES];
extern uint64_t all_knight_moves[NUM_SQUARES];

// ---------------- Initialization ----------------
void init_leapers(); // initialize knight & king attack tables
uint64_t all_king_moves_from(int sq);
uint64_t all_knight_moves_from(int sq);

// ---------------- Move Generation ----------------
class MoveGen
{
public:
    // Generate pseudo-legal moves for the current Chess::Board
    static void generate_pseudo_legal(const Chess::Board &B, std::vector<MoveType> &moves);

private:
    // Individual piece generators
    static void gen_pawn_moves(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_king_moves(const Chess::Board &B, std::vector<MoveType> &moves);

    // Helper
    static void gen_white_pawn_single_push(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_white_pawn_double_push(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_white_pawn_captures(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_black_pawn_single_push(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_black_pawn_double_push(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_black_pawn_captures(const Chess::Board &B, std::vector<MoveType> &moves);

    static void gen_white_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_black_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves);

    static void gen_white_king_moves(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_black_king_moves(const Chess::Board &B, std::vector<MoveType> &moves);
    static void gen_castling_moves(const Chess::Board &B, std::vector<MoveType> &moves);

    static std::pair<Move::Piece, bool> get_white_capturable_pieces(const Chess::Board& B, uint64_t targetSquare);
    static std::pair<Move::Piece, bool> get_black_capturable_pieces(const Chess::Board& B, uint64_t targetSquare);
    static void gen_white_pawn_promotions(int currPawnIndex, int nextPawnIndex, std::vector<MoveType>& moves, int flags, int capturedPiece);
    static void gen_black_pawn_promotions(int currPawnIndex, int nextPawnIndex, std::vector<MoveType>& moves, int flags, int capturedPiece);
};

// ---------------- Legal move filter ----------------
std::vector<MoveType> generate_legal_moves(const Chess::Board &B);
bool is_square_attacked(const Chess::Board &B, int sq, bool by_white);
