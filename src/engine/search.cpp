
#include "engine/search.h"
#include "chess/movegen.h"
#include <vector>
#include <algorithm>

Search::Search(): nodes_searched(0) { /*Empty Constructor*/ }

std::string square_to_string(int s) {
    std::string str = "";
    str += (char)('a' + (s % 8));
    str += (char)('1' + (s / 8));
    return str;
}

chess::Move Search::start_search(Board& board, int depth)
{
    std::vector<chess::Move> moveList;
    MoveGen::init(board, moveList, false);
    chess::Move bestMove{};
    // int legal_moves_found = 0;

    int alpha = INFINITY_EVAL; 
    int beta = -INFINITY_EVAL; 

    for(auto& move : moveList)
    {
        board.make_move(move);
        if(!board.is_position_legal()){
            board.unmake_move(move);
            continue;
        }
        // legal_moves_found++;
        
        int score = -negamax(board, depth-1, -beta, -alpha);
        
        if(score > alpha)
        {
            alpha = score;
            bestMove = move;
        }
        board.unmake_move(move);
    }
    return bestMove; // Will be a null move if no legal moves exist
}