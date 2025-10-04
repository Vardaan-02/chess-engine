#include "movegen.h"

#define ONE 1ULL

void MoveGen::gen_white_pawn_promotions(int currPawnIndex, int nextPawnIndex, std::vector<MoveType>& moves, int flags, int capturedPiece){
    std::vector<int> whitePieces = {
        static_cast<int>(Move::Piece::WQ),
        static_cast<int>(Move::Piece::WR),
        static_cast<int>(Move::Piece::WB),
        static_cast<int>(Move::Piece::WN)
    };

    for (auto piece : whitePieces){
        MoveType m = Move::encode(currPawnIndex, nextPawnIndex,
            static_cast<int>(Move::Piece::WP),
            capturedPiece,
            piece,
            flags | static_cast<int>(Move::Flags::PROMOTION));
        moves.push_back(m);
    }
}

void MoveGen::gen_white_pawn_single_push(const Chess::Board &B, std::vector<MoveType> &moves)
{
    uint64_t whitePawns = B.WP;

    while(whitePawns) {
        int currPawnIndex = pop_lsb(whitePawns);

        uint64_t occupied = B.getOccupied();
        uint64_t currPawnSquare = (ONE << currPawnIndex);

        uint64_t possibleNextPawnSquare = (currPawnSquare << 8);

        if (!(possibleNextPawnSquare & occupied)){
            if (B.getRank(currPawnIndex + 8) == 7){
                gen_white_pawn_promotions(currPawnIndex, currPawnIndex + 8, moves, 0, 0);
            } else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex + 8,
                    static_cast<int>(Move::Piece::WP), 0, 0, 0);
                moves.push_back(m);
            }
        }
    }
}

void MoveGen::gen_white_pawn_double_push(const Chess::Board &B, std::vector<MoveType> &moves){
    uint64_t whitePawns = B.WP;

    while (whitePawns){
        int currPawnIndex = pop_lsb(whitePawns);

        if (B.getRank(currPawnIndex) == 1){
            uint64_t occupied = B.getOccupied();

            uint64_t currPawnSquare = (ONE << currPawnIndex);
            uint64_t nextSquare = (currPawnSquare << 8);
            uint64_t nextPawnSquare = (nextSquare << 8);

            if (!(occupied & (nextPawnSquare | nextSquare))){
                MoveType m = Move::encode(currPawnIndex, currPawnIndex + 16,
                    static_cast<int>(Move::Piece::WP), 0, 0,
                    static_cast<int>(Move::Flags::DOUBLE_PUSH));
                moves.push_back(m);
            }
        }
    }
}

std::pair<Move::Piece, bool> MoveGen::get_black_capturable_pieces(const Chess::Board& B, uint64_t targetSquare)
{
    if (B.en_passant_index != -1 && (ONE << B.en_passant_index) & targetSquare){
        return {Move::Piece::BP, 1};
    }

    const std::pair<Move::Piece, uint64_t> pieceBitboards[] = {
        { Move::Piece::BP, B.BP },
        { Move::Piece::BQ, B.BQ },
        { Move::Piece::BR, B.BR },
        { Move::Piece::BB, B.BB },
        { Move::Piece::BN, B.BN }
    };

    for (const auto& [piece, bitboard] : pieceBitboards) {
        if (bitboard & targetSquare) {
            return {piece, 0};
        }
    }

    return {Move::Piece::EMPTY, 0};
}

void MoveGen::gen_white_pawn_captures(const Chess::Board &B, std::vector<MoveType> &moves){
    uint64_t whitePawns = B.WP;

    while (whitePawns){
        int currPawnIndex = pop_lsb(whitePawns);

        uint64_t currPawnSquare = (ONE << currPawnIndex);
        uint64_t leftPawnCaptureSquare  = (currPawnSquare << 7) & (~FILE_H);
        uint64_t rightPawnCaptureSquare = (currPawnSquare << 9) & (~FILE_A);

        uint64_t blackOccupied = B.getBlack();

        if (leftPawnCaptureSquare & blackOccupied){
            auto [capturedPiece, isEPCapture] = get_black_capturable_pieces(B, leftPawnCaptureSquare);

            if (B.getRank(currPawnIndex + 7) == 7){
                gen_white_pawn_promotions(currPawnIndex, currPawnIndex + 7, moves,
                    (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) |
                    static_cast<int>(Move::Flags::CAPTURE),
                    static_cast<int>(capturedPiece));
            } else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex + 7,
                    static_cast<int>(Move::Piece::WP),
                    static_cast<int>(capturedPiece),
                    0,
                    (isEPCapture ? static_cast<int>(Move::Flags::EP_CAPTURE) : 0) |
                    static_cast<int>(Move::Flags::CAPTURE));
                moves.push_back(m);
            }
        }

        if (rightPawnCaptureSquare & blackOccupied){
            auto [capturedPiece, isEPCapture] = get_black_capturable_pieces(B, rightPawnCaptureSquare);

            if (B.getRank(currPawnIndex + 9) == 7){
                gen_white_pawn_promotions(currPawnIndex, currPawnIndex + 9, moves,
                    (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) |
                    static_cast<int>(Move::Flags::CAPTURE),
                    static_cast<int>(capturedPiece));
            } else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex + 9,
                    static_cast<int>(Move::Piece::WP),
                    static_cast<int>(capturedPiece),
                    0,
                    (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) |
                    static_cast<int>(Move::Flags::CAPTURE));
                moves.push_back(m);
            }
        }
    }
}

