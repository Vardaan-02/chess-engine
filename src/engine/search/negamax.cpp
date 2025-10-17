#include "engine/search.h"
#include "chess/movegen.h"
#include "engine/move_orderer.h"


// Does not check for 50 move rule, 3 repetitions yet
int64_t Search::negamax(Board& board, int depth, int ply, int64_t alpha, int64_t beta)
{
    //checking every 2047 nodes if the stop search has been set to true
    if ((nodes_searched & 2047) == 0 && stopSearch.load()) {
        return DRAW_EVAL; // Return a neutral score if search is aborted
    }

    if(ply > 0)
    {
        if(board.halfmove_clock >= 100) return DRAW_EVAL;

        // the position can only repeat if there were no pawn pushes or captures
        // half move clock records that number of moves (so we dont have to search the entire undo stack)
        int end_index = board.undo_stack.size(); 
        int rep_count{};
        int start_index = std::max(0, end_index - (int)(board.halfmove_clock));
        for(; start_index < end_index; ++start_index)
        {
            if(board.zobrist_key == board.undo_stack[start_index].zobrist_before) ++rep_count;
            if(rep_count >= 2) return DRAW_EVAL; //2 times already and this is the third time
        }
    }

    TTEntry entry{};
    int64_t og_alpha = alpha;

    if(TT.probe(board.zobrist_key, entry)){
        if(entry.depth >= depth)
        {
            if(entry.bound == TTEntry::EXACT) return entry.score;
            if(entry.bound == TTEntry::LOWER_BOUND) alpha = std::max(alpha, entry.score);
            if(entry.bound == TTEntry::UPPER_BOUND) beta = std::min(beta, entry.score);
        }

        if(alpha >= beta) return entry.score;
    }

    nodes_searched++;    
    // Base case: If we've reached the desired depth, switch to quiescence search.
    if (depth == 0) {
        return search_captures_only(board, ply, alpha, beta);
        // return evaluate(board);
    }
    
    MoveOrderer orderer(board, ply, *this, false);
    chess::Move move;
    chess::Move best_move;

    int legal_moves_found = 0;
    
    // Main search loop
    while(!(move = orderer.get_next_move()).is_null()){
        if(stopSearch.load()) return DRAW_EVAL;

        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        
        // If we get here, we've found at least one legal move.
        legal_moves_found++;

        int64_t score = -negamax(board, depth - 1, ply+1, -beta, -alpha);
        board.unmake_move(move);

        if (score >= beta) {
            if(move.flags() != chess::FLAG_CAPTURE && move.flags() != chess::FLAG_CAPTURE_PROMO && move.flags() != chess::FLAG_EP && move.flags() != chess::FLAG_PROMO) 
            {
                update_killers(ply, move);
                update_history(board, move, depth); //need to implement some sort of ageing mechanism so that the quiet moves dont just take over
            }

            entry = { board.zobrist_key, (uint8_t)depth, score, TTEntry::LOWER_BOUND, move };
            TT.store(entry);

            return beta; // Beta-cutoff, a huge performance gain.
        }
        if (score > alpha) {
            best_move = move;
            alpha = score; // We've found a new best move.
        }
    }
    
    // After checking all moves, if we found no legal ones, it's mate or stalemate.
    if (legal_moves_found == 0) {
        // checkmate + ply to favor checkmates found with least amount of moves
        entry = { board.zobrist_key, (int8_t)MAX_PLY, board.checks ? CHECKMATE_EVAL + ply : DRAW_EVAL, TTEntry::EXACT, {} };
        TT.store(entry);
        return board.checks ? CHECKMATE_EVAL + ply : DRAW_EVAL;
    }
    
    TTEntry::Bound bound = (alpha <= og_alpha) ? TTEntry::UPPER_BOUND : TTEntry::EXACT;

    entry = { board.zobrist_key, (uint8_t)depth, alpha, bound, best_move };
    TT.store(entry);

    return alpha;
}