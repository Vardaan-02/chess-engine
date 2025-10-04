#include "movegen.h"

void MoveGen::gen_white_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves) {
    uint64_t knights = B.WN;
    uint64_t white = B.getWhite();
    uint64_t black = B.getBlack();

    while (knights) {
        int from = pop_lsb(knights);
        uint64_t mask = all_knight_moves[from] & ~white;
        while (mask) {
            int to = pop_lsb(mask);
            int captured = (black & (ONE << to)) ? static_cast<int>(get_black_capturable_pieces(B, ONE << to).first) : 0;
            moves.push_back(Move::encode(from, to, static_cast<int>(Move::Piece::WN), captured));
        }
    }
}

void MoveGen::gen_black_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves) {
    uint64_t knights = B.BN;
    uint64_t white = B.getWhite();
    uint64_t black = B.getBlack();

    while (knights) {
        int from = pop_lsb(knights);
        uint64_t mask = all_knight_moves[from] & ~black;
        while (mask) {
            int to = pop_lsb(mask);
            int captured = (white & (ONE << to)) ? static_cast<int>(get_white_capturable_pieces(B, ONE << to).first) : 0;
            moves.push_back(Move::encode(from, to, static_cast<int>(Move::Piece::BN), captured));
        }
    }
}

void MoveGen::gen_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves, bool whiteTurn) {
    if(whiteTurn) gen_white_knight_moves(B, moves);
    else gen_black_knight_moves(B, moves);
}
