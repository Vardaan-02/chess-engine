#include "chess/movegen.h"

void MoveGen::generate_orthogonal_sliders_moves(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly){
    chess::Color color = B.white_to_move ? chess::WHITE : chess::BLACK;
    uint64_t orthogonal_sliders = (B.bitboard[chess::make_piece(color, chess::ROOK)] | B.bitboard[chess::make_piece(color, chess::QUEEN)]);
    while (orthogonal_sliders){
        const chess::Square from_sq = util::pop_lsb(orthogonal_sliders);
        
        uint64_t attacks = get_orthogonal_slider_attacks(from_sq, B.occupied) & (color ? (~B.black_occupied) : (~B.white_occupied));
        while (attacks) {
            const chess::Square to_sq = util::pop_lsb(attacks);
            chess::MoveFlag flag = (util::create_bitboard_from_square(to_sq) & (color ? B.white_occupied : B.black_occupied)) ? chess::FLAG_CAPTURE : chess::FLAG_QUIET;
            
            if(capturesOnly && (flag == chess::FLAG_QUIET)) continue;

            moveList.push_back(chess::Move(from_sq, to_sq, flag, chess::NO_PIECE));
        }
    }
}
