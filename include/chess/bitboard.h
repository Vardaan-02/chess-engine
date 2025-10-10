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
#include "util.h"
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
inline uint64_t get_orthogonal_slider_attacks(Square s, uint64_t occupancy) {
    occupancy &= RookMagics[s].mask;
    occupancy *= RookMagics[s].magic;
    occupancy >>= RookMagics[s].shift;
    return RookAttacks[s][occupancy];
}

// Generates bishop attacks using the magic uint64_t lookup.
inline uint64_t get_diagonal_slider_attacks(Square s, uint64_t occupancy) {
    occupancy &= BishopMagics[s].mask;
    occupancy *= BishopMagics[s].magic;
    occupancy >>= BishopMagics[s].shift;
    return BishopAttacks[s][occupancy];
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

extern uint64_t Between[chess::SQUARE_NB][chess::SQUARE_NB];
extern uint64_t Rays[chess::SQUARE_NB][chess::SQUARE_NB];

// generator (can be used to fill the tables at program init if not constexpr-hardened)
void generate_between_and_ray_tables() noexcept;

} // namespace chess