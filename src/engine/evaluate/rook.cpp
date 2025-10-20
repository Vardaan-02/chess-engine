#include "engine/evaluate.h"
#include "engine/search.h"

void Search::rook_evaluation(const Board& b, int& mg_score, int& eg_score) {
    // 1. White Rooks
    uint64_t white_rooks = b.bitboard[chess::WR];
    while (white_rooks) {
        chess::Square sq = util::pop_lsb(white_rooks);
        // Material Score
        mg_score += eval::eval_data.material_values[chess::ROOK].mg;
        eg_score += eval::eval_data.material_values[chess::ROOK].eg;
        
        // PST
        mg_score += eval::eval_data.psts[chess::ROOK][sq].mg;
        eg_score += eval::eval_data.psts[chess::ROOK][sq].eg;

        // 7th Rank Bonus
        int rook_rank = util::get_rank(sq);
        if (rook_rank == 6){
            mg_score += eval::eval_data.rook_on_7th_bonus.mg;
            eg_score += eval::eval_data.rook_on_7th_bonus.eg;
        }

        // Open and Semi Open files
        int rook_file = util::get_file(sq);
        uint64_t file_mask = chess::files[rook_file];
        bool no_friendly_pawns = (file_mask & b.bitboard[chess::WP]) == 0;
        bool no_enemy_pawns = (file_mask & b.bitboard[chess::BP]) == 0;   

        if (no_friendly_pawns) {
            if (no_enemy_pawns) {
                // Open File
                mg_score += eval::eval_data.rook_on_open_file_bonus.mg;
                eg_score += eval::eval_data.rook_on_open_file_bonus.eg;
            } 
            else {
                // Semi-Open File
                mg_score += eval::eval_data.rook_on_semi_open_file_bonus.mg;
                eg_score += eval::eval_data.rook_on_semi_open_file_bonus.eg;
            }
        }  
        
        // Connected Rooks
        uint64_t rook_attack_mask = chess::get_orthogonal_slider_attacks(sq, b.occupied);
        if (rook_attack_mask & white_rooks){
            mg_score += eval::eval_data.rook_connected_bonus.mg;
            eg_score += eval::eval_data.rook_connected_bonus.eg;
        }

        // Mobility
        uint64_t rook_moves_bitboard = chess::get_orthogonal_slider_attacks(sq, b.occupied) & (~b.white_occupied);
        int rook_moves = util::count_bits(rook_moves_bitboard);
        mg_score += eval::eval_data.mobility_bonus[chess::ROOK][rook_moves].mg;
        eg_score += eval::eval_data.mobility_bonus[chess::ROOK][rook_moves].eg;

        // Space 
        uint64_t rook_attacks_on_black_side = rook_attack_mask & util::black_side_of_board;

        int no_of_square_controlled_by_white = util::count_bits(rook_attacks_on_black_side);

        mg_score += no_of_square_controlled_by_white * eval::eval_data.controlled_square_bonus.mg;
        eg_score += no_of_square_controlled_by_white * eval::eval_data.controlled_square_bonus.eg;
    }
    
    // 2. Black Rooks
    uint64_t black_rooks = b.bitboard[chess::BR];
    while (black_rooks) {
        // Materail Score
        chess::Square sq = util::pop_lsb(black_rooks);
        mg_score -= eval::eval_data.material_values[chess::ROOK].mg;
        eg_score -= eval::eval_data.material_values[chess::ROOK].eg;

        // Positional Score
        int pst_sq = util::flip(sq);
        mg_score -= eval::eval_data.psts[chess::ROOK][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::ROOK][pst_sq].eg;

        // Rook on 7th Rank
        int rook_rank = util::get_rank(sq);
        if (rook_rank == 1){
            mg_score -= eval::eval_data.rook_on_7th_bonus.mg;
            eg_score -= eval::eval_data.rook_on_7th_bonus.eg;
        }
        
        // Open and Semi Open files
        int rook_file = util::get_file(sq);
        uint64_t file_mask = chess::files[rook_file];
        bool no_friendly_pawns = (file_mask & b.bitboard[chess::BP]) == 0;
        bool no_enemy_pawns = (file_mask & b.bitboard[chess::WP]) == 0;   

        if (no_friendly_pawns) {
            if (no_enemy_pawns) {
                // Open File
                mg_score -= eval::eval_data.rook_on_open_file_bonus.mg;
                eg_score -= eval::eval_data.rook_on_open_file_bonus.eg;
            } 
            else {
                // Semi-Open File
                mg_score -= eval::eval_data.rook_on_semi_open_file_bonus.mg;
                eg_score -= eval::eval_data.rook_on_semi_open_file_bonus.eg;
            }
        }

        // Connected Rooks
        uint64_t rook_attack_mask = chess::get_orthogonal_slider_attacks(sq, b.occupied);
        if (rook_attack_mask & black_rooks){
            mg_score -= eval::eval_data.rook_connected_bonus.mg;
            eg_score -= eval::eval_data.rook_connected_bonus.eg;
        }

        // Mobility
        uint64_t rook_moves_bitboard = chess::get_orthogonal_slider_attacks(sq, b.occupied) & (~b.black_occupied);
        int rook_moves = util::count_bits(rook_moves_bitboard);
        mg_score -= eval::eval_data.mobility_bonus[chess::ROOK][rook_moves].mg;
        eg_score -= eval::eval_data.mobility_bonus[chess::ROOK][rook_moves].eg;

        // Space 
        uint64_t rook_attacks_on_white_side = rook_attack_mask & util::white_side_of_board;

        int no_of_square_controlled_by_black = util::count_bits(rook_attacks_on_white_side);

        mg_score -= no_of_square_controlled_by_black * eval::eval_data.controlled_square_bonus.mg;
        eg_score -= no_of_square_controlled_by_black * eval::eval_data.controlled_square_bonus.eg;
    }
}