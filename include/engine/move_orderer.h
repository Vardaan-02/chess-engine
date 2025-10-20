#pragma once

#include <vector>
#include <algorithm>
#include "chess/movegen.h"

class Search;

class MoveOrderer {
public:
    MoveOrderer(const Board& b, int ply, Search& s, bool captureOnly);
    int64_t see(const Board& board, chess::Move move) const;
    chess::Move get_next_move();

private:
    void score_moves(const Board& B, int ply, Search& s, std::vector<chess::Move>& moveList, const chess::Move& best_move);

    std::vector<std::pair<int, chess::Move>> scored_moves;
    size_t current_move = 0;
};