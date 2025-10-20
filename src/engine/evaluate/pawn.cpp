#include "engine/search.h"
#include "engine/evaluate.h"

void Search::pawn_evaluation(const Board& b, int& mg_score, int& eg_score) {
    
    // 1. White Pawns
    uint64_t white_pawns = b.bitboard[chess::WP];

    uint64_t white_pawns_east_attacks = util::shift_board(white_pawns, chess::NORTH_EAST) & util::black_side_of_board;
    uint64_t white_pawns_west_attacks = util::shift_board(white_pawns, chess::NORTH_WEST) & util::black_side_of_board;
    
    uint64_t white_pawns_attacks = white_pawns_east_attacks | white_pawns_west_attacks;

    // Pawn Chain
    uint64_t pawn_chain_white = white_pawns_attacks & white_pawns;
    int no_of_pawns_in_pawn_chain_white = util::count_bits(pawn_chain_white);

    mg_score += eval::eval_data.pawn_chain_bonus[no_of_pawns_in_pawn_chain_white].mg;
    eg_score += eval::eval_data.pawn_chain_bonus[no_of_pawns_in_pawn_chain_white].eg;
    
    // Space
    int no_of_square_controlled_by_white = util::count_bits(white_pawns_attacks);

    mg_score += no_of_square_controlled_by_white * eval::eval_data.controlled_square_bonus.mg;
    eg_score += no_of_square_controlled_by_white * eval::eval_data.controlled_square_bonus.eg;
    
    while (white_pawns) {
        chess::Square sq = util::pop_lsb(white_pawns);

        // Material
        mg_score += eval::eval_data.material_values[chess::PAWN].mg;
        eg_score += eval::eval_data.material_values[chess::PAWN].eg;
        
        // PST(s)
        mg_score += eval::eval_data.psts[chess::PAWN][sq].mg;
        eg_score += eval::eval_data.psts[chess::PAWN][sq].eg;
        
        // Passed Pawn Bonus
        uint64_t passing_mask = chess::passed_pawn_masks_white[sq];
        uint64_t enemy_pawns = b.bitboard[chess::BP] & passing_mask;
        if ( !enemy_pawns ) {
            mg_score += eval::eval_data.passed_pawn_bonus[util::get_rank(sq)].mg;
            eg_score += eval::eval_data.passed_pawn_bonus[util::get_rank(sq)].eg;
        }

        // Backward Pawn Penalty
        uint64_t backward_mask = chess::passed_pawn_masks_black[sq];
        uint64_t friendly_pawns = b.bitboard[chess::WP] & backward_mask;
        if ( !friendly_pawns ){
            uint64_t next_square_bitboard = util::shift_board(util::create_bitboard_from_square(sq), chess::NORTH);
            int next_square = util::lsb(next_square_bitboard);
            uint64_t next_square_attacking_mask = chess::PawnAttacks[chess::WHITE][next_square];

            if ( next_square_attacking_mask & b.bitboard[chess::BP] ){
                mg_score -= eval::eval_data.backward_pawn_penalty.mg;
                eg_score -= eval::eval_data.backward_pawn_penalty.eg;
            }
        }
        
        // Isolated Pawn Penalty
        int file = util::get_file(sq);
        uint64_t adjacent_files = eval::eval_data.adjacent_files_masks[file];
        if (!(b.bitboard[chess::WP] & adjacent_files)) {
            mg_score -= eval::eval_data.isolated_pawn_penalty.mg;
            eg_score -= eval::eval_data.isolated_pawn_penalty.eg;
        }
    }
    
    // 2. Black Pawns
    uint64_t black_pawns = b.bitboard[chess::BP];

    uint64_t black_pawns_east_attacks = util::shift_board(black_pawns, chess::SOUTH_EAST) & util::white_side_of_board;
    uint64_t black_pawns_west_attacks = util::shift_board(black_pawns, chess::SOUTH_WEST) & util::white_side_of_board;
    
    uint64_t black_pawns_attacks = black_pawns_east_attacks | black_pawns_west_attacks;
    
    // Pawn Chain
    uint64_t pawn_chain_black = black_pawns_attacks & black_pawns;
    int no_of_pawns_in_pawn_chain_black = util::count_bits(pawn_chain_black);
    
    mg_score -= eval::eval_data.pawn_chain_bonus[no_of_pawns_in_pawn_chain_black].mg;
    eg_score -= eval::eval_data.pawn_chain_bonus[no_of_pawns_in_pawn_chain_black].eg;
    
    // Space
    int no_of_square_controlled_by_black = util::count_bits(black_pawns_attacks);
    
    mg_score -= no_of_square_controlled_by_black * eval::eval_data.controlled_square_bonus.mg;
    eg_score -= no_of_square_controlled_by_black * eval::eval_data.controlled_square_bonus.eg;

    while (black_pawns) {
        chess::Square sq = util::pop_lsb(black_pawns);

        // Material
        mg_score -= eval::eval_data.material_values[chess::PAWN].mg;
        eg_score -= eval::eval_data.material_values[chess::PAWN].eg;
        
        chess::Square pst_sq = util::flip(sq);

        // PST(s)
        mg_score -= eval::eval_data.psts[chess::PAWN][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::PAWN][pst_sq].eg;
        
        // Passed Pawn Bonus
        uint64_t passing_mask = chess::passed_pawn_masks_black[sq];
        uint64_t enemy_pawns = b.bitboard[chess::WP] & passing_mask;
        if ( !enemy_pawns ) {
            mg_score -= eval::eval_data.passed_pawn_bonus[util::get_rank(pst_sq)].mg;
            eg_score -= eval::eval_data.passed_pawn_bonus[util::get_rank(pst_sq)].eg;
        }

        // Backward Pawn Penalty
        uint64_t backward_mask = chess::passed_pawn_masks_black[sq];
        uint64_t friendly_pawns = b.bitboard[chess::BP] & backward_mask;
        if ( !friendly_pawns ){
            uint64_t next_square_bitboard = util::shift_board(util::create_bitboard_from_square(sq), chess::SOUTH);
            int next_square = util::lsb(next_square_bitboard);
            uint64_t next_square_attacking_mask = chess::PawnAttacks[chess::BLACK][next_square];

            if ( next_square_attacking_mask & b.bitboard[chess::WP] ){
                mg_score += eval::eval_data.backward_pawn_penalty.mg;
                eg_score += eval::eval_data.backward_pawn_penalty.eg;
            }
        }
        
        // Isolated Pawn Penalty
        int file = util::get_file(sq);
        uint64_t adjacent_files = eval::eval_data.adjacent_files_masks[file];
        if (!(b.bitboard[chess::BP] & adjacent_files)) {
            mg_score += eval::eval_data.isolated_pawn_penalty.mg;
            eg_score += eval::eval_data.isolated_pawn_penalty.eg;
        }
    }    
    
    // Doubled Pawns Penalty
    for (auto file : chess::files) {
        uint64_t pawns_on_file_white = b.bitboard[chess::WP] & file;
        int no_of_doubled_pawns_white = util::count_bits(pawns_on_file_white);
        if (no_of_doubled_pawns_white > 1) {
            mg_score -= eval::eval_data.doubled_pawn_penalty.mg * (no_of_doubled_pawns_white - 1);
            eg_score -= eval::eval_data.doubled_pawn_penalty.eg * (no_of_doubled_pawns_white - 1);
        }
        uint64_t pawns_on_file_black = b.bitboard[chess::BP] & file;
        int no_of_doubled_pawns_black = util::count_bits(pawns_on_file_black);
        if (no_of_doubled_pawns_black > 1) {
            mg_score += eval::eval_data.doubled_pawn_penalty.mg * (no_of_doubled_pawns_black - 1);
            eg_score += eval::eval_data.doubled_pawn_penalty.eg * (no_of_doubled_pawns_black - 1);
        }
    }
}