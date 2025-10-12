#pragma once
#include "types.h"
#include "board.h"
#include "bitboard.h"
#include <vector>
namespace MoveGen {
    extern std::vector<chess::Move> moveList;

    void generate_pawn_moves(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly);
    void generate_knight_moves(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly);
    void generate_king_moves(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly);
    void generate_orthogonal_sliders_moves(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly);
    void generate_diagonal_sliders_moves(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly);

    void init(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly);
};
