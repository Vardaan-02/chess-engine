#include "chess/movegen.h"

void MoveGen::generate_diagonal_sliders_moves(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly){
    chess::Color color = B.white_to_move ? chess::WHITE : chess::BLACK;
    uint64_t diagonal_sliders = (B.bitboard[chess::make_piece(color, chess::BISHOP)] | B.bitboard[chess::make_piece(color, chess::QUEEN)]);
    while (diagonal_sliders){
        const chess::Square from_sq = util::pop_lsb(diagonal_sliders);
        
        uint64_t attacks = get_diagonal_slider_attacks(from_sq, B.occupied) & (color ? ~B.black_occupied : ~B.white_occupied);

        while (attacks) {
            const chess::Square to_sq = util::pop_lsb(attacks);

            chess::MoveFlag flag = (util::create_bitboard_from_square(to_sq) & (color ? B.white_occupied : B.black_occupied)) ? chess::FLAG_CAPTURE : chess::FLAG_QUIET;

            if(capturesOnly && (flag == chess::FLAG_QUIET)) continue;
            
            moveList.push_back(chess::Move(from_sq, to_sq, flag, chess::NO_PIECE));
        }
    }
}
