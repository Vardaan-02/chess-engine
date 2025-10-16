
#include "engine/search.h"
#include "chess/movegen.h"
#include "engine/move_orderer.h"
#include <vector>
#include <algorithm>

Search::Search(size_t size_of_tt_mb): nodes_searched(0), TT(size_of_tt_mb) { /*Empty Constructor*/ }

chess::Move Search::start_search(Board& board, int depth)
{
    TT.clear();
    nodes_searched = 0; // Also reset statistics

    std::vector<chess::Move> moveList;
    MoveGen::init(board, moveList, false);
    chess::Move bestMove{};
    // int legal_moves_found = 0;

<<<<<<< HEAD
    int64_t alpha = NEG_INFINITY_EVAL; 
    int64_t beta = -NEG_INFINITY_EVAL; 
    // std::cout << moveList.size() << '\n';
=======
    int alpha = CHECKMATE_EVAL-1; 
    int beta = -CHECKMATE_EVAL+1; 

>>>>>>> fbd576e (minor changes)
    for(auto& move : moveList)
    {
        
        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        // legal_moves_found++;
        
        int64_t score = -negamax(board, depth-1, 1, -beta, -alpha);

        std::cout << util::move_to_string(move) << ": " << score << '\n';

        if(score > alpha)
        {
            alpha = score;
            bestMove = move;
        }
        board.unmake_move(move);
    }
    return bestMove; // Will be a null move if no legal moves exist
}