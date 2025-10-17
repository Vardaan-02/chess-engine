#include "chess/board.h" // Assuming this contains your class and necessary definitions

/**
 * @brief A highly optimized function to calculate all pins, checks, and check masks.
 * * This function consolidates the logic for orthogonal, diagonal, and knight/pawn checks
 * to minimize overhead and redundant calculations.
 */
void Board::compute_pins_and_checks() {
    pinned = 0ULL;
    checks = 0ULL;
    check_mask = 0ULL;
    double_check = false;

    const chess::Color color = white_to_move ? chess::WHITE : chess::BLACK;
    const chess::Color oppColor = white_to_move ? chess::BLACK : chess::WHITE;
    const chess::Square king_sq = white_to_move ? white_king_sq : black_king_sq;
    const uint64_t friendly_bitboard = white_to_move ? white_occupied : black_occupied;

    const uint64_t opp_rooks_queens = bitboard[chess::make_piece(oppColor, chess::ROOK)] | 
                                    bitboard[chess::make_piece(oppColor, chess::QUEEN)];
    const uint64_t opp_bishops_queens = bitboard[chess::make_piece(oppColor, chess::BISHOP)] | 
                                      bitboard[chess::make_piece(oppColor, chess::QUEEN)];

    uint64_t ortho_attackers = opp_rooks_queens & chess::get_orthogonal_slider_attacks(king_sq, 0);
    while (ortho_attackers) {
        const chess::Square attacker_sq = util::pop_lsb(ortho_attackers);
        const uint64_t line_between = chess::Between[king_sq][attacker_sq];
        const uint64_t pieces_on_line = line_between & occupied;

        const int blocker_count = util::count_bits(pieces_on_line);

        if (blocker_count == 0) { 
            checks |= (1ULL << attacker_sq);
            check_mask |= chess::Rays[king_sq][attacker_sq];
        } else if (blocker_count == 1) { 
            if (pieces_on_line & friendly_bitboard) { 
                pinned |= pieces_on_line;
            }
        }
    }

    uint64_t diag_attackers = opp_bishops_queens & chess::get_diagonal_slider_attacks(king_sq, 0);
    while (diag_attackers) {
        const chess::Square attacker_sq = util::pop_lsb(diag_attackers);
        const uint64_t line_between = chess::Between[king_sq][attacker_sq];
        const uint64_t pieces_on_line = line_between & occupied;

        const int blocker_count = util::count_bits(pieces_on_line);

        if (blocker_count == 0) { 
            checks |= (1ULL << attacker_sq);
            check_mask |= chess::Rays[king_sq][attacker_sq];
        } else if (blocker_count == 1) { 
            if (pieces_on_line & friendly_bitboard) {
                pinned |= pieces_on_line;
            }
        }
    }

    checks |= chess::KnightAttacks[king_sq] & bitboard[chess::make_piece(oppColor, chess::KNIGHT)];
    checks |= chess::PawnAttacks[color][king_sq] & bitboard[chess::make_piece(oppColor, chess::PAWN)];

    check_mask |= checks;

    if (util::count_bits(checks) > 1) {
        double_check = true;
    }
}