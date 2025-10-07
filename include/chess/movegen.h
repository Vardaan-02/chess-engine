#pragma once
#include "types.h"
#include "board.h"
#include "bitboard.h"
#include <vector>
namespace MoveGen {
    std::vector<chess::Move> moveList;

    void generate_pawn_moves(const Board& B, std::vector<chess::Move>& moveList);
};
