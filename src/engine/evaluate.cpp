#include <array>
#include "engine/search.h"
#include "engine/evaluate.h"

eval::TaperedScore king_safety_score(const Board& b, chess::Color color) {
    eval::TaperedScore safety_score = {0, 0};
    chess::Square king_square = (color == chess::WHITE) ? b.white_king_sq : b.black_king_sq;
    int king_file = util::get_file(king_square);

    // --- Part 1: Pawn Shield Evaluation (Corrected) ---
    for (int file_idx = king_file - 1; file_idx <= king_file + 1; ++file_idx) {
        if (file_idx < 0 || file_idx > 7) {
            continue; 
        }

        uint64_t file_mask = chess::files[file_idx];
        uint64_t friendly_pawns = file_mask & (color == chess::WHITE ? b.bitboard[chess::WP] : b.bitboard[chess::BP]);

        if (friendly_pawns == 0) {
            safety_score.mg -= eval::eval_data.open_file_penalty.mg;
            safety_score.eg -= eval::eval_data.open_file_penalty.eg;
        } 
        else {
            chess::Square pawn_sq = (color == chess::WHITE) ? util::lsb(friendly_pawns) : util::msb(friendly_pawns);

            int pawn_rank = util::get_rank(pawn_sq);
            int ideal_rank = (color == chess::WHITE) ? util::Rank2 : util::Rank7;
            int rank_dist = std::abs(pawn_rank - ideal_rank);

            if (rank_dist > 0 && rank_dist < eval::eval_data.pawn_shield_penalty.size()) {
                safety_score.mg -= eval::eval_data.pawn_shield_penalty[rank_dist].mg;
                safety_score.eg -= eval::eval_data.pawn_shield_penalty[rank_dist].eg;
            }
        }
    }

    // --- Part 2: Attacks on the King Zone (This part was already correct) ---
    int attack_score = 0;
    chess::Color enemy_color = (color == chess::WHITE) ? chess::BLACK : chess::WHITE;
    bool by_white = (enemy_color == chess::WHITE);
    int king_rank = util::get_rank(king_square); // king_file is already defined above

    for (int df = -1; df <= 1; ++df) {
        for (int dr = -1; dr <= 1; ++dr) {
            if (df == 0 && dr == 0) continue;

            int target_file = king_file + df;
            int target_rank = king_rank + dr;

            if (target_file >= 0 && target_file <= 7 && target_rank >= 0 && target_rank <= 7) {
                chess::Square target_sq = chess::Square(target_rank * 8 + target_file);
                uint64_t attackers = b.attackers_to(target_sq, by_white);

                while (attackers) {
                    chess::Square attacker_sq = util::pop_lsb(attackers);
                    chess::PieceType pt = chess::type_of(b.piece_on_sq(attacker_sq));
                    attack_score += eval::eval_data.king_attack_weights[pt];
                }
            }
        }
    }

    int final_attack_index = std::min(attack_score, (int)eval::eval_data.king_safety_table.size() - 1);
    safety_score.mg += eval::eval_data.king_safety_table[final_attack_index].mg;
    safety_score.eg += eval::eval_data.king_safety_table[final_attack_index].eg;

    return safety_score;
}

eval::TaperedScore king_activity_score(const Board& b, chess::Color color) {
    eval::TaperedScore activity_score = {0, 0};
    const chess::Square king_square = (color == chess::WHITE) ? b.white_king_sq : b.black_king_sq;
    const chess::Square opponent_king_square = (color == chess::WHITE) ? b.black_king_sq : b.white_king_sq;

    const int king_rank = util::get_rank(king_square);
    const int king_file = util::get_file(king_square);
    const int opponent_king_rank = util::get_rank(opponent_king_square);
    const int opponent_king_file = util::get_file(opponent_king_square);

    const int king_dist_to_center_rank = std::max(3 - king_rank, king_rank - 4);
    const int king_dist_to_center_file = std::max(3 - king_file, king_file - 4);
    const int king_dist_from_center = king_dist_to_center_file + king_dist_to_center_rank;
    
    // Distance of King from Center
    activity_score.eg -= king_dist_from_center * eval::eval_data.king_distance_from_center_penalty.eg;

    const int dist_between_kings_rank = std::abs(opponent_king_rank - king_rank);
    const int dist_between_kings_file = std::abs(opponent_king_file - king_file);
    const int distance_between_kings = dist_between_kings_rank + dist_between_kings_file;

    // Distance between Kings
    activity_score.eg -= distance_between_kings * eval::eval_data.king_distance_from_opponent_king_penalty.eg;

    return activity_score;
}

