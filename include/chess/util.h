#pragma once

/**
 * @file uint64_t.h
 * @brief Defines the uint64_t type and all related manipulation functions.
 *
 * This file is the core of the engine's board representation and move generation.
 * It includes highly optimized functions for bit manipulation, bit scanning,
 * and generating attack sets for all piece types using pre-computed tables
 * and the "Magic uint64_t" technique for sliders.
 */

#include "types.h"
#include <iostream>


// Include intrinsics headers for performance
#if defined(__GNUC__) || defined(__clang__)
#include <x86intrin.h>
#endif
#if defined(_MSC_VER)
#include <intrin.h>
#endif


namespace util{
    const uint64_t Empty = 0ULL;
    const uint64_t Universal = ~0ULL;

    const uint64_t FileA = 0x0101010101010101ULL;
    const uint64_t FileB = FileA << 1;
    const uint64_t FileC = FileA << 2;
    const uint64_t FileD = FileA << 3;
    const uint64_t FileE = FileA << 4;
    const uint64_t FileF = FileA << 5;
    const uint64_t FileG = FileA << 6;
    const uint64_t FileH = FileA << 7;
    
    const uint64_t Rank1 = 0xFFULL;
    const uint64_t Rank2 = Rank1 << (8 * 1);
    const uint64_t Rank3 = Rank1 << (8 * 2);
    const uint64_t Rank4 = Rank1 << (8 * 3);
    const uint64_t Rank5 = Rank1 << (8 * 4);
    const uint64_t Rank6 = Rank1 << (8 * 5);
    const uint64_t Rank7 = Rank1 << (8 * 6);
    const uint64_t Rank8 = Rank1 << (8 * 7);

    const uint64_t black_squares = 0x5555555555555555;
    const uint64_t white_squares = 0xAAAAAAAAAAAAAAAA;

    const uint64_t white_side_of_board = 0xFFFFFFFF;
    const uint64_t black_side_of_board = 0xFFFFFFFF00000000;
    
    inline uint64_t create_bitboard_from_square(chess::Square s){
        return (1ULL << s);
    }
    
    const uint64_t whiteKingSideBitboard = create_bitboard_from_square(chess::Square::F1) | create_bitboard_from_square(chess::Square::G1);
    const uint64_t whiteQueenSideBitboard = create_bitboard_from_square(chess::Square::B1) | create_bitboard_from_square(chess::Square::C1) | create_bitboard_from_square(chess::Square::D1);
    const uint64_t blackKingSideBitboard = create_bitboard_from_square(chess::Square::F8) | create_bitboard_from_square(chess::Square::G8);
    const uint64_t blackQueenSideBitboard = create_bitboard_from_square(chess::Square::B8) | create_bitboard_from_square(chess::Square::C8) | create_bitboard_from_square(chess::Square::D8);

    const uint64_t allEdgesBB = (Rank1 | Rank8 | FileA | FileH);
    // --- ADD THESE TWO ARRAYS ---

    // Lookup table to get a uint64_t of a square's file.
    // e.g., File[E4] will return the uint64_t for File E.
    constexpr uint64_t File[chess::SQUARE_NB] = {
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH
    };

    // Lookup table to get a uint64_t of a square's rank.
    // e.g., Rank[E4] will return the uint64_t for Rank 4.
    constexpr uint64_t Rank[chess::SQUARE_NB] = {
        Rank1, Rank1, Rank1, Rank1, Rank1, Rank1, Rank1, Rank1,
        Rank2, Rank2, Rank2, Rank2, Rank2, Rank2, Rank2, Rank2,
        Rank3, Rank3, Rank3, Rank3, Rank3, Rank3, Rank3, Rank3,
        Rank4, Rank4, Rank4, Rank4, Rank4, Rank4, Rank4, Rank4,
        Rank5, Rank5, Rank5, Rank5, Rank5, Rank5, Rank5, Rank5,
        Rank6, Rank6, Rank6, Rank6, Rank6, Rank6, Rank6, Rank6,
        Rank7, Rank7, Rank7, Rank7, Rank7, Rank7, Rank7, Rank7,
        Rank8, Rank8, Rank8, Rank8, Rank8, Rank8, Rank8, Rank8
    };

    constexpr int KNIGHT_PHASE = 1;
    constexpr int BISHOP_PHASE = 1;
    constexpr int ROOK_PHASE   = 2;
    constexpr int QUEEN_PHASE  = 4;
    constexpr int TOTAL_PHASE  = 24;

    constexpr int phase_values[] = {
        0, // NO PIECE
        0, // PAWN (has no phase value)
        1, // KNIGHT
        1, // BISHOP
        2, // ROOK
        4, // QUEEN
        0  // KING (has no phase value)
    };

