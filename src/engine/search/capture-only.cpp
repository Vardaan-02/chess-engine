#include "engine/search.h"
#include "chess/movegen.h"

int Search::search_captures_only(Board& board, int alpha, int beta)
{   
    nodes_searched++;
    int score = evaluate(board);
    if(score >= beta) return beta;
    if(score > alpha) alpha = score;

    std::vector<chess::Move> capturesOnlyMoveList;
    MoveGen::init(board, capturesOnlyMoveList, true);

    for(auto& move : capturesOnlyMoveList)
    {
        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        
        score = -search_captures_only(board, -beta, -alpha);
        board.unmake_move(move);

        if(score >= beta) return beta;
        if(score > alpha) alpha = score;
    }

    return alpha;
}