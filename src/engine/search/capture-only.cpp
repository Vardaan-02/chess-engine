// #include "engine/search.h"
// #include "chess/movegen.h"
// #include "engine/move_orderer.h"


// int64_t Search::search_captures_only(Board& board, int ply, int64_t alpha, int64_t beta)
// {   
//     if((ply & 1024) && std::chrono::steady_clock::now() >= searchEndTime) stopSearch.store(true);

//     if(stopSearch.load()) return DRAW_EVAL;

//     TTEntry entry{};
//     int64_t og_alpha = alpha;

//     if(TT.probe(board.zobrist_key, entry)){
//         if(TT.probe(board.zobrist_key, entry)){
            
//             if(entry.bound == TTEntry::EXACT) return entry.score;
//             if(entry.bound == TTEntry::LOWER_BOUND) alpha = std::max(alpha, entry.score);
//             if(entry.bound == TTEntry::UPPER_BOUND) beta = std::min(beta, entry.score);

//             if(alpha >= beta) return entry.score;
//         }
//     }

//     nodes_searched++;
//     int64_t score = evaluate(board);
//     if(score >= beta) return beta;
//     if(score > alpha) alpha = score;

//     MoveOrderer orderer(board, ply, *this, false);
//     chess::Move move{};
//     chess::Move best_move{};

//     while(!(move = orderer.get_next_move()).is_null())
//     {
//         bool checkOrCapture = false;
//         board.make_move(move);
        
//         if(!board.is_position_legal()){
//             board.unmake_move(move);
//             continue;
//         }

//         if(move.flags() == chess::FLAG_CAPTURE || move.flags() == chess::FLAG_CAPTURE_PROMO || move.flags() == chess::FLAG_EP) checkOrCapture = true;
//         if (checkOrCapture && orderer.see(board, move) < 0) {
//             board.unmake_move(move);
//             continue; 
//         }

//         chess::Square opp_king_sq = (board.white_to_move) ? board.black_king_sq : board.white_king_sq;
//         if(board.square_attacked(opp_king_sq, board.white_to_move)) checkOrCapture = true;

//         if(!checkOrCapture){
//             board.unmake_move(move);
//             continue;
//         }
        
//         score = -search_captures_only(board, ply+1, -beta, -alpha);
//         board.unmake_move(move);

//         //Cutoffs deliberately not stored in the Transposition table here to avoid polluting the table
//         if(score >= beta) return beta;
//         if(score > alpha){ 
//             alpha = score;
//             best_move = move;
//         }
//     }

//     entry = { board.zobrist_key, 0, alpha, TTEntry::EXACT, best_move };
//     TT.store(entry);

//     return alpha;
// }

#include "engine/search.h"
#include "chess/movegen.h"
#include "engine/move_orderer.h"


int64_t Search::search_captures_only(Board& board, int ply, int64_t alpha, int64_t beta)
{   
    if((ply & 1024) && std::chrono::steady_clock::now() >= searchEndTime) stopSearch.store(true);

    if(stopSearch.load()) return DRAW_EVAL;

    TTEntry entry{};
    int64_t og_alpha = alpha;

    // --- TT FIX ---
    // Probe for depth 1 or greater (to catch other qsearch/negamax(1) results)
    // if(TT.probe(board.zobrist_key, entry)){
    //     if(entry.depth >= 1) // Changed from > 0
    //     {
    //         if(entry.bound == TTEntry::EXACT) return entry.score;
    //     }
    //     // Note: We don't use bounds from TT in qsearch, only exact scores.
    // }

    nodes_searched++;
    
    // Stand-pat score
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

        if(score >= beta) return beta;
        if(score > alpha){ 
            alpha = score;
            best_move = move;
        }
    }

    // --- TT FIX ---
    // Store qsearch results with depth=1 so negamax(depth=1) can use them.
    entry = { board.zobrist_key, 1, alpha, TTEntry::EXACT, best_move }; // Changed from 0
    TT.store(entry);

    return alpha;
}