    // Takes in a square and returns a square
    inline chess::Square shift_square(chess::Square square, chess::Direction dir) {
        if (square >= chess::SQUARE_NB) {
            return chess::SQUARE_NONE;
        }

        const uint64_t b = 1ULL << square;

        switch (dir) {
            case chess::NORTH:
                return (b & Rank8) ? chess::SQUARE_NONE : (chess::Square)(square + 8);
            case chess::SOUTH:
                return (b & Rank1) ? chess::SQUARE_NONE : (chess::Square)(square - 8);
            case chess::EAST:
                return (b & FileH) ? chess::SQUARE_NONE : (chess::Square)(square + 1);
            case chess::WEST:
                return (b & FileA) ? chess::SQUARE_NONE : (chess::Square)(square - 1);
            case chess::NORTH_EAST:
                return ((b & Rank8) || (b & FileH)) ? chess::SQUARE_NONE : (chess::Square)(square + 9);
            case chess::NORTH_WEST:
                return ((b & Rank8) || (b & FileA)) ? chess::SQUARE_NONE : (chess::Square)(square + 7);
            case chess::SOUTH_EAST:
                return ((b & Rank1) || (b & FileH)) ? chess::SQUARE_NONE : (chess::Square)(square - 7);
            case chess::SOUTH_WEST:
                return ((b & Rank1) || (b & FileA)) ? chess::SQUARE_NONE : (chess::Square)(square - 9);
        }

        return chess::SQUARE_NONE;
    }

    //Expects a bitboard with nothing on any of the edges of the direction to shift (trims the edges itself)
    // Shifts a bitboard, correctly handling wrap-around for each direction.
    inline uint64_t shift_board(uint64_t bitboard, chess::Direction dir)
    {
        switch(dir){
            case chess::NORTH:      return bitboard << 8;
            case chess::SOUTH:      return bitboard >> 8;
            case chess::EAST:       return (bitboard & (~FileH)) << 1;
            case chess::WEST:       return (bitboard & (~FileA)) >> 1;
            case chess::NORTH_EAST: return (bitboard & (~FileH)) << 9;
            case chess::NORTH_WEST: return (bitboard & (~FileA)) << 7;
            case chess::SOUTH_EAST: return (bitboard & (~FileH)) >> 7;
            case chess::SOUTH_WEST: return (bitboard & (~FileA)) >> 9;
        }
        return 0ULL; // Should not be reached
    }

    //-----------------------------------------------------------------------------
    // BIT SCANNING (LSB, POPCOUNT)
    //-----------------------------------------------------------------------------

    // Count the number of set bits in a uint64_t
    inline int count_bits(uint64_t bb) {
        return __builtin_popcountll(bb);
    }

    // Get the index of the least significant bit (LSB)
    inline chess::Square lsb(uint64_t bb) {
        return chess::Square(__builtin_ctzll(bb));
    }

    // Get the index of the most significant bit (MSB)
    inline chess::Square msb(uint64_t bb) {
        return chess::Square(63 - __builtin_clzll(bb));
    }

    // Get and remove the LSB from a uint64_t
    inline chess::Square pop_lsb(uint64_t& bb) {
        chess::Square s = lsb(bb);
        bb &= bb - 1; // Efficiently removes the LSB
        return s;
    }


    //-----------------------------------------------------------------------------
    // CORE BIT MANIPULATION
    //-----------------------------------------------------------------------------

    // Check if a bit is set at a given square
    constexpr bool get_bit(uint64_t bb, chess::Square s) {
        return (bb >> s) & 1;
    }

    // Set a bit at a given square
    inline void set_bit(uint64_t& bb, chess::Square s) {
        bb |= (1ULL << s);
    }

    // Clear (pop) a bit at a given square
    inline void pop_bit(uint64_t& bb, chess::Square s) {
        bb &= ~(1ULL << s);
    }

    // get file and rank
    inline int8_t get_file(chess::Square s)
    {
        return s%8;
    }

    inline int8_t get_rank(chess::Square s)
    {
        return s/8;
    }

    inline std::string square_to_string(int s) {
        std::string str = "";
        str += (char)('a' + (s % 8));
        str += (char)('1' + (s / 8));
        return str;
    }

    inline std::string move_to_string(const chess::Move& m) {
        if (m.from() == m.to()) return "NULL";
        return square_to_string(m.from()) + square_to_string(m.to());
    }

    constexpr chess::Square flip(chess::Square sq) {
    return chess::Square(sq ^ 56);
}
};
