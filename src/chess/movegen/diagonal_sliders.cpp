#include "movegen.h"

void MoveGen::generate_diagonal_sliders_moves(const Board& B, std::vector<chess::Move>& moveList){
    chess::Color color = B.white_to_move ? chess::WHITE : chess::BLACK;
    uint64_t diagonal_sliders = (B.bitboard[chess::make_piece(color, chess::BISHOP)] | B.bitboard[chess::make_piece(color, chess::QUEEN)]);

    while (diagonal_sliders){
        const chess::Square from_sq = util::pop_lsb(diagonal_sliders);
        
        uint64_t attacks = get_diagonal_slider_attacks(from_sq, B.occupied);

        if (B.pin_bitboard & util::create_bitboard_from_square(from_sq)) attacks &= B.pinRays[from_sq];
        if (B.checker_bitboard)  attacks &= B.checkRay;
        uint64_t quiet_moves = attacks & (~B.occupied);
        while (quiet_moves) {
            const chess::Square to_sq = util::pop_lsb(quiet_moves);
            moveList.push_back(chess::Move(from_sq, to_sq, chess::FLAG_QUIET, chess::NO_PIECE));
        }

        uint64_t opponentPieces = color ? B.white_occupied : B.black_occupied;

        uint64_t capture_moves = attacks & opponentPieces;
        while (capture_moves) {
            const chess::Square to_sq = util::pop_lsb(capture_moves);
            moveList.push_back(chess::Move(from_sq, to_sq, chess::FLAG_CAPTURE, chess::NO_PIECE));
        }
    }
}
