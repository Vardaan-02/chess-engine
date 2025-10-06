#pragma once

/**
 * @file bitboard.h
 * @brief Defines the Bitboard type and all related manipulation functions.
 *
 * This file is the core of the engine's board representation and move generation.
 * It includes highly optimized functions for bit manipulation, bit scanning,
 * and generating attack sets for all piece types using pre-computed tables
 * and the "Magic Bitboard" technique for sliders.
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

// A Bitboard is a 64-bit unsigned integer where each bit represents a square.
using Bitboard = uint64_t;

//-----------------------------------------------------------------------------
// CONSTANTS
//-----------------------------------------------------------------------------
namespace BB {
    const Bitboard Empty = 0ULL;
    const Bitboard Universal = ~0ULL;

    const Bitboard FileA = 0x0101010101010101ULL;
    const Bitboard FileB = FileA << 1;
    const Bitboard FileC = FileA << 2;
    const Bitboard FileD = FileA << 3;
    const Bitboard FileE = FileA << 4;
    const Bitboard FileF = FileA << 5;
    const Bitboard FileG = FileA << 6;
    const Bitboard FileH = FileA << 7;

    const Bitboard Rank1 = 0xFFULL;
    const Bitboard Rank2 = Rank1 << (8 * 1);
    const Bitboard Rank3 = Rank1 << (8 * 2);
    const Bitboard Rank4 = Rank1 << (8 * 3);
    const Bitboard Rank5 = Rank1 << (8 * 4);
    const Bitboard Rank6 = Rank1 << (8 * 5);
    const Bitboard Rank7 = Rank1 << (8 * 6);
    const Bitboard Rank8 = Rank1 << (8 * 7);

    // --- ADD THESE TWO ARRAYS ---

    // Lookup table to get a bitboard of a square's file.
    // e.g., File[E4] will return the bitboard for File E.
    constexpr Bitboard File[SQUARE_NB] = {
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH,
        FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH
    };

    // Lookup table to get a bitboard of a square's rank.
    // e.g., Rank[E4] will return the bitboard for Rank 4.
    constexpr Bitboard Rank[SQUARE_NB] = {
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
// CORE BIT MANIPULATION
//-----------------------------------------------------------------------------

// Check if a bit is set at a given square
constexpr bool get_bit(Bitboard bb, Square s) {
    return (bb >> s) & 1;
}

// Set a bit at a given square
inline void set_bit(Bitboard& bb, Square s) {
    bb |= (1ULL << s);
}

// Clear (pop) a bit at a given square
inline void pop_bit(Bitboard& bb, Square s) {
    bb &= ~(1ULL << s);
}


//-----------------------------------------------------------------------------
// BIT SCANNING (LSB, POPCOUNT)
//-----------------------------------------------------------------------------

// Count the number of set bits in a bitboard
inline int count_bits(Bitboard bb) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(bb);
#elif defined(_MSC_VER)
    return (int)__popcnt64(bb);
#else
    // Fallback implementation if no intrinsics are available
    int count = 0;
    while (bb) {
        bb &= bb - 1;
        count++;
    }
    return count;
#endif
}

// Get the index of the least significant bit (LSB)
inline Square lsb(Bitboard bb) {
    // Assert that the bitboard is not empty
    // assert(bb != 0);
#if defined(__GNUC__) || defined(__clang__)
    return Square(__builtin_ctzll(bb));
#elif defined(_MSC_VER)
    unsigned long index;
    _BitScanForward64(&index, bb);
    return (Square)index;
#else
    // Fallback implementation
    if (bb == 0) return SQUARE_NONE;
    int count = 0;
    while (!((bb >> count) & 1))
        count++;
    return (Square)count;
#endif
}

// Get and remove the LSB from a bitboard
inline Square pop_lsb(Bitboard& bb) {
    Square s = lsb(bb);
    bb &= bb - 1; // Efficiently removes the LSB
    return s;
}


//-----------------------------------------------------------------------------
// PRE-COMPUTED ATTACK TABLES & INITIALIZATION
//
// These tables are defined in bitboard.cpp and populated by the init() function.
//-----------------------------------------------------------------------------

// A one-time initialization function to be called at program startup.
void init();

// Pawn attacks [color][square]
extern Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
// Knight attacks [square]
extern Bitboard KnightAttacks[SQUARE_NB];
// King attacks [square]
extern Bitboard KingAttacks[SQUARE_NB];

//-----------------------------------------------------------------------------
// MAGIC BITBOARDS FOR SLIDER PIECES (ROOK, BISHOP)
//-----------------------------------------------------------------------------

// Magic struct to hold data for magic bitboard lookups
struct Magic {
    Bitboard mask;       // Mask to isolate relevant blocker squares
    Bitboard magic;      // The "magic" number
    uint8_t shift;       // Shift value for hashing
};

// Extern declarations for magic numbers and attack tables (defined in bitboard.cpp)
extern Magic RookMagics[SQUARE_NB];
extern Magic BishopMagics[SQUARE_NB];
extern Bitboard RookAttacks[SQUARE_NB][4096];  // Max 2^12 relevant squares for rooks
extern Bitboard BishopAttacks[SQUARE_NB][512]; // Max 2^9 relevant squares for bishops

// Generates rook attacks using the magic bitboard lookup.
inline Bitboard get_rook_attacks(Square s, Bitboard occupancy) {
    occupancy &= RookMagics[s].mask;
    occupancy *= RookMagics[s].magic;
    occupancy >>= RookMagics[s].shift;
    return RookAttacks[s][occupancy];
}

// Generates bishop attacks using the magic bitboard lookup.
inline Bitboard get_bishop_attacks(Square s, Bitboard occupancy) {
    occupancy &= BishopMagics[s].mask;
    occupancy *= BishopMagics[s].magic;
    occupancy >>= BishopMagics[s].shift;
    return BishopAttacks[s][occupancy];
}

// Generates queen attacks by combining rook and bishop attacks.
inline Bitboard get_queen_attacks(Square s, Bitboard occupancy) {
    return get_rook_attacks(s, occupancy) | get_bishop_attacks(s, occupancy);
}


//-----------------------------------------------------------------------------
// DEBUGGING
//-----------------------------------------------------------------------------

// Prints a visual representation of a bitboard to the console.
inline void print_bitboard(Bitboard bb) {
    std::cout << "\n";
    for (int r = 7; r >= 0; --r) {
        std::cout << " " << (r + 1) << " |";
        for (int f = 0; f < 8; ++f) {
            Square s = Square(r * 8 + f);
            std::cout << " " << get_bit(bb, s);
        }
        std::cout << "\n";
    }
    std::cout << "   +----------------\n     a b c d e f g h\n\n"
              << " Bitboard: " << bb << "ULL\n"
              << " Popcount: " << count_bits(bb) << "\n" << std::endl;
}

} // namespace chess