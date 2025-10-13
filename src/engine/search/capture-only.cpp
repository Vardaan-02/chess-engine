#include "engine/search.h"
#include "chess/movegen.h"
#include "engine/move_orderer.h"


int Search::search_captures_only(Board& board, int ply, int alpha, int beta)
{   
    nodes_searched++;
    int score = evaluate(board);
    if(score >= beta) return beta;
    if(score > alpha) alpha = score;

    MoveOrderer orderer(board, ply, *this, true);
    chess::Move move;

    while(!(move = orderer.get_next_move()).is_null())
    {
        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        
        score = -search_captures_only(board, ply+1, -beta, -alpha);
        board.unmake_move(move);

        if(score >= beta) return beta;
        if(score > alpha) alpha = score;
    }

    return alpha;
}