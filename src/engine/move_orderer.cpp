#include "engine/move_orderer.h"
#include "engine/search.h" 
#include "engine/transposition.h"

const int piece_vals[7] = {0, 100, 320, 330, 500, 900, 0}; //null, P, N, B, R, Q, K
const int HASH_MOVE_BONUS = 20000;
const int CAPTURE_BONUS = 5000;
const int KILLER_BONUS = 900;

MoveOrderer::MoveOrderer(const Board& B, int ply, Search& s, bool capturesOnly)
{
    chess::Move best_move{};
    TTEntry entry{};
    if(s.TT.probe(B.zobrist_key, entry))
    {
        best_move = entry.best_move;
    }

    std::vector<chess::Move> moveList;
    MoveGen::init(B, moveList, capturesOnly);
    score_moves(B,ply,s,moveList,best_move);

    std::sort(scored_moves.begin(), scored_moves.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
}

void MoveOrderer::score_moves(const Board& B, int ply, Search& s, std::vector<chess::Move>& moveList, const chess::Move& best_move){
    for(auto& v : moveList)
    {
        int score{};
        if(v.m == best_move.m)
        {
            score += HASH_MOVE_BONUS;
        }
        else if(v.flags() == (chess::FLAG_PROMO) || v.flags() == (chess::FLAG_CAPTURE_PROMO))
        {
            static const int promoBonus[] = {0, 0, 5200, 5100, 5400, 6000, 0};  //Null, P, N, B, R, Q, K
            chess::PieceType promotedPieceType = chess::type_of((chess::Piece)v.promo());

            score += promoBonus[promotedPieceType];

            if(v.flags() == chess::FLAG_CAPTURE_PROMO)
            {
                chess::PieceType victim = chess::type_of(B.board_array[v.to()]);
                chess::PieceType attacker = chess::type_of(B.board_array[v.from()]);

                score += CAPTURE_BONUS + (piece_vals[victim] - piece_vals[attacker]);
            }
        }
        else if(v.flags() == chess::FLAG_CAPTURE || v.flags() == chess::FLAG_EP)
        {
            chess::PieceType victim = (v.flags() == chess::FLAG_EP) ? chess::PAWN : chess::type_of(B.board_array[v.to()]);
            chess::PieceType attacker = chess::type_of(B.board_array[v.from()]);

            score += CAPTURE_BONUS + (piece_vals[victim] - piece_vals[attacker]);
        }
        else{
            if((s.killer_moves[ply][0].m == v.m) || (s.killer_moves[ply][1].m == v.m))
            {
                score += KILLER_BONUS;
            }
            else{
                // score += s.history_scores[B.board_array[v.from()]][v.to()];
            }
        }
        scored_moves.push_back({score, v});
    }
}

chess::Move MoveOrderer::get_next_move() {
    if (current_move < scored_moves.size()) {
        return scored_moves[current_move++].second;
    }
    return {}; // Return a null move when done
}

