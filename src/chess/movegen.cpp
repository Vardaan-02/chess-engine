#include "movegen.h"

#define ONE 1ULL

// All positions where king can move from sq (even illegal ones)
uint64_t all_king_moves_from(int sq)
{
    uint64_t currPos = (ONE << sq);

    uint64_t up         = currPos << 8;
    uint64_t down       = currPos >> 8;
    uint64_t left       = (currPos >> 1) & ~FILE_H;
    uint64_t right      = (currPos << 1) & ~FILE_A;
    uint64_t up_left    = (currPos << 7) & ~FILE_H;
    uint64_t up_right   = (currPos << 9) & ~FILE_A;
    uint64_t down_left  = (currPos >> 9) & ~FILE_H;
    uint64_t down_right = (currPos >> 7) & ~FILE_A;

    return up | down | left | right | up_left | up_right | down_left | down_right;

}

uint64_t all_knight_moves_from(int sq)
{
    uint64_t currPos = (ONE << sq);

    uint64_t right1 = (currPos << 1) & (~FILE_A);           //Prevents going out of the board
    uint64_t right2 = (currPos << 2) & (~(FILE_A | FILE_B));

    uint64_t left1 = (currPos >> 1) & (~FILE_H);
    uint64_t left2 = (currPos >> 2) & (~(FILE_H | FILE_G));

    return ((left1 >> 16) | (left1 << 16) | (right1 >> 16) | (right1 << 16) | (left2 >> 8) | (left2 << 8) | (right2 >> 8) | (right2 << 8));   
}

// all possible moves of knight and king added in array
void init_leapers()
{
    for(int sq_no = 0 ; sq_no < NUM_SQUARES ; ++sq_no)
    {
        all_king_moves[sq_no] = all_king_moves_from(sq_no);
        all_knight_moves[sq_no] = all_knight_moves_from(sq_no);
    }
}

void MoveGen::gen_white_pawn_promotions(int currPawnIndex, int nextPawnIndex, std::vector<MoveType>& moves, int flags, int capturedPiece){
    std::vector<int> whitePieces = {static_cast<int>(Move::Piece::WQ), static_cast<int>(Move::Piece::WR), static_cast<int>(Move::Piece::WB), static_cast<int>(Move::Piece::WN)};
                
    for (auto piece : whitePieces){
        MoveType m = Move::encode(currPawnIndex, nextPawnIndex, static_cast<int>(Move::Piece::WP), capturedPiece, piece, flags | static_cast<int>(Move::Flags::PROMOTION));
        moves.push_back(m);
    }
}


// getting all possible pawn moves with promotion 
void MoveGen::gen_white_pawn_single_push(const Chess::Board &B, std::vector<MoveType> &moves)
{
    uint64_t whitePawns = B.WP;

    while(whitePawns) {
        int currPawnIndex = pop_lsb(whitePawns);
        
        uint64_t occupied = B.getOccupied(); 
        
        uint64_t currPawnSquare = (ONE << currPawnIndex); // current position

        uint64_t possibleNextPawnSquare = (currPawnSquare << 8);
        
        if( !(possibleNextPawnSquare & occupied) ){ // if move possible
            if ( B.getRank(currPawnIndex + 8) == 7 ){
                gen_white_pawn_promotions(currPawnIndex, currPawnIndex + 8, moves, 0, 0);
            }
            else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex + 8, static_cast<int>(Move::Piece::WP), 0, 0, 0);
                moves.push_back(m);
            }
        }
    }
}

void MoveGen::gen_white_pawn_double_push(const Chess::Board &B, std::vector<MoveType> &moves){
    uint64_t whitePawns = B.WP;

    while (whitePawns){
        int currPawnIndex = pop_lsb(whitePawns);

        if ( B.getRank(currPawnIndex) == 1 ){
            uint64_t occupied = B.getOccupied();

            uint64_t currPawnSquare = (ONE << currPawnIndex) ;
            uint64_t nextSquare = (currPawnSquare << 8) ;
            uint64_t nextPawnSquare = (nextSquare << 8) ;

            if (! (occupied & (nextPawnSquare | nextSquare) )){
                MoveType m = Move::encode(currPawnIndex, currPawnIndex + 16, static_cast<int>(Move::Piece::WP), 0, 0, static_cast<int>(Move::Flags::DOUBLE_PUSH));
                moves.push_back(m);
            }
        }
    }
}

