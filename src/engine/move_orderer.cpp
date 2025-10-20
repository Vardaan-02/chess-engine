#include "engine/move_orderer.h"
#include "engine/search.h" 
#include "engine/transposition.h"
#include <algorithm> 
#include <cmath>     

const int see_piece_vals[7] = {
    0,     // NO_PIECE_TYPE
    100,   // PAWN
    320,   // KNIGHT
    330,   // BISHOP
    500,   // ROOK
    900,   // QUEEN
    10000  // KING
};

const int HASH_MOVE_BONUS = 20000;
const int CAPTURE_BONUS = 10000; 
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
        else if (v.flags() & (chess::FLAG_CAPTURE | chess::FLAG_EP | chess::FLAG_CAPTURE_PROMO | chess::FLAG_PROMO))
        {
            int64_t see_score = this->see(B, v);

            if (see_score >= 0) {
                score += CAPTURE_BONUS + see_score;
            } else {
                score += see_score; 
            }
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
    return {};
}


static chess::Piece get_least_valuable_attacker(const Board& b, chess::Square sq, chess::Color side, uint64_t all_attackers, uint64_t occupied) {
    
    const bool is_white = (side == chess::WHITE);

    // 1. Pawns
    uint64_t pawn_attackers = chess::PawnAttacks[is_white ? chess::BLACK : chess::WHITE][sq] 
                            & b.bitboard[chess::make_piece(side, chess::PAWN)] & all_attackers;
    if (pawn_attackers) {
        return chess::make_piece(side, chess::PAWN);
    }

    // 2. Knights
    uint64_t knight_attackers = chess::KnightAttacks[sq] 
                              & b.bitboard[chess::make_piece(side, chess::KNIGHT)] & all_attackers;
    if (knight_attackers) {
        return chess::make_piece(side, chess::KNIGHT);
    }

    // 3. Bishops
    uint64_t bishop_attackers = chess::get_diagonal_slider_attacks(sq, occupied) 
                              & b.bitboard[chess::make_piece(side, chess::BISHOP)] & all_attackers;
    if (bishop_attackers) {
        return chess::make_piece(side, chess::BISHOP);
    }

    // 4. Rooks
    uint64_t rook_attackers = chess::get_orthogonal_slider_attacks(sq, occupied) 
                            & b.bitboard[chess::make_piece(side, chess::ROOK)] & all_attackers;
    if (rook_attackers) {
        return chess::make_piece(side, chess::ROOK);
    }

    // 5. Queens
    uint64_t queen_attackers = (chess::get_diagonal_slider_attacks(sq, occupied) | chess::get_orthogonal_slider_attacks(sq, occupied)) 
                             & b.bitboard[chess::make_piece(side, chess::QUEEN)] & all_attackers;
    if (queen_attackers) {
        return chess::make_piece(side, chess::QUEEN);
    }

    // 6. King
    uint64_t king_attackers = chess::KingAttacks[sq] 
                            & b.bitboard[chess::make_piece(side, chess::KING)] & all_attackers;
    if (king_attackers) {
        return chess::make_piece(side, chess::KING);
    }
    
    return chess::NO_PIECE;
}


int64_t MoveOrderer::see(const Board& board, chess::Move move) const {
    int64_t gain[32]; 
    int d = 0;        

    const chess::Square from_sq = (chess::Square)move.from();
    const chess::Square to_sq = (chess::Square)move.to();
    
    chess::Piece moving_piece = board.board_array[from_sq];
    chess::Piece captured_piece = (move.flags() & chess::FLAG_EP) ? 
        (board.white_to_move ? chess::BP : chess::WP) : 
        board.board_array[to_sq];

    if (move.flags() & chess::FLAG_PROMO) {
        moving_piece = (chess::Piece)move.promo();
    }

    else if (move.flags() == chess::FLAG_PROMO) {
         captured_piece = chess::NO_PIECE;
    }

    uint64_t all_attackers = board.attackers_to(to_sq, true) | board.attackers_to(to_sq, false);
    uint64_t occupied = board.occupied;

    gain[d] = see_piece_vals[chess::type_of(captured_piece)];
    chess::Color side = board.white_to_move ? chess::BLACK : chess::WHITE;

    occupied ^= (ONE << from_sq); 
    all_attackers &= occupied;     

    chess::Piece last_capturing_piece = moving_piece;

    while (true) {
        d++; 
        chess::Piece attacker_piece = get_least_valuable_attacker(board, to_sq, side, all_attackers, occupied);
        
        if (attacker_piece == chess::NO_PIECE) {
            break; 
        }
        
        gain[d] = see_piece_vals[chess::type_of(last_capturing_piece)] - gain[d-1];

        uint64_t attacker_bb = board.bitboard[attacker_piece] & all_attackers;
        chess::Square attacker_from_sq = util::lsb(attacker_bb);
        
        occupied ^= (ONE << attacker_from_sq); 
        all_attackers &= occupied;     
        
        last_capturing_piece = attacker_piece;
        side = (side == chess::WHITE) ? chess::BLACK : chess::WHITE;
    }

    // Correct unroll loop
    while (--d > 0) {
        gain[d-1] = -std::max(-gain[d-1], gain[d]);
    }

    return gain[0];
}