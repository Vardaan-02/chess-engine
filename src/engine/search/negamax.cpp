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

        int end_index = board.undo_stack.size(); 
        int rep_count{};
        int start_index = std::max(0, end_index - (int)(board.halfmove_clock));
        for(; start_index < end_index; ++start_index)
        {
            if(board.zobrist_key == board.undo_stack[start_index].zobrist_before) ++rep_count;
            if(rep_count >= 2) return DRAW_EVAL; 
        }
    }

    if (board.checks) {
        depth++;
    }

    TTEntry entry{};
    int64_t og_alpha = alpha;
    chess::Move best_move_from_tt; // Store TT move

    if(TT.probe(board.zobrist_key, entry)){
        if(entry.depth >= depth)
        {
            if(entry.bound == TTEntry::EXACT) return entry.score;
            if(entry.bound == TTEntry::LOWER_BOUND) alpha = std::max(alpha, entry.score);
            if(entry.bound == TTEntry::UPPER_BOUND) beta = std::min(beta, entry.score);
        }
        best_move_from_tt = entry.best_move; // Get TT move for ordering
        if(alpha >= beta) return entry.score;
    }

    if (!board.checks && ply > 0 && depth > 2 && (board.white_to_move ? board.material_white > 3000 : board.material_black > 3000)) {
        int R = 3;
        board.make_move({});
        int64_t null_score = -negamax(board, depth - 1 - R, ply + 1, -beta, -beta + 1);
        board.unmake_move({}); 

        if (null_score >= beta) {
            return beta;
        }
    }

    nodes_searched++;    
    if (depth == 0) {
        return search_captures_only(board, ply, alpha, beta);
    }
    
    // Orderer will use best_move_from_tt if it's valid
    MoveOrderer orderer(board, ply, *this, false);
    chess::Move move;
    chess::Move best_move = best_move_from_tt; // Initialize with TT move

    int legal_moves_found = 0;
    
    while(!(move = orderer.get_next_move()).is_null()){
        if(stopSearch.load()) return DRAW_EVAL;

        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        
        legal_moves_found++;
        int64_t score;

        // --- CORRECT PVS (Principal Variation Search) ---
        if (legal_moves_found == 1) {
            // 1. First Move (PV): Search with the full window.
            score = -negamax(board, depth - 1, ply + 1, -beta, -alpha);
        
        } else {
            // 2. Subsequent Moves: Assume they are worse. Search with a "null window".
            
            // --- LMR (Late Move Reduction) ---
            int reduction = 0;
            if (legal_moves_found > 5 && depth > 4 && move.flags() == chess::FLAG_QUIET && !board.checks) {
                reduction = std::min(4, 1 + depth / 5); 
            }
            // ---------------------------------

            score = -negamax(board, depth - 1 - reduction, ply + 1, -alpha - 1, -alpha);

            // 3. Re-search: If the null window failed high, re-search with the full window.
            if (score > alpha && score < beta) {
                score = -negamax(board, depth - 1, ply + 1, -beta, -alpha);
            }
        }
        // --- END PVS ---

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
            best_move = move; // This is our new best move in this node
            alpha = score; 
        }
    }
    
    if (legal_moves_found == 0) {
        int64_t final_score = board.checks ? (CHECKMATE_EVAL + ply) : DRAW_EVAL;
        entry = { board.zobrist_key, (int8_t)MAX_PLY, final_score, TTEntry::EXACT, {} };
        TT.store(entry);
        return final_score;
    }
    
    TTEntry::Bound bound = (alpha <= og_alpha) ? TTEntry::UPPER_BOUND : TTEntry::EXACT;

    entry = { board.zobrist_key, (uint8_t)depth, alpha, bound, best_move };
    TT.store(entry);

    return alpha;
}