#include "movegen.h"

void MoveGen::gen_king_moves(const Chess::Board &B, std::vector<MoveType> &moves, bool whiteTurn)
{
    if(whiteTurn)   gen_white_king_moves(B, moves);
    else            gen_black_king_moves(B,moves);

    gen_castling_moves(B, moves, whiteTurn);
}


void MoveGen::gen_white_king_moves(const Chess::Board &B, std::vector<MoveType> &moves) {
    uint64_t king = B.WK;
    uint64_t white = B.getWhite();
    uint64_t black = B.getBlack();

    while (king) {
        int from = pop_lsb(king);
        uint64_t mask = all_king_moves[from] & ~white;
        while (mask) {
            int to = pop_lsb(mask);
            int captured = (black & (ONE << to)) ? static_cast<int>(get_black_capturable_pieces(B, ONE << to).first) : 0;
            moves.push_back(Move::encode(from, to, static_cast<int>(Move::Piece::WK), captured));
        }
    }
}

void MoveGen::gen_black_king_moves(const Chess::Board &B, std::vector<MoveType> &moves) {
    uint64_t king = B.BK;
    uint64_t white = B.getWhite();
    uint64_t black = B.getBlack();

    while (king) {
        int from = pop_lsb(king);
        uint64_t mask = all_king_moves[from] & ~black;
        while (mask) {
            int to = pop_lsb(mask);
            int captured = (white & (ONE << to)) ? static_cast<int>(get_white_capturable_pieces(B, ONE << to).first) : 0;
            moves.push_back(Move::encode(from, to, static_cast<int>(Move::Piece::BK), captured));
        }
    }
}

void MoveGen::gen_castling_moves(const Chess::Board &B, std::vector<MoveType> &moves, bool whiteTurn) {
    const int castlingRights = B.castling_rights;
    const uint64_t occupied = B.getOccupied();

    auto try_castle = [&](uint64_t kingSquare, int fromOffset, int toOffset, int flag) {
        uint64_t mask = 0ULL;
        for (int shift = 1; shift <= abs(fromOffset); ++shift) {
            mask |= (fromOffset > 0) ? (kingSquare << shift) : (kingSquare >> shift);
        }

        bool canCastle = true;
        uint64_t checkMask = mask;
        while (checkMask) {
            int sq = pop_lsb(checkMask);
            uint64_t sqMask = (ONE << sq);
            if ((occupied & sqMask) || B.is_attacked(sqMask)) {
                canCastle = false;
                break;
            }
        }

        if (canCastle) {
            int fromIndex = lsb_index(kingSquare);
            moves.push_back(Move::encode(fromIndex, fromIndex + toOffset,
                (kingSquare == B.WK) ? static_cast<int>(Move::Piece::WK) : static_cast<int>(Move::Piece::BK),
                0, 0, flag));
        }
    };

    if(whiteTurn){
        if ((castlingRights >> 3) & 1) try_castle(B.WK, 2, +2, static_cast<int>(Move::Flags::KING_CASTLE));
        if ((castlingRights >> 2) & 1) try_castle(B.WK, 3, -3, static_cast<int>(Move::Flags::QUEEN_CASTLE));
    }
    else{
        if ((castlingRights >> 1) & 1) try_castle(B.BK, 2, +2, static_cast<int>(Move::Flags::KING_CASTLE));
        if (castlingRights & 1)       try_castle(B.BK, 3, -3, static_cast<int>(Move::Flags::QUEEN_CASTLE));
    }
}
