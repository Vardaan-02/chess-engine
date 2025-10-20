#include "engine/evaluate.h"
#include "engine/search.h"

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

void Search::king_evaluation(const Board& b, int& mg_score, int& eg_score) {
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
