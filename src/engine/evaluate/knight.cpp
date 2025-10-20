#include "engine/evaluate.h"
#include "engine/search.h"

void Search::knight_evaluation(const Board& b, int& mg_score, int& eg_score) {
    // 1. White Knights
    uint64_t white_knights = b.bitboard[chess::WN];
    while (white_knights) {
        chess::Square sq = util::pop_lsb(white_knights);
        
        // Material
        mg_score += eval::eval_data.material_values[chess::KNIGHT].mg;
        eg_score += eval::eval_data.material_values[chess::KNIGHT].eg;
        
        // PST(s)
        mg_score += eval::eval_data.psts[chess::KNIGHT][sq].mg;
        eg_score += eval::eval_data.psts[chess::KNIGHT][sq].eg;

        // Knight Outpost
        uint64_t knight_outpost_square = chess::PawnAttacks[chess::BLACK][sq];
        if (knight_outpost_square & b.bitboard[chess::WP]){
            mg_score += eval::eval_data.knight_outpost_bonus.mg;
            eg_score += eval::eval_data.knight_outpost_bonus.eg;
        }

        // Mobility
        uint64_t knight_moves_bitboard = chess::KnightAttacks[sq] & (~b.white_occupied);
        int knight_moves = util::count_bits(knight_moves_bitboard);
        mg_score += eval::eval_data.mobility_bonus[chess::KNIGHT][knight_moves].mg;
        eg_score += eval::eval_data.mobility_bonus[chess::KNIGHT][knight_moves].eg;

        // Space 
        uint64_t knight_attacks_on_black_side = chess::KnightAttacks[sq] & util::black_side_of_board;

        int no_of_square_controlled_by_white = util::count_bits(knight_attacks_on_black_side);

        mg_score += no_of_square_controlled_by_white * eval::eval_data.controlled_square_bonus.mg;
        eg_score += no_of_square_controlled_by_white * eval::eval_data.controlled_square_bonus.eg;
    }
    
    // 2. Black Knights
    uint64_t black_knights = b.bitboard[chess::BN];
    while (black_knights) {
        chess::Square sq = util::pop_lsb(black_knights);

        // Material
        mg_score -= eval::eval_data.material_values[chess::KNIGHT].mg;
        eg_score -= eval::eval_data.material_values[chess::KNIGHT].eg;

        int pst_sq = util::flip(sq);

        // PST(s)
        mg_score -= eval::eval_data.psts[chess::KNIGHT][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::KNIGHT][pst_sq].eg;

        // knight outpost
        uint64_t knight_outpost_square = chess::PawnAttacks[chess::WHITE][sq];
        if (knight_outpost_square & b.bitboard[chess::BP]){
            mg_score -= eval::eval_data.knight_outpost_bonus.mg;
            eg_score -= eval::eval_data.knight_outpost_bonus.eg;
        }

        // Mobility
        uint64_t knight_moves_bitboard = chess::KnightAttacks[sq] & (~b.black_occupied);
        int knight_moves = util::count_bits(knight_moves_bitboard);
        mg_score -= eval::eval_data.mobility_bonus[chess::KNIGHT][knight_moves].mg;
        eg_score -= eval::eval_data.mobility_bonus[chess::KNIGHT][knight_moves].eg;

        // Space 
        uint64_t knight_attacks_on_white_side = chess::KnightAttacks[sq] & util::white_side_of_board;

        int no_of_square_controlled_by_black = util::count_bits(knight_attacks_on_white_side);

        mg_score -= no_of_square_controlled_by_black * eval::eval_data.controlled_square_bonus.mg;
        eg_score -= no_of_square_controlled_by_black * eval::eval_data.controlled_square_bonus.eg;
    }
}