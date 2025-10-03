#pragma once
#include <cstdint>
#include <iostream>

namespace Chess {

class Board {
public:
    // White pieces
    uint64_t WP, WN, WB, WR, WQ, WK;
    // Black pieces
    uint64_t BP, BN, BB, BR, BQ, BK;

    uint64_t white, black, occupied;

    bool white_to_move;      // true = white to move
    int castling_rights;     // 4-bit mask: KQkq
    int en_passant_square;   // -1 if none
    int halfmove_clock;
    int fullmove_number;

    // Constructor: initializes empty board
    Board();

    // --------------------
    // Bitboard helpers
    // --------------------
    void set_piece(uint64_t &bb, int sq);
    void remove_piece(uint64_t &bb, int sq);
    bool is_occupied(uint64_t bb, int sq) const;
    int square_index(int rank, int file) const;

    inline uint64_t getWhite() const { return (WP | WN | WB | WR | WQ | WK) ; }
    inline uint64_t getblack() const { return (BP | BN | BB | BR | BQ | BK) ; }
    inline uint64_t getOccupied() const { return (WP | WN | WB | WR | WQ | WK | BP | BN | BB | BR | BQ | BK) ; }

    inline int getRank(int sq) const { return sq / 8 ; }
    inline int getFile(int sq) const { return sq % 8 ; }

    // Print ASCII board for debugging
    void print_board() const;
};

} // namespace Chess
