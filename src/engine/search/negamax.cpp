#include "engine/search.h"
#include "chess/movegen.h"
#include "engine/move_orderer.h"


int64_t Search::negamax(Board& board, int depth, int ply, int64_t alpha, int64_t beta)
{
    if ((nodes_searched & 1024) == 0 && std::chrono::steady_clock::now() >= searchEndTime) {
            stopSearch.store(true);
        }

    if ((nodes_searched & 1024) == 0 && stopSearch.load()) {
        return DRAW_EVAL;
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

    if (board.checks) {
        depth++;
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

    // Conditions to apply Null move pruning:
    // 1. Not in check.
    // 2. Not in the first few plies of the game.
    // 3. The current search depth is deep enough (e.g., > 2).
    // 4. The side to move has enough non-pawn material (to avoid zugzwang issues).
    if (!board.checks && ply > 0 && depth > 2 && (board.white_to_move ? board.material_white > 3000 : board.material_black > 3000)) {
        // The reduction factor 'R' is typically 2 or 3.
        int R = 3;

        board.make_move({});
        int64_t null_score = -negamax(board, depth - 1 - R, ply + 1, -beta, -beta + 1);
        board.unmake_move({}); 

        //even after making a null move the opp couldnt make our score < Beta so its too good lets prune
        if (null_score >= beta) {
            return beta;
        }
    }

    nodes_searched++;    
    if (depth == 0) {
        return search_captures_only(board, ply, alpha, beta);
    }
    
    MoveOrderer orderer(board, ply, *this, false);
    chess::Move move;
    chess::Move best_move;

    int legal_moves_found = 0;
    
    while(!(move = orderer.get_next_move()).is_null()){
        if(stopSearch.load()) return DRAW_EVAL;

        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        
        legal_moves_found++;

        // int64_t score = -negamax(board, depth - 1, ply+1, -beta, -alpha);
        int64_t score;

        if (legal_moves_found > 5 && depth > 4 && move.flags() == chess::FLAG_QUIET) {
            // After the first few moves, if the move is quiet and depth is sufficient...
            int reduction = std::min(4, 1 + depth / 5);
            // More complex reductions can depend on depth and move number
            // Search with a reduced depth
            score = -negamax(board, depth - 1 - reduction, ply + 1, -alpha - 1, -alpha);
        } 
        else {
            // Ensure the full search is done for the first few moves or if a re-search is needed
            score = -negamax(board, depth - 1, ply + 1, -beta, -alpha);
        }
        // If the reduced search was better than expected, it might be a good move. Re-search at full depth.
        if (score > alpha) {
            score = -negamax(board, depth - 1, ply + 1, -beta, -alpha);
        }

        board.unmake_move(move);

        if (score >= beta) {
            if(move.flags() != chess::FLAG_CAPTURE && move.flags() != chess::FLAG_CAPTURE_PROMO && move.flags() != chess::FLAG_EP && move.flags() != chess::FLAG_PROMO) 
            {
                update_killers(ply, move);
                // update_history(board, move, depth);
            }

            entry = { board.zobrist_key, (uint8_t)depth, score, TTEntry::LOWER_BOUND, move };
            TT.store(entry);

            return beta; 
        }
        if (score > alpha) {
            best_move = move;
            alpha = score; 
        }
    }
    
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