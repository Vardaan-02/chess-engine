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


extern uint64_t files[8];
extern uint64_t ranks[8];

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

const uint64_t passed_pawn_masks_white[SQUARE_NB] = {217020518514230016ULL, 506381209866536704ULL, 1012762419733073408ULL, 2025524839466146816ULL, 4051049678932293632ULL, 8102099357864587264ULL, 16204198715729174528ULL, 13889313184910721024ULL, 217020518514229248ULL, 506381209866534912ULL, 1012762419733069824ULL, 2025524839466139648ULL, 4051049678932279296ULL, 8102099357864558592ULL, 16204198715729117184ULL, 13889313184910671872ULL, 217020518514032640ULL, 506381209866076160ULL, 1012762419732152320ULL, 2025524839464304640ULL, 4051049678928609280ULL, 8102099357857218560ULL, 16204198715714437120ULL, 13889313184898088960ULL, 217020518463700992ULL, 506381209748635648ULL, 1012762419497271296ULL, 2025524838994542592ULL, 4051049677989085184ULL, 8102099355978170368ULL, 16204198711956340736ULL, 13889313181676863488ULL, 217020505578799104ULL, 506381179683864576ULL, 1012762359367729152ULL, 2025524718735458304ULL, 4051049437470916608ULL, 8102098874941833216ULL, 16204197749883666432ULL, 13889312357043142656ULL, 217017207043915776ULL, 506373483102470144ULL, 1012746966204940288ULL, 2025493932409880576ULL, 4050987864819761152ULL, 8101975729639522304ULL, 16203951459279044608ULL, 13889101250810609664ULL, 216172782113783808ULL, 504403158265495552ULL, 1008806316530991104ULL, 2017612633061982208ULL, 4035225266123964416ULL, 8070450532247928832ULL, 16140901064495857664ULL, 13835058055282163712ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL};

const uint64_t passed_pawn_masks_black[SQUARE_NB] = {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 3ULL, 7ULL, 14ULL, 28ULL, 56ULL, 112ULL, 224ULL, 192ULL, 771ULL, 1799ULL, 3598ULL, 7196ULL, 14392ULL, 28784ULL, 57568ULL, 49344ULL, 197379ULL, 460551ULL, 921102ULL, 1842204ULL, 3684408ULL, 7368816ULL, 14737632ULL, 12632256ULL, 50529027ULL, 117901063ULL, 235802126ULL, 471604252ULL, 943208504ULL, 1886417008ULL, 3772834016ULL, 3233857728ULL, 12935430915ULL, 30182672135ULL, 60365344270ULL, 120730688540ULL, 241461377080ULL, 482922754160ULL, 965845508320ULL, 827867578560ULL, 3311470314243ULL, 7726764066567ULL, 15453528133134ULL, 30907056266268ULL, 61814112532536ULL, 123628225065072ULL, 247256450130144ULL, 211934100111552ULL, 847736400446211ULL, 1978051601041159ULL, 3956103202082318ULL, 7912206404164636ULL, 15824412808329272ULL, 31648825616658544ULL, 63297651233317088ULL, 54255129628557504ULL};

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