#include "engine/search.h"
#include "engine/evaluate.h"

eval::TaperedScore king_safety_score(const Board& b, chess::Color color) {
    eval::TaperedScore safety_score = {0, 0};
    chess::Square king_square = (color == chess::WHITE) ? b.white_king_sq : b.black_king_sq;
    int king_file = util::get_file(king_square);
    int king_rank = util::get_rank(king_square);

    // --- Part 1: Pawn Shield Evaluation ---
    for (int file = king_file - 1; file <= king_file + 1; file++) {
        if (file < 0 || file > 7) continue; // Out of board
        
        chess::Square pawn_sq = chess::SQUARE_NONE;

        int step = (color == chess::WHITE) ? 1 : -1;
        
        for (int rank = king_rank + step; rank >= util::Rank1 && rank <= util::Rank8; rank += step) {
            chess::Square sq = (chess::Square)(rank * 8 + file);
            chess::Piece p = b.piece_on_sq(sq);

            if (p != chess::NO_PIECE && chess::type_of(p) == chess::PAWN && chess::color_of(p) == color) {
                pawn_sq = sq;
                break;
            }
        }

        if (pawn_sq == chess::SQUARE_NONE) {
            // No pawn on this file, apply open file penalty.
            safety_score.mg += eval::eval_data.open_file_penalty.mg;
            safety_score.eg += eval::eval_data.open_file_penalty.eg;
        } 
        else {
            // Pendalty based on how far the pawn is from the ideal shield rank.
            int pawn_rank = util::get_rank(pawn_sq);
            int ideal_rank = (color == chess::WHITE) ? util::Rank2 : util::Rank7;
            int rank_dist = std::abs(pawn_rank - ideal_rank);

            if (rank_dist == 1) { 
                safety_score.mg += eval::eval_data.pawn_shield_penalty[1].mg;
                safety_score.eg += eval::eval_data.pawn_shield_penalty[1].eg;
            } 
            else if (rank_dist >= 2) { 
                safety_score.mg += eval::eval_data.pawn_shield_penalty[2].mg;
                safety_score.eg += eval::eval_data.pawn_shield_penalty[2].eg;
            }
        }
    }

    // --- Part 2: Attacks on the King Zone ---
    int attack_score = 0;
    chess::Color enemy_color = (color == chess::WHITE) ? chess::BLACK : chess::WHITE;

    // Define the "king zone" as a 3x3 square area around the king.
    for (int df = -1; df <= 1; ++df) {
        for (int dr = -1; dr <= 1; ++dr) {
            if (df == 0 && dr == 0) continue;

            int target_file = king_file + df;
            int target_rank = util::get_rank(king_square) + dr;

            if (target_file >= util::FileA && target_file <= util::FileH && target_rank >= util::Rank1 && target_rank <= util::Rank8)
            {
                chess::Square target_sq = chess::Square(target_file + target_rank * 8);
                
                uint64_t attackers = b.attackers_to(target_sq, enemy_color);

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

int get_score_from_white_perspective(const Board& b){
    int mg_score = 0;
    int eg_score = 0;
    int game_phase = 0;

    // std::cout << "HELLO start \n";

    for (chess::Square sq = chess::A1; sq <= chess::H8; ++sq) {
        chess::Piece p = b.piece_on_sq(sq);
        if (p == chess::NO_PIECE) {
            continue;
        }

        chess::PieceType type = chess::type_of(p);
        chess::Color color = chess::color_of(p);

        int sign = (color == chess::WHITE) ? 1 : -1;

        // Material Score
        mg_score += sign * eval::eval_data.material_values[type].mg;
        eg_score += sign * eval::eval_data.material_values[type].eg;

        // Piece-Square Table (PST) Score
        int pst_sq = (color == chess::WHITE) ? sq : util::flip(sq);
        mg_score += sign * eval::eval_data.psts[type][pst_sq].mg;
        eg_score += sign * eval::eval_data.psts[type][pst_sq].eg;

        // Note: Doubled, isolated, and passed pawns
        
        // Game Phase Calculation
        if (type != chess::PAWN && type != chess::KING) {
            game_phase += eval::eval_data.phase_values[type];
        }
    }
    // King Safty
    eval::TaperedScore white_king_safety = king_safety_score(b, chess::Color::WHITE);
    eval::TaperedScore black_king_safety = king_safety_score(b, chess::Color::BLACK);
    mg_score += white_king_safety.mg - black_king_safety.mg;
    eg_score += white_king_safety.eg - black_king_safety.eg;

    // Pawn Structure, etc. can be added here

    // std::cout << "HELLO mid 2 \n";
    
    // Bishop Pair Bonus
    int white_bishops = util::count_bits(b.bitboard[chess::WB]);
    int black_bishops = util::count_bits(b.bitboard[chess::BB]);
    
    if (white_bishops >= 2) {
        mg_score += eval::eval_data.bishop_pair_bonus.mg;
        eg_score += eval::eval_data.bishop_pair_bonus.eg;
    }
    if (black_bishops >= 2) {
        mg_score -= eval::eval_data.bishop_pair_bonus.mg;
        eg_score -= eval::eval_data.bishop_pair_bonus.eg;
    }

    // Clamp game phase to valid range
    game_phase = std::max(0, std::min(game_phase, eval::TOTAL_PHASE));

    int final_score = (mg_score * game_phase + eg_score * (eval::TOTAL_PHASE - game_phase)) / eval::TOTAL_PHASE;

    // std::cout << "HELLO END \n";
    return (b.white_to_move) ? final_score : -final_score;
}

int Search::evaluate(const Board& b) {
    int total_score = get_score_from_white_perspective(b);

    return total_score;
}