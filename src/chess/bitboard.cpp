#include "bitboard.h"
#include "types.h"

/**
 * @file uint64_t.cpp
 * @brief Implements the initialization of all pre-computed uint64_t data.
 *
 * This file contains the definitions for the attack tables and the logic
 * for the one-time `init()` function. This function populates the tables for
 * pawn, knight, and king attacks, and crucially, generates the complete set of
 * attacks for rooks and bishops using the Magic uint64_t technique.
 */

namespace chess {

//-----------------------------------------------------------------------------
// TABLE DEFINITIONS
//
// These are the actual memory allocations for the tables declared as 'extern'
// in the header file.
//-----------------------------------------------------------------------------

uint64_t PawnAttacks[COLOR_NB][SQUARE_NB];
uint64_t KnightAttacks[SQUARE_NB];
uint64_t KingAttacks[SQUARE_NB];

Magic RookMagics[SQUARE_NB];
Magic BishopMagics[SQUARE_NB];
uint64_t RookAttacks[SQUARE_NB][4096];
uint64_t BishopAttacks[SQUARE_NB][512];

//-----------------------------------------------------------------------------
// ANONYMOUS NAMESPACE FOR HELPER FUNCTIONS
//
// These functions are only used within this file to perform the one-time
// initialization of the attack tables.
//-----------------------------------------------------------------------------
namespace {

// Pre-computed magic numbers for rooks and bishops. These are standard values.
const Magic ROOK_MAGICS_INIT[SQUARE_NB] = {
    {0x1000020008080ULL, 0x8a80104000800020ULL, 52}, {0x20004000808000ULL, 0x4100200040080400ULL, 55},
    {0x400080010000000ULL, 0x2040010000200008ULL, 56}, {0x800100020000000ULL, 0x20800040000ULL, 59},
    {0x1000200040000000ULL, 0x4804000800100ULL, 59}, {0x200040008000000ULL, 0x12000200010040ULL, 58},
    {0x400080010000000ULL, 0x401004000800ULL, 57}, {0x800100020000000ULL, 0x408000802000100ULL, 56},
    {0x8040002000800ULL, 0x40200010080ULL, 54}, {0x4080200010000ULL, 0x20080080401000ULL, 54},
    {0x81000400020000ULL, 0x4002000040080ULL, 57}, {0x102000800400000ULL, 0x10000100004000ULL, 57},
    {0x204001000080000ULL, 0x2000002000010ULL, 57}, {0x408002000100000ULL, 0x4000001000020ULL, 57},
    {0x81000400020000ULL, 0x8000004000080ULL, 54}, {0x1000020008080ULL, 0x800001004000ULL, 52},
    {0x802001000400ULL, 0x8000402000100ULL, 53}, {0x4010008002000ULL, 0x10002000080040ULL, 55},
    {0x8020004001000ULL, 0x2000100004008ULL, 56}, {0x100040008002000ULL, 0x100008000200ULL, 59},
    {0x200080010000400ULL, 0x40001002000ULL, 59}, {0x400100020000800ULL, 0x80004000100ULL, 58},
    {0x80020004001000ULL, 0x20000800040ULL, 57}, {0x100040008080ULL, 0x10000100200ULL, 52},
    {0x400801000200ULL, 0x8000804000200ULL, 54}, {0x2004008001000ULL, 0x20004000100080ULL, 55},
    {0x4002000400801ULL, 0x4000800020001ULL, 56}, {0x800100020004000ULL, 0x800040001000ULL, 59},
    {0x100020004000800ULL, 0x10000800020ULL, 59}, {0x200040008001000ULL, 0x40000100008ULL, 58},
    {0x400080010002000ULL, 0x80000200010ULL, 57}, {0x800100020080ULL, 0x80000040010ULL, 52},
    {0x800400200100ULL, 0x2000800080400ULL, 54}, {0x4002001000800ULL, 0x10004000200080ULL, 55},
    {0x8001000800402ULL, 0x4000200010001ULL, 56}, {0x100020004000800ULL, 0x80001000040ULL, 59},
    {0x200040008001000ULL, 0x10000400008ULL, 59}, {0x400080010002000ULL, 0x20000800010ULL, 58},
    {0x800100020004000ULL, 0x400001000020ULL, 57}, {0x100020004080ULL, 0x80000080010ULL, 52},
    {0x1000800400200ULL, 0x4000802000400ULL, 53}, {0x20004002001000ULL, 0x80002000040010ULL, 54},
    {0x40002001000800ULL, 0x10001000020004ULL, 56}, {0x800010002000400ULL, 0x400008000100ULL, 59},
    {0x100020004000800ULL, 0x80001000020ULL, 59}, {0x200040008001000ULL, 0x10000400008ULL, 58},
    {0x400080010002000ULL, 0x20000800010ULL, 57}, {0x800100020080ULL, 0x80000040010ULL, 52},
    {0x804000200ULL, 0x1000804000200ULL, 53}, {0x4080001000ULL, 0x20001000080040ULL, 54},
    {0x8100020000ULL, 0x40000800020001ULL, 56}, {0x1020000000000ULL, 0x100004000080ULL, 59},
    {0x2040000000000ULL, 0x200008000100ULL, 59}, {0x4080000000000ULL, 0x400001000020ULL, 58},
    {0x8100000000000ULL, 0x800002000010ULL, 57}, {0x100000000080ULL, 0x40000080010ULL, 52}
};

const Magic BISHOP_MAGICS_INIT[SQUARE_NB] = {
    {0x0ULL, 0x400408444084080ULL, 58}, {0x1000000000000ULL, 0x200204222042040ULL, 59},
    {0x2800000000000ULL, 0x100102111021020ULL, 61}, {0x5000000000000ULL, 0x400088108800ULL, 62},
    {0xa000000000000ULL, 0x20004104100ULL, 62}, {0x14000000000000ULL, 0x40002082080ULL, 61},
    {0x28000000000000ULL, 0x8000404040202ULL, 59}, {0x0ULL, 0x80810202020100ULL, 58},
    {0x400000000000000ULL, 0x400408444084080ULL, 58}, {0x8008000000000000ULL, 0x200204222042040ULL, 59},
    {0x1001000000000000ULL, 0x100102111021020ULL, 61}, {0x2002000000000000ULL, 0x400088108800ULL, 62},
    {0x4004000000000000ULL, 0x20004104100ULL, 62}, {0x8008000000000000ULL, 0x40002082080ULL, 61},
    {0x1001000000000000ULL, 0x8000404040202ULL, 59}, {0x400000000000000ULL, 0x80810202020100ULL, 58},
    {0x2000000000000ULL, 0x400408444084080ULL, 58}, {0x100040000000000ULL, 0x200204222042040ULL, 59},
    {0x200080080000000ULL, 0x100102111021020ULL, 61}, {0x400100040000000ULL, 0x400088108800ULL, 62},
    {0x800200020000000ULL, 0x20004104100ULL, 62}, {0x1000400040000000ULL, 0x40002082080ULL, 61},
    {0x200080080000000ULL, 0x8000404040202ULL, 59}, {0x2000000000000ULL, 0x80810202020100ULL, 58},
    {0x100000000000ULL, 0x400408444084080ULL, 58}, {0x8200000000000ULL, 0x200204222042040ULL, 59},
    {0x44100000000000ULL, 0x100102111021020ULL, 61}, {0x88200000000000ULL, 0x400088108800ULL, 62},
    {0x110400000000000ULL, 0x20004104100ULL, 62}, {0x220800000000000ULL, 0x40002082080ULL, 61},
    {0x44100000000000ULL, 0x8000404040202ULL, 59}, {0x100000000000ULL, 0x80810202020100ULL, 58},
    {0x80000000000ULL, 0x400408444084080ULL, 58}, {0x410000000000ULL, 0x200204222042040ULL, 59},
    {0x2208000000000ULL, 0x100102111021020ULL, 61}, {0x1104000000000ULL, 0x400088108800ULL, 62},
    {0x882000000000ULL, 0x20004104100ULL, 62}, {0x4410000000000ULL, 0x40002082080ULL, 61},
    {0x2208000000000ULL, 0x8000404040202ULL, 59}, {0x80000000000ULL, 0x80810202020100ULL, 58},
    {0x4000000000ULL, 0x400408444084080ULL, 58}, {0x20800000000ULL, 0x200204222042040ULL, 59},
    {0x104400000000ULL, 0x100102111021020ULL, 61}, {0x82200000000ULL, 0x400088108800ULL, 62},
    {0x41100000000ULL, 0x20004104100ULL, 62}, {0x20880000000ULL, 0x40002082080ULL, 61},
    {0x104400000000ULL, 0x8000404040202ULL, 59}, {0x4000000000ULL, 0x80810202020100ULL, 58},
    {0x200000000ULL, 0x400408444084080ULL, 58}, {0x1040000000ULL, 0x200204222042040ULL, 59},
    {0x8220000000ULL, 0x100102111021020ULL, 61}, {0x4110000000ULL, 0x400088108800ULL, 62},
    {0x2088000000ULL, 0x20004104100ULL, 62}, {0x1044000000ULL, 0x40002082080ULL, 61},
    {0x8220000000ULL, 0x8000404040202ULL, 59}, {0x200000000ULL, 0x80810202020100ULL, 58},
    {0x102020404080ULL, 0x400408444084080ULL, 58}, {0x2040408081000ULL, 0x200204222042040ULL, 59},
    {0x4080810202000ULL, 0x100102111021020ULL, 61}, {0x810202040400ULL, 0x400088108800ULL, 62},
    {0x102040408080ULL, 0x20004104100ULL, 62}, {0x204080810200ULL, 0x40002082080ULL, 61},
    {0x408102020400ULL, 0x8000404040202ULL, 59}, {0x81020404080ULL, 0x80810202020100ULL, 58}
};


// Helper function to generate slider attacks "on the fly". This is used
// only once during initialization to populate the magic lookup tables.
uint64_t generate_attacks_on_the_fly(Square s, uint64_t blockers, int deltas[], int num_deltas) {
    uint64_t attacks = Bitboard::Empty;
    for (int i = 0; i < num_deltas; ++i) {
        Square current_sq = s;
        while (true) {
            current_sq = Square(current_sq + deltas[i]);
            // Check for wrap-around or off-board
            if (current_sq < A1 || current_sq > H8 || square_distance(current_sq, Square(current_sq-deltas[i])) > 2) {
                break;
            }
            set_bit(attacks, current_sq);
            // Stop if we hit a blocking piece
            if (get_bit(blockers, current_sq)) {
                break;
            }
        }
    }
    return attacks;
}

// Generates rook attacks and masks for a given square
void init_rook_magics(Square s) {
    RookMagics[s] = ROOK_MAGICS_INIT[s];
    uint64_t edges = ((Bitboard::Rank1 | Bitboard::Rank8) & ~Bitboard::Rank[s]) | ((Bitboard::FileA | Bitboard::FileH) & ~Bitboard::File[s]);
    RookMagics[s].mask &= ~edges;

    uint64_t b = Bitboard::Empty;
    int num_blockers = count_bits(RookMagics[s].mask);
    for (int i = 0; i < (1 << num_blockers); ++i) {
        int deltas[] = { -8, -1, 1, 8 };
        uint64_t attacks = generate_attacks_on_the_fly(s, b, deltas, 4);
        size_t magic_index = (b * RookMagics[s].magic) >> RookMagics[s].shift;
        RookAttacks[s][magic_index] = attacks;
        b = (b - RookMagics[s].mask) & RookMagics[s].mask; // Carry-rippler trick
    }
}

// Generates bishop attacks and masks for a given square
void init_bishop_magics(Square s) {
    BishopMagics[s] = BISHOP_MAGICS_INIT[s];
    uint64_t edges = Bitboard::Rank1 | Bitboard::Rank8 | Bitboard::FileA | Bitboard::FileH;
    BishopMagics[s].mask &= ~edges;

    uint64_t b = Bitboard::Empty;
    int num_blockers = count_bits(BishopMagics[s].mask);
    for (int i = 0; i < (1 << num_blockers); ++i) {
        int deltas[] = { -9, -7, 7, 9 };
        uint64_t attacks = generate_attacks_on_the_fly(s, b, deltas, 4);
        size_t magic_index = (b * BishopMagics[s].magic) >> BishopMagics[s].shift;
        BishopAttacks[s][magic_index] = attacks;
        b = (b - BishopMagics[s].mask) & BishopMagics[s].mask; // Carry-rippler trick
    }
}

// Initializes all slider piece attacks (rooks and bishops)
void init_magics() {
    for (Square s = A1; s <= H8; s = Square(s + 1)) {
        init_rook_magics(s);
        init_bishop_magics(s);
    }
}

// Initializes pawn, knight, and king attack tables
void init_leaper_attacks() {
    for (Square s = A1; s <= H8; s = Square(s + 1)) {
        // Pawns
        uint64_t white_pawn_attacks = Bitboard::Empty;
        uint64_t black_pawn_attacks = Bitboard::Empty;
        if (!get_bit(Bitboard::FileA, s)) { // Not on File A
            if (s <= H7) set_bit(white_pawn_attacks, Square(s + 7));
            if (s >= A2) set_bit(black_pawn_attacks, Square(s - 9));
        }
        if (!get_bit(Bitboard::FileH, s)) { // Not on File H
            if (s <= H7) set_bit(white_pawn_attacks, Square(s + 9));
            if (s >= A2) set_bit(black_pawn_attacks, Square(s - 7));
        }
        PawnAttacks[WHITE][s] = white_pawn_attacks;
        PawnAttacks[BLACK][s] = black_pawn_attacks;

        // Knights and Kings
        int knight_deltas[] = { -17, -15, -10, -6, 6, 10, 15, 17 };
        int king_deltas[] = { -9, -8, -7, -1, 1, 7, 8, 9 };

        for (int delta : knight_deltas) {
            Square target = Square(s + delta);
            if (target >= A1 && target <= H8 && square_distance(s, target) <= 2) {
                set_bit(KnightAttacks[s], target);
            }
        }
        for (int delta : king_deltas) {
            Square target = Square(s + delta);
            if (target >= A1 && target <= H8 && square_distance(s, target) <= 1) {
                set_bit(KingAttacks[s], target);
            }
        }
    }
}

} // anonymous namespace

//-----------------------------------------------------------------------------
// MAIN INITIALIZATION FUNCTION
//-----------------------------------------------------------------------------
void init() {
    init_leaper_attacks();
    init_magics();
}

} // namespace chess