void MoveGen::gen_black_pawn_promotions(int currPawnIndex,int nextPawnIndex, std::vector<MoveType>& moves, int flags, int capturedPiece){
    std::vector<int> blackPieces = {
        static_cast<int>(Move::Piece::BQ),
        static_cast<int>(Move::Piece::BR),
        static_cast<int>(Move::Piece::BB),
        static_cast<int>(Move::Piece::BN)
    };

    for (auto piece : blackPieces){
        MoveType m = Move::encode(currPawnIndex, nextPawnIndex,
            static_cast<int>(Move::Piece::BP),
            capturedPiece,
            piece,
            flags | static_cast<int>(Move::Flags::PROMOTION));
        moves.push_back(m);
    }
}

void MoveGen::gen_black_pawn_single_push(const Chess::Board &B, std::vector<MoveType> &moves)
{
    uint64_t blackPawns = B.BP;

    while(blackPawns) {
        int currPawnIndex = pop_lsb(blackPawns);

        uint64_t occupied = B.getOccupied();
        uint64_t currPawnSquare = (ONE << currPawnIndex);

        uint64_t possibleNextPawnSquare = (currPawnSquare >> 8);

        if (!(possibleNextPawnSquare & occupied)){
            if (B.getRank(currPawnIndex - 8) == 0){
                gen_black_pawn_promotions(currPawnIndex, currPawnIndex - 8, moves, 0, 0);
            } else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex - 8,
                    static_cast<int>(Move::Piece::BP), 0, 0, 0);
                moves.push_back(m);
            }
        }
    }
}

void MoveGen::gen_black_pawn_double_push(const Chess::Board &B, std::vector<MoveType> &moves){
    uint64_t blackPawns = B.BP;

    while (blackPawns){
        int currPawnIndex = pop_lsb(blackPawns);

        if (B.getRank(currPawnIndex) == 6){
            uint64_t occupied = B.getOccupied();

            uint64_t currPawnSquare = (ONE << currPawnIndex);
            uint64_t nextSquare = (currPawnSquare >> 8);
            uint64_t nextPawnSquare = (nextSquare >> 8);

            if (!(occupied & (nextPawnSquare | nextSquare))){
                MoveType m = Move::encode(currPawnIndex, currPawnIndex - 16,
                    static_cast<int>(Move::Piece::BP), 0, 0,
                    static_cast<int>(Move::Flags::DOUBLE_PUSH));
                moves.push_back(m);
            }
        }
    }
}

std::pair<Move::Piece, bool> MoveGen::get_white_capturable_pieces(const Chess::Board& B, uint64_t targetSquare)
{
    if (B.en_passant_index != -1 && ((ONE << B.en_passant_index) & targetSquare)) {
        return {Move::Piece::BP, 1};
    }

    const std::pair<Move::Piece, uint64_t> pieceBitboards[] = {
        { Move::Piece::WP, B.WP },
        { Move::Piece::WQ, B.WQ },
        { Move::Piece::WR, B.WR },
        { Move::Piece::WB, B.WB },
        { Move::Piece::WN, B.WN }
    };

    for (const auto& [piece, bitboard] : pieceBitboards) {
        if (bitboard & targetSquare) {
            return {piece, 0};
        }
    }

    return {Move::Piece::EMPTY, 0};
}

void MoveGen::gen_black_pawn_captures(const Chess::Board &B, std::vector<MoveType> &moves){
    uint64_t blackPawns = B.BP;

    while (blackPawns){
        int currPawnIndex = pop_lsb(blackPawns);

        uint64_t currPawnSquare = (ONE << currPawnIndex);
        uint64_t leftPawnCaptureSquare  = (currPawnSquare >> 9) & (~FILE_H);
        uint64_t rightPawnCaptureSquare = (currPawnSquare >> 7) & (~FILE_A);

        uint64_t whiteOccupied = B.getWhite();

        if (leftPawnCaptureSquare & whiteOccupied){
            auto [capturedPiece, isEPCapture] = get_white_capturable_pieces(B, leftPawnCaptureSquare);

            if (B.getRank(currPawnIndex - 9) == 0){
                gen_black_pawn_promotions(currPawnIndex, currPawnIndex - 9, moves,
                    (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) |
                    static_cast<int>(Move::Flags::CAPTURE),
                    static_cast<int>(capturedPiece));
            } else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex - 9,
                    static_cast<int>(Move::Piece::BP),
                    static_cast<int>(capturedPiece),
                    0,
                    (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) |
                    static_cast<int>(Move::Flags::CAPTURE));
                moves.push_back(m);
            }
        }

        if (rightPawnCaptureSquare & whiteOccupied){
            auto [capturedPiece, isEPCapture] = get_white_capturable_pieces(B, rightPawnCaptureSquare);

            if (B.getRank(currPawnIndex - 7) == 0){
                gen_black_pawn_promotions(currPawnIndex, currPawnIndex - 7, moves,
                    (isEPCapture ? static_cast<int>(Move::Flags::EP_CAPTURE) : 0) |
                    static_cast<int>(Move::Flags::CAPTURE),
                    static_cast<int>(capturedPiece));
            } else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex - 7,
                    static_cast<int>(Move::Piece::BP),
                    static_cast<int>(capturedPiece),
                    0,
                    (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) |
                    static_cast<int>(Move::Flags::CAPTURE));
                moves.push_back(m);
            }
        }
    }
}

void MoveGen::gen_pawn_moves(const Chess::Board &B, std::vector<MoveType> &moves, bool whiteTurn){
    
    if(!whiteTurn){
        gen_black_pawn_captures(B,moves);
        gen_black_pawn_double_push(B,moves);
        gen_black_pawn_single_push(B,moves);
    }
    else{
        gen_white_pawn_captures(B,moves);
        gen_white_pawn_double_push(B,moves);
        gen_white_pawn_single_push(B,moves);
    }
}
