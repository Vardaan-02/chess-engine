// #include <vector>
// #include <algorithm>
// #include "chess/movegen.h"

// class Search;

// class MoveOrderer {
// public:
//     MoveOrderer(const Board& b, int ply, Search& s, bool captureOnly);
//     chess::Move get_next_move();

// private:
//     // REMOVED: std::vector<std::pair<int, chess::Move>> scored_moves;
//     // We will now operate on the move list directly.
//     std::vector<chess::Move> moveList;
//     size_t current_move = 0;
    
//     // We will score moves one by one, so we need access to these.
//     const Board& board;
//     int ply;
//     Search& searcher;
//     chess::Move hash_move;
//     bool captures_only;

//     int score_move(const chess::Move& move); // Helper to score a single move
// };