void pawn_evaluation(const Board& b, int& mg_score, int& eg_score) {
    
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

void knight_evaluation(const Board& b, int& mg_score, int& eg_score) {
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

void bishop_evaluation(const Board& b, int& mg_score, int& eg_score) {
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

        uint64_t pawns_of_required_color;
        if (util::create_bitboard_from_square(sq) & util::black_squares) pawns_of_required_color = b.bitboard[chess::WP] & util::white_squares;
        else pawns_of_required_color = b.bitboard[chess::WP] & util::black_squares;

        int no_of_pawns_on_required_color = util::count_bits(pawns_of_required_color);

        // Good Bishop Bonus
        mg_score += no_of_pawns_on_required_color * eval::eval_data.good_bishop_bonus.mg;
        eg_score += no_of_pawns_on_required_color * eval::eval_data.good_bishop_bonus.eg;

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

        uint64_t black_pawns_of_required_color;
        if (util::create_bitboard_from_square(sq) & util::black_squares) black_pawns_of_required_color = b.bitboard[chess::BP] & util::white_squares;
        else black_pawns_of_required_color = b.bitboard[chess::BP] & util::black_squares;

        int no_of_pawns_on_bishop_color = util::count_bits(black_pawns_of_required_color);

        // Good Bishop bonus
        mg_score -= no_of_pawns_on_bishop_color * eval::eval_data.good_bishop_bonus.mg;
        eg_score -= no_of_pawns_on_bishop_color * eval::eval_data.good_bishop_bonus.eg;

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

void rook_evaluation(const Board& b, int& mg_score, int& eg_score) {
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

void queen_evaluation(const Board& b, int& mg_score, int& eg_score) {
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

void king_evaluation(const Board& b, int& mg_score, int& eg_score) {
    // 1. White King
    uint64_t white_king = b.bitboard[chess::WK];
    if (white_king) {
        chess::Square sq = util::pop_lsb(white_king);

        mg_score += eval::eval_data.psts[chess::KING][sq].mg;
        eg_score += eval::eval_data.psts[chess::KING][sq].eg;
    }
    
    // 2. Black King
    uint64_t black_king = b.bitboard[chess::BK];
    if (black_king) {
        chess::Square sq = util::pop_lsb(black_king);

        int pst_sq = util::flip(sq);
        mg_score -= eval::eval_data.psts[chess::KING][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::KING][pst_sq].eg;
    }

    // King Safty
    eval::TaperedScore white_king_safety = king_safety_score(b, chess::Color::WHITE);
    eval::TaperedScore black_king_safety = king_safety_score(b, chess::Color::BLACK);
    mg_score += white_king_safety.mg - black_king_safety.mg;
    eg_score += white_king_safety.eg - black_king_safety.eg;

    // King Activity
    eval::TaperedScore white_king_activity = king_activity_score(b, chess::Color::WHITE);
    eval::TaperedScore black_king_activity = king_activity_score(b, chess::Color::BLACK);
    mg_score += white_king_activity.mg - black_king_activity.mg;
    eg_score += white_king_activity.eg - black_king_activity.eg;
}

int Search::evaluate(const Board& b) {
    int mg_score = 0;
    int eg_score = 0;

    pawn_evaluation(b, mg_score, eg_score);
    // std::cout << "Pawn Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    knight_evaluation(b, mg_score, eg_score);
    // std::cout << "Knight Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    bishop_evaluation(b, mg_score, eg_score);
    // std::cout << "Bishop Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    rook_evaluation(b, mg_score, eg_score);
    // std::cout << "Rook Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    queen_evaluation(b, mg_score, eg_score);
    // std::cout << "Queen Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    king_evaluation(b, mg_score, eg_score);
    // std::cout << "King Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;

    int final_score = (mg_score * b.game_phase + eg_score * (util::TOTAL_PHASE - b.game_phase)) / util::TOTAL_PHASE;

    // return final_score;
    return b.white_to_move ? final_score : -final_score;

}