std::pair<Move::Piece, bool> MoveGen::get_black_capturable_pieces(const Chess::Board& B, uint64_t targetSquare)
{
    if ( B.en_passant_index != -1 && (ONE << B.en_passant_index) & targetSquare){
        return {Move::Piece::BP, 1};
    }

    // Map each piece to its corresponding bitboard in Board
    const std::pair<Move::Piece, uint64_t> pieceBitboards[] = {
        { Move::Piece::BP, B.BP },
        { Move::Piece::BQ, B.BQ },
        { Move::Piece::BR, B.BR },
        { Move::Piece::BB, B.BB },
        { Move::Piece::BN, B.BN }
    };

    for (const auto& [piece, bitboard] : pieceBitboards) {
        if (bitboard & targetSquare) {
            return {piece, 0}; // Found the capturable piece
        }
    }

    // If no piece is found, you could return a sentinel value or throw
    // For example, assuming Move::Piece::Empty exists:
    return {Move::Piece::EMPTY, 0};
}


void MoveGen::gen_white_pawn_captures(const Chess::Board &B, std::vector<MoveType> &moves){
    uint64_t whitePawns = B.WP;

    while (whitePawns){
        int currPawnIndex = pop_lsb(whitePawns);

        uint64_t currPawnSquare = (ONE << currPawnIndex) ;
        uint64_t leftPawnCaptureSquare = (currPawnSquare << 7) & (~FILE_H); // get left + get up + check out of bound
        uint64_t rightPawnCaptureSquare = (currPawnSquare << 9) & (~FILE_A); // get right  + get up + check out of bound

        uint64_t blackOccupied = B.getBlack();

        if (leftPawnCaptureSquare & blackOccupied){
            auto [capturedPiece, isEPCapture] = get_black_capturable_pieces(B, leftPawnCaptureSquare);

            // promotion 
            if ( B.getRank(currPawnIndex + 7) == 7 ){
                gen_white_pawn_promotions(currPawnIndex, currPawnIndex + 7, moves , (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) | static_cast<int>(Move::Flags::CAPTURE), static_cast<int>(capturedPiece));
            }

            // no promotion
            else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex + 7, static_cast<int>(Move::Piece::WP), static_cast<int>(capturedPiece), 0, (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) | static_cast<int>(Move::Flags::CAPTURE));
                moves.push_back(m);
            }
        }
        if (rightPawnCaptureSquare & blackOccupied){
            auto [capturedPiece, isEPCapture] = get_black_capturable_pieces(B, rightPawnCaptureSquare);
            // promotion 
            if ( B.getRank(currPawnIndex + 9) == 7 ){
                gen_white_pawn_promotions(currPawnIndex,currPawnIndex +9, moves, (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) | static_cast<int>(Move::Flags::CAPTURE), static_cast<int>(capturedPiece));
            }

            // no promotion
            else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex + 9, static_cast<int>(Move::Piece::WP), static_cast<int>(capturedPiece), 0, (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) | static_cast<int>(Move::Flags::CAPTURE));
                moves.push_back(m);
            }
        }
    }
}

void MoveGen::gen_black_pawn_promotions(int currPawnIndex,int nextPawnIndex, std::vector<MoveType>& moves, int flags, int capturedPiece){
    std::vector<int> blackPieces = {static_cast<int>(Move::Piece::BQ), static_cast<int>(Move::Piece::BR), static_cast<int>(Move::Piece::BB), static_cast<int>(Move::Piece::BN)};
                
    for (auto piece : blackPieces){
        MoveType m = Move::encode(currPawnIndex, nextPawnIndex, static_cast<int>(Move::Piece::BP), capturedPiece, piece, flags | static_cast<int>(Move::Flags::PROMOTION));
        moves.push_back(m);
    }
}

