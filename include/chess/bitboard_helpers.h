// bitboard_helpers.h
#pragma once
#include <cstdint>
#include <cassert>

using U64 = uint64_t;
constexpr U64 ONE = 1ULL;

// Basic masks
//Masks for files and ranks
constexpr U64 FILE_A = 0x0101010101010101ULL;
constexpr U64 FILE_B = FILE_A << 1;
constexpr U64 FILE_G = FILE_A << 6;
constexpr U64 FILE_H = FILE_A << 7;

constexpr U64 RANK_1 = 0xFFULL;
constexpr U64 RANK_2 = RANK_1 << 8;
constexpr U64 RANK_3 = RANK_1 << 16;
constexpr U64 RANK_4 = RANK_1 << 24;
constexpr U64 RANK_5 = RANK_1 << 32;
constexpr U64 RANK_6 = RANK_1 << 40;
constexpr U64 RANK_7 = RANK_1 << 48;
constexpr U64 RANK_8 = RANK_1 << 56;

// Utility inline helpers
inline int lsb_index(U64 bb) { assert(bb); return __builtin_ctzll(bb); }    // Find the lowest square with a piece.
inline int popcount(U64 bb) { return __builtin_popcountll(bb); }            // Count how many pieces are on this bitboard.

// Remove & return index of least significant set bit   
//Take one piece out (lowest square), return where it was.
inline int pop_lsb(U64 &bb) {   
    int i = __builtin_ctzll(bb);
    bb &= bb - 1; // clear LSB
    return i;
}
inline void set_bit(U64 &bb, int sq) { bb |= (ONE << sq); }                 // Put a piece on sq.
inline void clear_bit(U64 &bb, int sq) { bb &= ~(ONE << sq); }              // Remove a piece from sq.      
inline bool test_bit(U64 bb, int sq) { return (bb >> sq) & 1ULL; }          // Check if sq has a piece.