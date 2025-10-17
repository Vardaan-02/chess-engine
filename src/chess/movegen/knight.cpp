#include "chess/movegen.h"

void MoveGen::generate_knight_moves(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly){
    const chess::Color color = B.white_to_move ? chess::WHITE : chess::BLACK;
    uint64_t knightBitboard = B.bitboard[chess::make_piece(color, chess::KNIGHT)];
    while (knightBitboard){
        const chess::Square currKnightSquare = util::pop_lsb(knightBitboard);
        uint64_t attacks = chess::KnightAttacks[currKnightSquare] & ~(color ? B.black_occupied : B.white_occupied);

        while (attacks){
            const chess::Square destinationKnightSquare = util::pop_lsb(attacks);
            const chess::MoveFlag flag = (util::create_bitboard_from_square(destinationKnightSquare) & (color ? B.white_occupied : B.black_occupied)) ? chess::FLAG_CAPTURE : chess::FLAG_QUIET; 

            if(capturesOnly && flag == chess::FLAG_QUIET) continue;

            chess::Move m(currKnightSquare, destinationKnightSquare, flag, chess::NO_PIECE);
            moveList.push_back(m);
        }
    }
}