// getting all possible pawn moves with promotion 
void MoveGen::gen_black_pawn_single_push(const Chess::Board &B, std::vector<MoveType> &moves)
{
    uint64_t blackPawns = B.BP;

    while(blackPawns) {
        int currPawnIndex = pop_lsb(blackPawns);
        
        uint64_t occupied = B.getOccupied(); 
        
        uint64_t currPawnSquare = (ONE << currPawnIndex); // current position

        uint64_t possibleNextPawnSquare = (currPawnSquare >> 8);
        
        if( !(possibleNextPawnSquare & occupied) ){ // if move possible
            if ( B.getRank(currPawnIndex - 8) == 0 ){
                gen_black_pawn_promotions(currPawnIndex, currPawnIndex - 8, moves, 0, 0);
            }
            else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex - 8, static_cast<int>(Move::Piece::BP), 0, 0, 0);
                moves.push_back(m);
            }
        }
    }
}

void MoveGen::gen_black_pawn_double_push(const Chess::Board &B, std::vector<MoveType> &moves){
    uint64_t blackPawns = B.BP;

    while (blackPawns){
        int currPawnIndex = pop_lsb(blackPawns);

        if ( B.getRank(currPawnIndex) == 6 ){
            uint64_t occupied = B.getOccupied();

            uint64_t currPawnSquare = (ONE << currPawnIndex) ;
            uint64_t nextSquare = (currPawnSquare >> 8) ;
            uint64_t nextPawnSquare = (nextSquare >> 8) ;

            if (! (occupied & (nextPawnSquare | nextSquare) )){
                MoveType m = Move::encode(currPawnIndex, currPawnIndex - 16, static_cast<int>(Move::Piece::BP), 0, 0, static_cast<int>(Move::Flags::DOUBLE_PUSH));
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

    // Map each white piece to its corresponding bitboard in Board
    const std::pair<Move::Piece, uint64_t> pieceBitboards[] = {
        { Move::Piece::WP, B.WP },
        { Move::Piece::WQ, B.WQ },
        { Move::Piece::WR, B.WR },
        { Move::Piece::WB, B.WB },
        { Move::Piece::WN, B.WN }
    };

    for (const auto& [piece, bitboard] : pieceBitboards) {
        if (bitboard & targetSquare) {
            return {piece, 0}; // Found the capturable piece
        }
    }

    // If no piece is found, return sentinel value
    return {Move::Piece::EMPTY, 0};
}

void MoveGen::gen_black_pawn_captures(const Chess::Board &B, std::vector<MoveType> &moves){
    uint64_t blackPawns = B.BP;

    while (blackPawns){
        int currPawnIndex = pop_lsb(blackPawns);

        uint64_t currPawnSquare = (ONE << currPawnIndex) ;
        uint64_t leftPawnCaptureSquare = (currPawnSquare >> 9) & (~FILE_H); // get left + get up + check out of bound
        uint64_t rightPawnCaptureSquare = (currPawnSquare >> 7) & (~FILE_A); // get right  + get up + check out of bound

        uint64_t whiteOccupied = B.getWhite();

        if (leftPawnCaptureSquare & whiteOccupied){
            auto [capturedPiece, isEPCapture] = get_white_capturable_pieces(B, leftPawnCaptureSquare);
            // promotion 
            if ( B.getRank(currPawnIndex - 9) == 0 ){
                gen_black_pawn_promotions(currPawnIndex, currPawnIndex - 9, moves, (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) | static_cast<int>(Move::Flags::CAPTURE), static_cast<int>(capturedPiece));
            }

            // no promotion
            else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex - 9, static_cast<int>(Move::Piece::BP), static_cast<int>(capturedPiece), 0, (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) | static_cast<int>(Move::Flags::CAPTURE));
                moves.push_back(m);
            }
        }
        if (rightPawnCaptureSquare & whiteOccupied){
            auto [capturedPiece, isEPCapture] = get_white_capturable_pieces(B, rightPawnCaptureSquare);
            // promotion 
            if ( B.getRank(currPawnIndex - 7) == 0 ){
                gen_black_pawn_promotions(currPawnIndex, currPawnIndex - 7, moves, (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) | static_cast<int>(Move::Flags::CAPTURE), static_cast<int>(capturedPiece));
            }

            // no promotion
            else {
                MoveType m = Move::encode(currPawnIndex, currPawnIndex - 7, static_cast<int>(Move::Piece::BP), static_cast<int>(capturedPiece), 0, (isEPCapture && static_cast<int>(Move::Flags::EP_CAPTURE)) | static_cast<int>(Move::Flags::CAPTURE));
                moves.push_back(m);
            }
        }
    }
}


// pins missing
void MoveGen::gen_pawn_moves(const Chess::Board &B, std::vector<MoveType> &moves){
    gen_black_pawn_captures(B,moves);
    gen_black_pawn_double_push(B,moves);
    gen_black_pawn_single_push(B,moves);

    gen_white_pawn_captures(B,moves);
    gen_white_pawn_double_push(B,moves);
    gen_white_pawn_single_push(B,moves);
}

void MoveGen::gen_white_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves) {
    uint64_t whiteKnights = B.WN;
    uint64_t white = B.getWhite();
    uint64_t black = B.getBlack();

    while (whiteKnights) {
        int currPos = pop_lsb(whiteKnights);
        uint64_t mask = all_knight_moves[currPos];

        // Remove friendly squares
        mask &= ~white;

        while (mask) {
            int nextPos = pop_lsb(mask);

            int capturedPiece = (black & (1ULL << nextPos)) ? static_cast<int>(get_black_capturable_pieces(B, ONE << nextPos).first) : 0;

            MoveType m = Move::encode(currPos, nextPos, static_cast<int>(Move::Piece::WN), capturedPiece);
            moves.push_back(m);
        }
    }
}

