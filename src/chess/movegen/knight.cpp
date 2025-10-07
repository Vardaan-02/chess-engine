#include "movegen.h"

void MoveGen::generate_knight_moves(const Board& B, std::vector<chess::Move>& moveList){
    const chess::Color color = B.white_to_move ? chess::WHITE : chess::BLACK;
    uint64_t knightBitboard = B.bitboard[chess::make_piece(color, chess::KNIGHT)];
    while (knightBitboard){
        const chess::Square currKnightSquare = util::pop_lsb(knightBitboard);
        uint64_t attacks = chess::KnightAttacks[currKnightSquare];
        uint64_t quietMoves = attacks & (~B.occupied);


        while (quietMoves){
            const chess::Square destinationKnightSquare = util::pop_lsb(quietMoves);
            chess::Move m(currKnightSquare, destinationKnightSquare, chess::FLAG_QUIET, chess::NO_PIECE);
            moveList.push_back(m);
        }

        uint64_t captures = (attacks & (color ? B.white_occupied : B.black_occupied));

        while (captures){
            const chess::Square destinationKnightSquare = util::pop_lsb(captures);
            chess::Move m(currKnightSquare, destinationKnightSquare, chess::FLAG_CAPTURE, chess::NO_PIECE);
            moveList.push_back(m);
        }
    }
}