
#include "engine/search.h"
#include "chess/movegen.h"
#include <vector>
#include <algorithm>

#define DRAW_EVAL 0
#define CHECKMATE_EVAL -30000

Search::Search(): nodes_searched(0) { /*Empty Constructor*/ }

int Search::evaluate(const Board& b) {
    int score = b.material_white - b.material_black;
    return b.white_to_move ? score : -score;
}

// returns true for legal position
bool is_position_legal(Board& board)
{
      chess::Square king_sq = board.white_to_move
            ? board.black_king_sq // Black just moved, check their king
            : board.white_king_sq; // White just moved, check their king

        // square_attacked checks if the opponent is attacking the king's square.
    return !(board.square_attacked(king_sq, board.white_to_move));
}

chess::Move Search::start_search(Board& board, int depth)
{
    std::vector<chess::Move> moveList;
    MoveGen::init(board, moveList, false);
    chess::Move bestMove{};
    int legal_moves_found = 0;

    int alpha = CHECKMATE_EVAL - 1; 
    int beta = -CHECKMATE_EVAL + 1; 

    for(auto& move : moveList)
    {
        board.make_move(move);
        if(!is_position_legal(board)){
            board.unmake_move(move);
            continue;
        }
        legal_moves_found++;

        int score = -negamax(board, depth-1, -beta, -alpha);
        
        //std::cout << util::move_to_string(move) << ' ' << score << '\n';
        if(score > alpha)
        {
            alpha = score;
            bestMove = move;
        }
        board.unmake_move(move);
    }
    return bestMove; // Will be a null move if no legal moves exist
}

int Search::negamax(Board& board, int depth, int alpha, int beta)
{
    nodes_searched++;    
    
    std::vector<chess::Move> moveList;
    MoveGen::init(board, moveList, false);

    int legal_moves_found = 0;
    
    for (auto& move : moveList) {
        board.make_move(move);
        if(!is_position_legal(board)){
            board.unmake_move(move);
            continue;
        }
        legal_moves_found++;
        board.unmake_move(move);
        break;  //move found so its not a checkmate/draw position
    }

    // Now, we can reliably check for mate or stalemate.
    if (legal_moves_found == 0) {
        return board.checks ? CHECKMATE_EVAL : DRAW_EVAL;
    }
    
    if (depth == 0) {
        return search_captures_only(board, alpha, beta);
    }
    
    // Now perform the actual search over the filtered legal moves
    for (auto& move : moveList) {
        board.make_move(move);
        if(!is_position_legal(board)){
            board.unmake_move(move);
            continue;
        }

        int score = -negamax(board, depth - 1, -beta, -alpha);
        board.unmake_move(move);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    
    return alpha;
}

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
        if(!is_position_legal(board)){
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