void MoveGen::gen_black_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves) {
    uint64_t blackKnights = B.BN;
    uint64_t white = B.getWhite();
    uint64_t black = B.getBlack();

    while (blackKnights) {
        int currPos = pop_lsb(blackKnights);
        uint64_t mask = all_knight_moves[currPos];

        // Remove friendly squares
        mask &= ~black;

        while (mask) {
            int nextPos = pop_lsb(mask);

            int capturedPiece = (white & (1ULL << nextPos)) ? static_cast<int>(get_white_capturable_pieces(B, ONE << nextPos).first) : 0;

            MoveType m = Move::encode(currPos, nextPos, static_cast<int>(Move::Piece::BN), capturedPiece);
            moves.push_back(m);
        }
    }
}

void MoveGen::gen_knight_moves(const Chess::Board &B, std::vector<MoveType> &moves) {
    gen_white_knight_moves(B, moves);
    gen_black_knight_moves(B, moves);
}

void MoveGen::gen_white_king_moves(const Chess::Board &B, std::vector<MoveType> &moves)
{
    uint64_t whiteKing = B.WK;
    uint64_t white = B.getWhite();
    uint64_t black = B.getBlack();

    while (whiteKing) {
        int currPos = pop_lsb(whiteKing);
        uint64_t mask = all_king_moves[currPos];

        // Remove friendly squares
        mask &= ~white;

        while (mask) {
            int nextPos = pop_lsb(mask);

            int capturedPiece = (black & (1ULL << nextPos)) ? static_cast<int>(get_black_capturable_pieces(B, ONE << nextPos).first) : 0;

            MoveType m = Move::encode(currPos, nextPos, static_cast<int>(Move::Piece::WK), capturedPiece);
            moves.push_back(m);
        }
    }
}

void MoveGen::gen_black_king_moves(const Chess::Board &B, std::vector<MoveType> &moves)
{
    uint64_t blackKing = B.BK;
    uint64_t white = B.getWhite();
    uint64_t black = B.getBlack();

    while (blackKing) {
        int currPos = pop_lsb(blackKing);
        uint64_t mask = all_king_moves[currPos];

        // Remove friendly squares
        mask &= ~black;

        while (mask) {
            int nextPos = pop_lsb(mask);

            int capturedPiece = (white & (1ULL << nextPos)) ? static_cast<int>(get_white_capturable_pieces(B, ONE << nextPos).first) : 0;

            MoveType m = Move::encode(currPos, nextPos, static_cast<int>(Move::Piece::BK), capturedPiece);
            moves.push_back(m);
        }
    }
}
