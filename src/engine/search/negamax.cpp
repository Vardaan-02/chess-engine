#include "engine/search.h"
#include "chess/movegen.h"

int Search::negamax(Board& board, int depth, int alpha, int beta)
{
    nodes_searched++;    
    
    // Base case: If we've reached the desired depth, switch to quiescence search.
    if (depth == 0) {
        return search_captures_only(board, alpha, beta);
    }
    
    std::vector<chess::Move> moveList;
    MoveGen::init(board, moveList, false);

    int legal_moves_found = 0;
    
    // Main search loop
    for (auto& move : moveList) {
        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        
        // If we get here, we've found at least one legal move.
        legal_moves_found++;

        int score = -negamax(board, depth - 1, -beta, -alpha);
        board.unmake_move(move);

        if (score >= beta) {
            return beta; // Beta-cutoff, a huge performance gain.
        }
        if (score > alpha) {
            alpha = score; // We've found a new best move.
        }
    }
    
    // After checking all moves, if we found no legal ones, it's mate or stalemate.
    if (legal_moves_found == 0) {
        return board.checks ? CHECKMATE_EVAL : DRAW_EVAL;
    }
    
    return alpha;
}