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
            safety_score.mg += eval::eval_data.open_file_penalty.mg;
            safety_score.eg += eval::eval_data.open_file_penalty.eg;
        } 
        else {
            chess::Square pawn_sq = (color == chess::WHITE) ? util::lsb(friendly_pawns) : util::msb(friendly_pawns);

            int pawn_rank = util::get_rank(pawn_sq);
            int ideal_rank = (color == chess::WHITE) ? util::Rank2 : util::Rank7;
            int rank_dist = std::abs(pawn_rank - ideal_rank);

            if (rank_dist > 0 && rank_dist < eval::eval_data.pawn_shield_penalty.size()) {
                safety_score.mg += eval::eval_data.pawn_shield_penalty[rank_dist].mg;
                safety_score.eg += eval::eval_data.pawn_shield_penalty[rank_dist].eg;
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
void pawn_evaluation(const Board& b, int& mg_score, int& eg_score) {
    
    // 1. White Pawns
    uint64_t white_pawns = b.bitboard[chess::WP];
    while (white_pawns) {
        chess::Square sq = util::pop_lsb(white_pawns);
        mg_score += eval::eval_data.material_values[chess::PAWN].mg;
        eg_score += eval::eval_data.material_values[chess::PAWN].eg;
        
        mg_score += eval::eval_data.psts[chess::PAWN][sq].mg;
        eg_score += eval::eval_data.psts[chess::PAWN][sq].eg;
        
        // Passed Pawn Bonus
        uint64_t passing_mask = eval::eval_data.passed_pawn_masks_white[sq];
        uint64_t enemy_pawns = b.bitboard[chess::BP] & passing_mask;
        if ( !enemy_pawns ) {
            mg_score += eval::eval_data.passed_pawn_bonus[util::get_rank(sq)].mg;
            eg_score += eval::eval_data.passed_pawn_bonus[util::get_rank(sq)].eg;
        }
        
        // Connected Pawn Bonus
        uint64_t connecting_square = chess::PawnAttacks[chess::BLACK][sq];
        if (connecting_square & b.bitboard[chess::WP]) {
            mg_score += eval::eval_data.connected_pawn_bonus.mg;
            eg_score += eval::eval_data.connected_pawn_bonus.eg;
        }
        
        // Isolated Pawn Penalty
        int file = util::get_file(sq);
        uint64_t adjacent_files = eval::eval_data.adjacent_files_masks[file];
        if (!(b.bitboard[chess::WP] & adjacent_files)) {
            mg_score += eval::eval_data.isolated_pawn_penalty.mg;
            eg_score += eval::eval_data.isolated_pawn_penalty.eg;
        }
    }
    
    // 2. Black Pawns
    uint64_t black_pawns = b.bitboard[chess::BP];
    while (black_pawns) {
        chess::Square sq = util::pop_lsb(black_pawns);
        mg_score -= eval::eval_data.material_values[chess::PAWN].mg;
        eg_score -= eval::eval_data.material_values[chess::PAWN].eg;
        
        chess::Square pst_sq = util::flip(sq);
        mg_score -= eval::eval_data.psts[chess::PAWN][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::PAWN][pst_sq].eg;
        
        // Passed Pawn Bonus
        uint64_t passing_mask = eval::eval_data.passed_pawn_masks_black[sq];
        uint64_t enemy_pawns = b.bitboard[chess::WP] & passing_mask;
        if ( !enemy_pawns ) {
            mg_score -= eval::eval_data.passed_pawn_bonus[util::get_rank(pst_sq)].mg;
            eg_score -= eval::eval_data.passed_pawn_bonus[util::get_rank(pst_sq)].eg;
        }
        
        // Connected Pawn Bonus
        uint64_t connecting_square = chess::PawnAttacks[chess::WHITE][sq];
        if ( connecting_square & b.bitboard[chess::BP] ) {
            mg_score -= eval::eval_data.connected_pawn_bonus.mg;
            eg_score -= eval::eval_data.connected_pawn_bonus.eg;
        }
        
        // Isolated Pawn Penalty
        int file = util::get_file(sq);
        uint64_t adjacent_files = eval::eval_data.adjacent_files_masks[file];
        if (!(b.bitboard[chess::BP] & adjacent_files)) {
            mg_score -= eval::eval_data.isolated_pawn_penalty.mg;
            eg_score -= eval::eval_data.isolated_pawn_penalty.eg;
        }
    }    
    
    // Doubled Pawns Penalty
    for (auto file : chess::files) {
        uint64_t pawns_on_file_white = b.bitboard[chess::WP] & file;
        if (util::count_bits(pawns_on_file_white) > 1) {
            mg_score += eval::eval_data.doubled_pawn_penalty.mg * util::count_bits(pawns_on_file_white);
            eg_score += eval::eval_data.doubled_pawn_penalty.eg * util::count_bits(pawns_on_file_white);
        }
        uint64_t pawns_on_file_black = b.bitboard[chess::BP] & file;
        if (util::count_bits(pawns_on_file_black) > 1) {
            mg_score -= eval::eval_data.doubled_pawn_penalty.mg * util::count_bits(pawns_on_file_black);
            eg_score -= eval::eval_data.doubled_pawn_penalty.eg * util::count_bits(pawns_on_file_black);
        }
    }
}

void knight_evaluation(const Board& b, int& mg_score, int& eg_score, int& game_phase) {
    // 1. White Knights
    uint64_t white_knights = b.bitboard[chess::WN];
    while (white_knights) {
        chess::Square sq = util::pop_lsb(white_knights);
        mg_score += eval::eval_data.material_values[chess::KNIGHT].mg;
        eg_score += eval::eval_data.material_values[chess::KNIGHT].eg;
        
        mg_score += eval::eval_data.psts[chess::KNIGHT][sq].mg;
        eg_score += eval::eval_data.psts[chess::KNIGHT][sq].eg;

        game_phase += eval::eval_data.phase_values[chess::KNIGHT]; 
    }
    
    // 2. Black Knights
    uint64_t black_knights = b.bitboard[chess::BN];
    while (black_knights) {
        chess::Square sq = util::pop_lsb(black_knights);
        mg_score -= eval::eval_data.material_values[chess::KNIGHT].mg;
        eg_score -= eval::eval_data.material_values[chess::KNIGHT].eg;

        int pst_sq = util::flip(sq);
        mg_score -= eval::eval_data.psts[chess::KNIGHT][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::KNIGHT][pst_sq].eg;

        game_phase += eval::eval_data.phase_values[chess::KNIGHT]; 
    }
}

void bishop_evaluation(const Board& b, int& mg_score, int& eg_score, int& game_phase) {
    // 1. White Bishops
    uint64_t white_bishops = b.bitboard[chess::WB];
    while (white_bishops) {
        chess::Square sq = util::pop_lsb(white_bishops);
        mg_score += eval::eval_data.material_values[chess::BISHOP].mg;
        eg_score += eval::eval_data.material_values[chess::BISHOP].eg;
        
        mg_score += eval::eval_data.psts[chess::BISHOP][sq].mg;
        eg_score += eval::eval_data.psts[chess::BISHOP][sq].eg;

        game_phase += eval::eval_data.phase_values[chess::BISHOP]; 
    }
    
    // 2. Black Bishops
    uint64_t black_bishops = b.bitboard[chess::BB];
    while (black_bishops) {
        chess::Square sq = util::pop_lsb(black_bishops);
        mg_score -= eval::eval_data.material_values[chess::BISHOP].mg;
        eg_score -= eval::eval_data.material_values[chess::BISHOP].eg;

        int pst_sq = util::flip(sq);
        mg_score -= eval::eval_data.psts[chess::BISHOP][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::BISHOP][pst_sq].eg;

        game_phase += eval::eval_data.phase_values[chess::BISHOP]; 
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

void rook_evaluation(const Board& b, int& mg_score, int& eg_score, int& game_phase) {
    // 1. White Rooks
    uint64_t white_rooks = b.bitboard[chess::WR];
    while (white_rooks) {
        chess::Square sq = util::pop_lsb(white_rooks);
        mg_score += eval::eval_data.material_values[chess::ROOK].mg;
        eg_score += eval::eval_data.material_values[chess::ROOK].eg;
        
        mg_score += eval::eval_data.psts[chess::ROOK][sq].mg;
        eg_score += eval::eval_data.psts[chess::ROOK][sq].eg;

        game_phase += eval::eval_data.phase_values[chess::ROOK]; 
    }
    
    // 2. Black Rooks
    uint64_t black_rooks = b.bitboard[chess::BR];
    while (black_rooks) {
        chess::Square sq = util::pop_lsb(black_rooks);
        mg_score -= eval::eval_data.material_values[chess::ROOK].mg;
        eg_score -= eval::eval_data.material_values[chess::ROOK].eg;

        int pst_sq = util::flip(sq);
        mg_score -= eval::eval_data.psts[chess::ROOK][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::ROOK][pst_sq].eg;

        game_phase += eval::eval_data.phase_values[chess::ROOK]; 
    }
}

void queen_evaluation(const Board& b, int& mg_score, int& eg_score, int& game_phase) {
    // 1. White Queens
    uint64_t white_queens = b.bitboard[chess::WQ];
    while (white_queens) {
        chess::Square sq = util::pop_lsb(white_queens);
        mg_score += eval::eval_data.material_values[chess::QUEEN].mg;
        eg_score += eval::eval_data.material_values[chess::QUEEN].eg;
        
        mg_score += eval::eval_data.psts[chess::QUEEN][sq].mg;
        eg_score += eval::eval_data.psts[chess::QUEEN][sq].eg;

        game_phase += eval::eval_data.phase_values[chess::QUEEN];
    }
    
    // 2. Black Queens
    uint64_t black_queens = b.bitboard[chess::BQ];
    while (black_queens) {
        chess::Square sq = util::pop_lsb(black_queens);
        mg_score -= eval::eval_data.material_values[chess::QUEEN].mg;
        eg_score -= eval::eval_data.material_values[chess::QUEEN].eg;

        int pst_sq = util::flip(sq);
        mg_score -= eval::eval_data.psts[chess::QUEEN][pst_sq].mg;
        eg_score -= eval::eval_data.psts[chess::QUEEN][pst_sq].eg;

        game_phase += eval::eval_data.phase_values[chess::QUEEN];
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
}

int Search::evaluate(const Board& b) {
    int mg_score = 0;
    int eg_score = 0;
    int game_phase = 0;

    pawn_evaluation(b, mg_score, eg_score);
    // std::cout << "Pawn Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    knight_evaluation(b, mg_score, eg_score, game_phase);
    // std::cout << "Knight Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    bishop_evaluation(b, mg_score, eg_score, game_phase);
    // std::cout << "Bishop Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    rook_evaluation(b, mg_score, eg_score, game_phase);
    // std::cout << "Rook Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    queen_evaluation(b, mg_score, eg_score, game_phase);
    // std::cout << "Queen Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    king_evaluation(b, mg_score, eg_score);
    // std::cout << "King Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;

    // Clamp game phase to valid range
    game_phase = std::max(0, std::min(game_phase, eval::TOTAL_PHASE));

    int final_score = (mg_score * game_phase + eg_score * (eval::TOTAL_PHASE - game_phase)) / eval::TOTAL_PHASE;

    // return final_score;
    return b.white_to_move ? final_score : -final_score;

}