#include "engine/evaluate.h"
#include "engine/search.h"

void Search::queen_evaluation(const Board& b, int& mg_score, int& eg_score) {
    // 1. White Queens
    uint64_t white_queens = b.bitboard[chess::WQ];
    while (white_queens) {
        chess::Square sq = util::pop_lsb(white_queens);

        // Material
        mg_score += eval::eval_data.material_values[chess::QUEEN].mg;
        eg_score += eval::eval_data.material_values[chess::QUEEN].eg;
        
        // PST(s)
        mg_score += eval::eval_data.psts[chess::QUEEN][sq].mg;
        eg_score += eval::eval_data.psts[chess::QUEEN][sq].eg;

        uint64_t attack_mask = chess::get_diagonal_slider_attacks(sq, b.occupied) | chess::get_orthogonal_slider_attacks(sq, b.occupied);
        attack_mask = attack_mask & (!b.white_occupied);

        // Mobility
        int queen_moves = util::count_bits(attack_mask);
        mg_score += eval::eval_data.mobility_bonus[chess::QUEEN][queen_moves].mg;
        eg_score += eval::eval_data.mobility_bonus[chess::QUEEN][queen_moves].eg;
        
        // Space 
        uint64_t queen_attacks_on_black_side = attack_mask & util::black_side_of_board;

        int no_of_square_controlled_by_white = util::count_bits(queen_attacks_on_black_side);

        mg_score += no_of_square_controlled_by_white * eval::eval_data.controlled_square_bonus.mg;
        eg_score += no_of_square_controlled_by_white * eval::eval_data.controlled_square_bonus.eg;
    }
    
    // 2. Black Queens
    uint64_t black_queens = b.bitboard[chess::BQ];
    while (black_queens) {
        chess::Square sq = util::pop_lsb(black_queens);

        // Material
        mg_score -= eval::eval_data.material_values[chess::QUEEN].mg;
        eg_score -= eval::eval_data.material_values[chess::QUEEN].eg;

        int pst_sq = util::flip(sq);

        // PST(s)
        mg_score -= eval::eval_data.psts[chess::QUEEN][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::QUEEN][pst_sq].eg;

        uint64_t attack_mask = chess::get_diagonal_slider_attacks(sq, b.occupied) | chess::get_orthogonal_slider_attacks(sq, b.occupied);
        attack_mask = attack_mask & (~b.black_occupied);
        
        // Mobility
        int queen_moves = util::count_bits(attack_mask);
        mg_score -= eval::eval_data.mobility_bonus[chess::QUEEN][queen_moves].mg;
        eg_score -= eval::eval_data.mobility_bonus[chess::QUEEN][queen_moves].eg;
        
        // Space 
        uint64_t queen_attacks_on_white_side = attack_mask & util::white_side_of_board;
        int no_of_square_controlled_by_black = util::count_bits(queen_attacks_on_white_side);

        mg_score -= no_of_square_controlled_by_black * eval::eval_data.controlled_square_bonus.mg;
        eg_score -= no_of_square_controlled_by_black * eval::eval_data.controlled_square_bonus.eg;
    }
}
