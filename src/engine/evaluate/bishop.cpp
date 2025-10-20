#include "engine/evaluate.h"
#include "engine/search.h"

void Search::bishop_evaluation(const Board& b, int& mg_score, int& eg_score) {
    // 1. White Bishops
    uint64_t white_bishops = b.bitboard[chess::WB];
    while (white_bishops) {
        chess::Square sq = util::pop_lsb(white_bishops);

        // Material
        mg_score += eval::eval_data.material_values[chess::BISHOP].mg;
        eg_score += eval::eval_data.material_values[chess::BISHOP].eg;
        
        // PST(s)
        mg_score += eval::eval_data.psts[chess::BISHOP][sq].mg;
        eg_score += eval::eval_data.psts[chess::BISHOP][sq].eg;

        uint64_t white_pawns_of_bishop_color;
        if (util::create_bitboard_from_square(sq) & util::black_squares) white_pawns_of_bishop_color = b.bitboard[chess::WP] & util::black_squares;
        else white_pawns_of_bishop_color = b.bitboard[chess::WP] & util::white_squares;

        int no_of_pawns_on_bishop_color = util::count_bits(white_pawns_of_bishop_color);

        // Bad Bishop Penality
        mg_score -= no_of_pawns_on_bishop_color * eval::eval_data.bad_bishop_penalty.mg;
        eg_score -= no_of_pawns_on_bishop_color * eval::eval_data.bad_bishop_penalty.eg;

        // Mobility
        uint64_t bishop_moves_bitboard = chess::get_diagonal_slider_attacks(sq, b.occupied) & (~b.white_occupied);
        int bishop_moves = util::count_bits(bishop_moves_bitboard);
        mg_score += eval::eval_data.mobility_bonus[chess::BISHOP][bishop_moves].mg;
        eg_score += eval::eval_data.mobility_bonus[chess::BISHOP][bishop_moves].eg;

        // Space 
        uint64_t bishop_attacks_on_balck_side = chess::get_diagonal_slider_attacks(sq, b.occupied) & util::black_side_of_board;

        int no_of_square_controlled_by_white = util::count_bits(bishop_attacks_on_balck_side);

        mg_score += no_of_square_controlled_by_white * eval::eval_data.controlled_square_bonus.mg;
        eg_score += no_of_square_controlled_by_white * eval::eval_data.controlled_square_bonus.eg;
    }
    
    // 2. Black Bishops
    uint64_t black_bishops = b.bitboard[chess::BB];
    while (black_bishops) {
        chess::Square sq = util::pop_lsb(black_bishops);

        // Material
        mg_score -= eval::eval_data.material_values[chess::BISHOP].mg;
        eg_score -= eval::eval_data.material_values[chess::BISHOP].eg;

        int pst_sq = util::flip(sq);

        // PST(s)
        mg_score -= eval::eval_data.psts[chess::BISHOP][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::BISHOP][pst_sq].eg;

        uint64_t black_pawns_of_bishop_color;
        if (util::create_bitboard_from_square(sq) & util::black_squares) black_pawns_of_bishop_color = b.bitboard[chess::BP] & util::black_squares;
        else black_pawns_of_bishop_color = b.bitboard[chess::BP] & util::white_squares;

        int no_of_pawns_on_bishop_color = util::count_bits(black_pawns_of_bishop_color);

        // Bad Bishop Penality
        mg_score += no_of_pawns_on_bishop_color * eval::eval_data.bad_bishop_penalty.mg;
        eg_score += no_of_pawns_on_bishop_color * eval::eval_data.bad_bishop_penalty.eg;

        // Mobility
        uint64_t bishop_moves_bitboard = chess::get_diagonal_slider_attacks(sq, b.occupied) & (~b.black_occupied);
        int bishop_moves = util::count_bits(bishop_moves_bitboard);
        mg_score -= eval::eval_data.mobility_bonus[chess::BISHOP][bishop_moves].mg;
        eg_score -= eval::eval_data.mobility_bonus[chess::BISHOP][bishop_moves].eg;

        // Space 
        uint64_t bishop_attacks_on_white_side = chess::get_diagonal_slider_attacks(sq, b.occupied) & util::white_side_of_board;

        int no_of_square_controlled_by_black = util::count_bits(bishop_attacks_on_white_side);

        mg_score -= no_of_square_controlled_by_black * eval::eval_data.controlled_square_bonus.mg;
        eg_score -= no_of_square_controlled_by_black * eval::eval_data.controlled_square_bonus.eg;
    }

    // Bishop Pair Bonus
    int white_bishops_count = util::count_bits(b.bitboard[chess::WB]);
    int black_bishops_count = util::count_bits(b.bitboard[chess::BB]);
    
    if (white_bishops_count >= 2) {
        mg_score += eval::eval_data.bishop_pair_bonus.mg;
        eg_score += eval::eval_data.bishop_pair_bonus.eg;
    }
    if (black_bishops_count >= 2) {
        mg_score -= eval::eval_data.bishop_pair_bonus.mg;
        eg_score -= eval::eval_data.bishop_pair_bonus.eg;
    }
}