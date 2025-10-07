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


namespace chess {

//-----------------------------------------------------------------------------
// CONSTANTS
//-----------------------------------------------------------------------------
namespace Bitboard {
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

    // --- ADD THESE TWO ARRAYS ---

    // Lookup table to get a uint64_t of a square's file.
    // e.g., File[E4] will return the uint64_t for File E.
    constexpr uint64_t File[SQUARE_NB] = {
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
    constexpr uint64_t Rank[SQUARE_NB] = {
        Rank1, Rank1, Rank1, Rank1, Rank1, Rank1, Rank1, Rank1,
        Rank2, Rank2, Rank2, Rank2, Rank2, Rank2, Rank2, Rank2,
        Rank3, Rank3, Rank3, Rank3, Rank3, Rank3, Rank3, Rank3,
        Rank4, Rank4, Rank4, Rank4, Rank4, Rank4, Rank4, Rank4,
        Rank5, Rank5, Rank5, Rank5, Rank5, Rank5, Rank5, Rank5,
        Rank6, Rank6, Rank6, Rank6, Rank6, Rank6, Rank6, Rank6,
        Rank7, Rank7, Rank7, Rank7, Rank7, Rank7, Rank7, Rank7,
        Rank8, Rank8, Rank8, Rank8, Rank8, Rank8, Rank8, Rank8
    };
}


//-----------------------------------------------------------------------------
// PRE-COMPUTED ATTACK TABLES & INITIALIZATION
//
// These tables are defined in uint64_t.cpp and populated by the init() function.
//-----------------------------------------------------------------------------

// A one-time initialization function to be called at program startup.
void init();

// Pawn attacks [color][square]
extern uint64_t PawnAttacks[COLOR_NB][SQUARE_NB];
// Knight attacks [square]
extern uint64_t KnightAttacks[SQUARE_NB];
// King attacks [square]
extern uint64_t KingAttacks[SQUARE_NB];

//-----------------------------------------------------------------------------
// MAGIC BITBOARDS FOR SLIDER PIECES (ROOK, BISHOP)
//-----------------------------------------------------------------------------

// Magic struct to hold data for magic uint64_t lookups
struct Magic {
    uint64_t mask;       // Mask to isolate relevant blocker squares
    uint64_t magic;      // The "magic" number
    uint8_t shift;       // Shift value for hashing
};

// Extern declarations for magic numbers and attack tables (defined in uint64_t.cpp)
extern Magic RookMagics[SQUARE_NB];
extern Magic BishopMagics[SQUARE_NB];
extern uint64_t RookAttacks[SQUARE_NB][4096];  // Max 2^12 relevant squares for rooks
extern uint64_t BishopAttacks[SQUARE_NB][512]; // Max 2^9 relevant squares for bishops

// Generates rook attacks using the magic uint64_t lookup.
inline uint64_t get_rook_attacks(Square s, uint64_t occupancy) {
    occupancy &= RookMagics[s].mask;
    occupancy *= RookMagics[s].magic;
    occupancy >>= RookMagics[s].shift;
    return RookAttacks[s][occupancy];
}

// Generates bishop attacks using the magic uint64_t lookup.
inline uint64_t get_bishop_attacks(Square s, uint64_t occupancy) {
    occupancy &= BishopMagics[s].mask;
    occupancy *= BishopMagics[s].magic;
    occupancy >>= BishopMagics[s].shift;
    return BishopAttacks[s][occupancy];
}

// Generates queen attacks by combining rook and bishop attacks.
inline uint64_t get_queen_attacks(Square s, uint64_t occupancy) {
    return get_rook_attacks(s, occupancy) | get_bishop_attacks(s, occupancy);
}


//-----------------------------------------------------------------------------
// DEBUGGING
//-----------------------------------------------------------------------------

// Prints a visual representation of a uint64_t to the console.
inline void print_bitboard(uint64_t bb) {
    std::cout << "\n";
    for (int r = 7; r >= 0; --r) {
        std::cout << " " << (r + 1) << " |";
        for (int f = 0; f < 8; ++f) {
            Square s = Square(r * 8 + f);
            std::cout << " " << util::get_bit(bb, s);
        }
        std::cout << "\n";
    }
    std::cout << "   +----------------\n     a b c d e f g h\n\n"
              << " uint64_t: " << bb << "ULL\n"
              << " Popcount: " << util::count_bits(bb) << "\n" << std::endl;
}

} // namespace chess

namespace util{
    inline uint64_t create_bitboard_from_square(chess::Square s){
        return (ONE << s);
    }

    // Takes in a square and returns a square
    inline chess::Square shift_square(chess::Square square, chess::Direction dir)
    {   
        uint64_t bitboard = create_bitboard_from_square(square);
        switch(dir){
            case chess::NORTH:         return (chess::Square)lsb(bitboard << 8);
            case chess::SOUTH:         return (chess::Square)lsb(bitboard >> 8);
            case chess::EAST:          return (chess::Square)lsb((get_file(lsb(bitboard)) == 7) ? 0ULL : (bitboard >> 1));
            case chess::WEST:          return (chess::Square)lsb((get_file(lsb(bitboard)) == 0) ? 0ULL : (bitboard << 1));
            case chess::NORTH_WEST:    return (chess::Square)lsb((get_file(lsb(bitboard)) == 0) ? 0ULL : (bitboard << 7));
            case chess::NORTH_EAST:    return (chess::Square)lsb((get_file(lsb(bitboard)) == 7) ? 0ULL : (bitboard << 9));
            case chess::SOUTH_EAST:    return (chess::Square)lsb((get_file(lsb(bitboard)) == 7) ? 0ULL : (bitboard >> 7));
            case chess::SOUTH_WEST:    return (chess::Square)lsb((get_file(lsb(bitboard)) == 0) ? 0ULL : (bitboard >> 9)); 
        }
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
};