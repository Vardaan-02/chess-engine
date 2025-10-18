#include "engine/search.h"
#include "chess/movegen.h"
#include "engine/move_orderer.h"


int64_t Search::search_captures_only(Board& board, int ply, int64_t alpha, int64_t beta)
{   
    if((ply & 1024) && std::chrono::steady_clock::now() >= searchEndTime) stopSearch.store(true);

    if(stopSearch.load()) return DRAW_EVAL;

    TTEntry entry{};
    int64_t og_alpha = alpha;

    if(TT.probe(board.zobrist_key, entry)){
        if(entry.depth > 0)
        {
            //we only care about exact nodes for Qsearch to avoid bad cutoffs
            if(entry.bound == TTEntry::EXACT) return entry.score;
        }

        if(alpha >= beta) return entry.score;
    }

    nodes_searched++;
    int64_t score = evaluate(board);
    if(score >= beta) return beta;
    if(score > alpha) alpha = score;

    MoveOrderer orderer(board, ply, *this, true);
    chess::Move move{};
    chess::Move best_move{};

    while(!(move = orderer.get_next_move()).is_null())
    {
        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        
        score = -search_captures_only(board, ply+1, -beta, -alpha);
        board.unmake_move(move);

        //Cutoffs deliberately not stored in the Transposition table here to avoid polluting the table
        if(score >= beta) return beta;
        if(score > alpha){ 
            alpha = score;
            best_move = move;
        }
    }

    entry = { board.zobrist_key, 0, alpha, TTEntry::EXACT, best_move };
    TT.store(entry);

    return alpha;
}