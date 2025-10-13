
#include "engine/search.h"
#include "chess/movegen.h"
#include "engine/move_orderer.h"
#include <vector>
#include <algorithm>

Search::Search(): nodes_searched(0) { /*Empty Constructor*/ }

chess::Move Search::start_search(Board& board, int depth)
{
    std::vector<chess::Move> moveList;
    MoveGen::init(board, moveList, false);
    chess::Move bestMove{};
    // int legal_moves_found = 0;

    int alpha = NEG_INFINITY_EVAL; 
    int beta = -NEG_INFINITY_EVAL; 

    for(auto& move : moveList)
    {
        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        // legal_moves_found++;
        
        int score = -negamax(board, depth-1, 1, -beta, -alpha);

        if(score > alpha)
        {
            alpha = score;
            bestMove = move;
        }
        board.unmake_move(move);
    }
    return bestMove; // Will be a null move if no legal moves exist
}