// Does not sort the entire list at once, may be good for shallow nodes like for Q search

// #include "engine/move_orderer.h"
// #include "engine/search.h" 
// #include "engine/transposition.h"

// // Constants remain the same
// const int piece_vals[7] = {0, 100, 320, 330, 500, 900, 0};
// const int HASH_MOVE_BONUS = 20000;
// const int CAPTURE_BONUS = 5000; // Includes promotions and EP
// const int KILLER_BONUS = 900;

// // NEW constructor: Just generates moves, doesn't score or sort them yet.
// // In move_orderer.cpp

// MoveOrderer::MoveOrderer(const Board& B, int ply, Search& s, bool capturesOnly)
//     : board(B), ply(ply), searcher(s), captures_only(capturesOnly), current_move(0) // Also initialize current_move
// {
//     MoveGen::init(board, moveList, captures_only);
    
//     TTEntry entry{};
//     if (searcher.TT.probe(board.zobrist_key, entry)) {
//         hash_move = entry.best_move;
//     }
// }

// // NEW helper function to score a single move
// int MoveOrderer::score_move(const chess::Move& move) {
//     if (move.m == hash_move.m) {
//         return HASH_MOVE_BONUS;
//     }
    
//     if (move.flags() & chess::FLAG_CAPTURE) {
//         chess::PieceType victim = (move.flags() & chess::FLAG_EP) ? chess::PAWN : chess::type_of(board.board_array[move.to()]);
//         chess::PieceType attacker = chess::type_of(board.board_array[move.from()]);
//         return CAPTURE_BONUS + (piece_vals[victim] - piece_vals[attacker]);
//     }
    
//     if (move.flags() & chess::FLAG_PROMO) {
//         // Simple but effective bonus for any promotion
//         return CAPTURE_BONUS + piece_vals[chess::type_of((chess::Piece)move.promo())];
//     }
    
//     // It's a quiet move
//     if (searcher.killer_moves[ply][0].m == move.m || searcher.killer_moves[ply][1].m == move.m) {
//         return KILLER_BONUS;
//     }
    
//     // return searcher.history_scores[board.board_array[move.from()]][move.to()];
//     return 0;
// }

// // REWRITTEN get_next_move: Finds the best remaining move and returns it.
// chess::Move MoveOrderer::get_next_move() {
//     if (current_move >= moveList.size()) {
//         return {}; // No moves left
//     }

//     int best_score = -1;
//     size_t best_index = current_move;

//     // Find the move with the highest score from the current position onwards
//     for (size_t i = current_move; i < moveList.size(); ++i) {
//         int score = score_move(moveList[i]);
//         if (score > best_score) {
//             best_score = score;
//             best_index = i;
//         }
//     }

//     // Swap the best move found to the current position in the list
//     std::swap(moveList[current_move], moveList[best_index]);
    
//     // Return the best move and advance the index
//     return moveList[current_move++];
// }