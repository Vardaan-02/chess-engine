#pragma once

#include <cstdint>
#include <random>

class Board;

class Zobrist {
    public:
        static void init();
        static uint64_t calculate_zobrist_hash(const Board& B);
        //Key for every piece (both colors) on every square
        static uint64_t piecesArray[16][64];
        // Each side as 4 possible castling states: none, queenside, kingside, both (so 2^4 possible states for both sides combined)
        static uint64_t castlingRights[16];
        static uint64_t enPassantKey[64]; //one for each square, we xor in the target ep square 
        static uint64_t sideToMove;

};
