#include "engine/search.h"
#include "chess/movegen.h"
#include "engine/move_orderer.h"


// Does not check for 50 move rule, 3 repetitions yet
int Search::negamax(Board& board, int depth, int ply, int alpha, int beta)
{
    nodes_searched++;    
    
    // Base case: If we've reached the desired depth, switch to quiescence search.
    if (depth == 0) {
        return search_captures_only(board, ply, alpha, beta);
        // return evaluate(board);
    }
    
    MoveOrderer orderer(board, ply, *this, false);
    chess::Move move;

    int legal_moves_found = 0;
    
    // Main search loop
    while(!(move = orderer.get_next_move()).is_null()){
        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        
        // If we get here, we've found at least one legal move.
        legal_moves_found++;

        int score = -negamax(board, depth - 1, ply+1, -beta, -alpha);
        board.unmake_move(move);

        if (score >= beta) {
            if(move.flags() != chess::FLAG_CAPTURE && move.flags() != chess::FLAG_CAPTURE_PROMO && move.flags() != chess::FLAG_EP || move.flags() != chess::FLAG_PROMO) 
            {
                update_killers(ply, move);
                update_history(board, move, depth); //need to implement some sort of ageing mechanism so that the quiet moves dont just take over
            }

            return beta; // Beta-cutoff, a huge performance gain.
        }
        if (score > alpha) {
            alpha = score; // We've found a new best move.
        }
    }
    
    // After checking all moves, if we found no legal ones, it's mate or stalemate.
    if (legal_moves_found == 0) {
        // checkmate + ply to favor checkmates found with least amount of moves
        return board.checks ? CHECKMATE_EVAL + ply : DRAW_EVAL;
    }
    
    return alpha;
}