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
const Magic ROOK_MAGICS_INIT[64] = {
    {0x01010101010101FEULL, 0x8a80104000800020ULL, 52},
    {0x02020202020202FDULL, 0x4100200040080400ULL, 53},
    {0x04040404040404FBULL, 0x2040010000200008ULL, 53},
    {0x08080808080808F7ULL, 0x20800040000ULL, 53},
    {0x10101010101010EFULL, 0x4804000800100ULL, 53},
    {0x20202020202020DFULL, 0x12000200010040ULL, 53},
    {0x40404040404040BFULL, 0x401004000800ULL, 53},
    {0x808080808080807FULL, 0x408000802000100ULL, 52},

    {0x010101010101FE01ULL, 0x40200010080ULL, 53},
    {0x020202020202FD02ULL, 0x20080080401000ULL, 54},
    {0x040404040404FB04ULL, 0x4002000040080ULL, 54},
    {0x080808080808F708ULL, 0x10000100004000ULL, 54},
    {0x101010101010EF10ULL, 0x2000002000010ULL, 54},
    {0x202020202020DF20ULL, 0x4000001000020ULL, 54},
    {0x404040404040BF40ULL, 0x8000004000080ULL, 54},
    {0x8080808080807F80ULL, 0x800001004000ULL, 53},

    {0x0101010101FE0101ULL, 0x8000402000100ULL, 53},
    {0x0202020202FD0202ULL, 0x10002000080040ULL, 54},
    {0x0404040404FB0404ULL, 0x2000100004008ULL, 54},
    {0x0808080808F70808ULL, 0x100008000200ULL, 54},
    {0x1010101010EF1010ULL, 0x40001002000ULL, 54},
    {0x2020202020DF2020ULL, 0x80004000100ULL, 54},
    {0x4040404040BF4040ULL, 0x20000800040ULL, 54},
    {0x80808080807F8080ULL, 0x10000100200ULL, 53},

    {0x01010101FE010101ULL, 0x8000804000200ULL, 53},
    {0x02020202FD020202ULL, 0x20004000100080ULL, 54},
    {0x04040404FB040404ULL, 0x4000800020001ULL, 54},
    {0x08080808F7080808ULL, 0x800040001000ULL, 54},
    {0x10101010EF101010ULL, 0x10000800020ULL, 54},
    {0x20202020DF202020ULL, 0x40000100008ULL, 54},
    {0x40404040BF404040ULL, 0x80000200010ULL, 54},
    {0x808080807F808080ULL, 0x80000040010ULL, 53},

    {0x010101FE01010101ULL, 0x2000800080400ULL, 53},
    {0x020202FD02020202ULL, 0x10004000200080ULL, 54},
    {0x040404FB04040404ULL, 0x4000200010001ULL, 54},
    {0x080808F708080808ULL, 0x80001000040ULL, 54},
    {0x101010EF10101010ULL, 0x10000400008ULL, 54},
    {0x202020DF20202020ULL, 0x20000800010ULL, 54},
    {0x404040BF40404040ULL, 0x400001000020ULL, 54},
    {0x8080807F80808080ULL, 0x80000080010ULL, 53},

    {0x0101FE0101010101ULL, 0x4000802000400ULL, 53},
    {0x0202FD0202020202ULL, 0x80002000040010ULL, 54},
    {0x0404FB0404040404ULL, 0x10001000020004ULL, 54},
    {0x0808F70808080808ULL, 0x400008000100ULL, 54},
    {0x1010EF1010101010ULL, 0x80001000020ULL, 54},
    {0x2020DF2020202020ULL, 0x10000400008ULL, 54},
    {0x4040BF4040404040ULL, 0x20000800010ULL, 54},
    {0x80807F8080808080ULL, 0x80000040010ULL, 53},

    {0x01FE010101010101ULL, 0x1000804000200ULL, 52},
    {0x02FD020202020202ULL, 0x20001000080040ULL, 53},
    {0x04FB040404040404ULL, 0x40000800020001ULL, 53},
    {0x08F7080808080808ULL, 0x100004000080ULL, 53},
    {0x10EF101010101010ULL, 0x200008000100ULL, 53},
    {0x20DF202020202020ULL, 0x400001000020ULL, 53},
    {0x40BF404040404040ULL, 0x800002000010ULL, 53},
    {0x807F808080808080ULL, 0x40000080010ULL, 52},

    {0xFE01010101010101ULL, 0x1000804000200ULL, 52},
    {0xFD02020202020202ULL, 0x20001000080040ULL, 53},
    {0xFB04040404040404ULL, 0x40000800020001ULL, 53},
    {0xF708080808080808ULL, 0x100004000080ULL, 53},
    {0xEF10101010101010ULL, 0x200008000100ULL, 53},
    {0xDF20202020202020ULL, 0x400001000020ULL, 53},
    {0xBF40404040404040ULL, 0x800002000010ULL, 53},
    {0x7F80808080808080ULL, 0x40000080010ULL, 52},
};

const Magic BISHOP_MAGICS_INIT[64] = {
    {0x40201008040200ULL, 0x2101002004448ULL, 58}, // 0 (bits=6 tries=1330)
    {0x402010080500ULL, 0x30202004401806eULL, 58}, // 1 (bits=6 tries=248)
    {0x4020110a00ULL, 0x108008020840141ULL, 58}, // 2 (bits=6 tries=403)
    {0x41221400ULL, 0xa4240090030001ULL, 58}, // 3 (bits=6 tries=200)
    {0x102442800ULL, 0x1004602000000030ULL, 58}, // 4 (bits=6 tries=207)
    {0x10204085000ULL, 0x404220802004090ULL, 58}, // 5 (bits=6 tries=138)
    {0x1020408102000ULL, 0x41c420201a00080ULL, 58}, // 6 (bits=6 tries=89)
    {0x2040810204000ULL, 0x420e01c2021000ULL, 58}, // 7 (bits=6 tries=11897)
    {0x20100804020002ULL, 0x4226840108010dULL, 58}, // 8 (bits=6 tries=68)
    {0x40201008050005ULL, 0x20801000a0802200ULL, 56}, // 9 (bits=8 tries=8891)
    {0x4020110a000aULL, 0x88140104010001ULL, 56}, // 10 (bits=8 tries=165)
    {0x4122140014ULL, 0x420822800080ULL, 56}, // 11 (bits=8 tries=216)
    {0x10244280028ULL, 0x10041004020ULL, 56}, // 12 (bits=8 tries=135)
    {0x1020408500050ULL, 0x1108080400000ULL, 56}, // 13 (bits=8 tries=352)
    {0x2040810200020ULL, 0x100b0100821800ULL, 58}, // 14 (bits=6 tries=926)
    {0x4081020400040ULL, 0x6c08a50021010800ULL, 58}, // 15 (bits=6 tries=943)
    {0x10080402000204ULL, 0x48012002208120ULL, 58}, // 16 (bits=6 tries=323)
    {0x20100805000508ULL, 0x42502008101000aULL, 56}, // 17 (bits=8 tries=519)
    {0x4020110a000a11ULL, 0x6348000080210020ULL, 54}, // 18 (bits=10 tries=47165)
    {0x412214001422ULL, 0x104c04000600ULL, 54}, // 19 (bits=10 tries=593)
    {0x1024428002844ULL, 0x101088040002ULL, 54}, // 20 (bits=10 tries=557)
    {0x2040850005008ULL, 0xa000060840080ULL, 56}, // 21 (bits=8 tries=1346)
    {0x4081020002010ULL, 0x5001800c1402a800ULL, 58}, // 22 (bits=6 tries=1011)
    {0x8102040004020ULL, 0x4280701091002ULL, 58}, // 23 (bits=6 tries=551)
    {0x8040200020408ULL, 0x40080c00c1210800ULL, 58}, // 24 (bits=6 tries=238)
    {0x10080500050810ULL, 0x424800440300ULL, 56}, // 25 (bits=8 tries=2030)
    {0x20110a000a1120ULL, 0x3200060084001404ULL, 54}, // 26 (bits=10 tries=187)
    {0x41221400142241ULL, 0x80010020040100ULL, 52}, // 27 (bits=12 tries=161699)
    {0x2442800284402ULL, 0x4702000100400ULL, 54}, // 28 (bits=10 tries=17127)
    {0x4085000500804ULL, 0x2102000109001ULL, 56}, // 29 (bits=8 tries=1319)
    {0x8102000201008ULL, 0x20021024040402c2ULL, 58}, // 30 (bits=6 tries=615)
    {0x10204000402010ULL, 0x2011104014140420ULL, 58}, // 31 (bits=6 tries=405)
    {0x4020002040810ULL, 0x1011104000280104ULL, 58}, // 32 (bits=6 tries=481)
    {0x8050005081020ULL, 0x802180a001100140ULL, 56}, // 33 (bits=8 tries=576)
    {0x110a000a112040ULL, 0x200020240048010ULL, 54}, // 34 (bits=10 tries=1276)
    {0x22140014224100ULL, 0x10040040400ULL, 54}, // 35 (bits=10 tries=6675)
    {0x44280028440201ULL, 0x8000004a0000c010ULL, 54}, // 36 (bits=10 tries=2716)
    {0x8500050080402ULL, 0x2002200230280ULL, 56}, // 37 (bits=8 tries=455)
    {0x10200020100804ULL, 0x8144891040084ULL, 58}, // 38 (bits=6 tries=436)
    {0x20400040201008ULL, 0x40488140008042ULL, 58}, // 39 (bits=6 tries=216)
    {0x2000204081020ULL, 0x8000481c04080900ULL, 58}, // 40 (bits=6 tries=485)
    {0x5000508102040ULL, 0x180082004428ULL, 56}, // 41 (bits=8 tries=43)
    {0xa000a11204000ULL, 0x1808140090200802ULL, 56}, // 42 (bits=8 tries=487)
    {0x14001422410000ULL, 0x20000483040180ULL, 56}, // 43 (bits=8 tries=2697)
    {0x28002844020100ULL, 0x1000204044012040ULL, 56}, // 44 (bits=8 tries=378)
    {0x50005008040201ULL, 0x4000428122010050ULL, 56}, // 45 (bits=8 tries=1816)
    {0x20002010080402ULL, 0x1040132022436ULL, 58}, // 46 (bits=6 tries=47)
    {0x40004020100804ULL, 0x22020404101020ULL, 58}, // 47 (bits=6 tries=523)
    {0x20408102040ULL, 0x2004404104249001ULL, 58}, // 48 (bits=6 tries=222)
    {0x50810204000ULL, 0x8002404402012120ULL, 58}, // 49 (bits=6 tries=15)
    {0xa1120400000ULL, 0x100010100421028ULL, 58}, // 50 (bits=6 tries=725)
    {0x142241000000ULL, 0x200200022080040ULL, 58}, // 51 (bits=6 tries=97)
    {0x284402010000ULL, 0x1408024010208308ULL, 58}, // 52 (bits=6 tries=21)
    {0x500804020100ULL, 0x22004010004ULL, 58}, // 53 (bits=6 tries=177)
    {0x201008040201ULL, 0x480049005104900ULL, 58}, // 54 (bits=6 tries=169)
    {0x402010080402ULL, 0x4004340062026002ULL, 58}, // 55 (bits=6 tries=22)
    {0x2040810204000ULL, 0x3154140103086000ULL, 58}, // 56 (bits=6 tries=20464)
    {0x5081020400000ULL, 0x800020208840c02ULL, 58}, // 57 (bits=6 tries=398)
    {0xa112040000000ULL, 0x8000404884028806ULL, 58}, // 58 (bits=6 tries=91)
    {0x14224100000000ULL, 0x200128000430200ULL, 58}, // 59 (bits=6 tries=158)
    {0x28440201000000ULL, 0x1020800600c1400ULL, 58}, // 60 (bits=6 tries=3)
    {0x50080402010000ULL, 0x4d0006012008206ULL, 58}, // 61 (bits=6 tries=123)
    {0x20100804020100ULL, 0x181905020480ULL, 58}, // 62 (bits=6 tries=1437)
    {0x40201008040201ULL, 0x80000102026c1820ULL, 57}, // 63 (bits=7 tries=1282)
};


// Helper function to generate slider attacks "on the fly". This is used
// only once during initialization to populate the magic lookup tables.
uint64_t generate_attacks_on_the_fly(Square s, uint64_t blockers, int deltas[], int num_deltas) {
    uint64_t attacks = util::Empty;
    for (int i = 0; i < num_deltas; ++i) {
        Square current_sq = s;
        while (true) {
            current_sq = Square(current_sq + deltas[i]);
            // Check for wrap-around or off-board
            if (current_sq < A1 || current_sq > H8 || square_distance(current_sq, Square(current_sq-deltas[i])) > 2) {
                break;
            }
            util::set_bit(attacks, current_sq);
            // Stop if we hit a blocking piece
            if (util::get_bit(blockers, current_sq)) {
                break;
            }
        }
    }
    return attacks;
}

// Generates rook attacks and masks for a given square
void init_rook_magics(Square s) {
    RookMagics[s] = ROOK_MAGICS_INIT[s];
    uint64_t edges = ((util::Rank1 | util::Rank8) & ~util::Rank[s]) | ((util::FileA | util::FileH) & ~util::File[s]);
    RookMagics[s].mask &= (~edges);

    uint64_t b = util::Empty;
    int num_blockers = util::count_bits(RookMagics[s].mask);
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
    uint64_t edges = util::Rank1 | util::Rank8 | util::FileA | util::FileH;
    BishopMagics[s].mask &= ~edges;
    
    uint64_t b = util::Empty;
    int num_blockers = util::count_bits(BishopMagics[s].mask);
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
        uint64_t white_pawn_attacks = util::Empty;
        uint64_t black_pawn_attacks = util::Empty;
        if (!util::get_bit(util::FileA, s)) { // Not on File A
            if (s <= H7) util::set_bit(white_pawn_attacks, Square(s + 7));
            if (s >= A2) util::set_bit(black_pawn_attacks, Square(s - 9));
        }
        if (!util::get_bit(util::FileH, s)) { // Not on File H
            if (s <= H7) util::set_bit(white_pawn_attacks, Square(s + 9));
            if (s >= A2) util::set_bit(black_pawn_attacks, Square(s - 7));
        }
        PawnAttacks[WHITE][s] = white_pawn_attacks;
        PawnAttacks[BLACK][s] = black_pawn_attacks;

        // Knights and Kings
        int knight_deltas[] = { -17, -15, -10, -6, 6, 10, 15, 17 };
        int king_deltas[] = { -9, -8, -7, -1, 1, 7, 8, 9 };

        for (int delta : knight_deltas) {
            Square target = Square(s + delta);
            if (target >= A1 && target <= H8 && square_distance(s, target) <= 2) {
                util::set_bit(KnightAttacks[s], target);
            }
        }
        for (int delta : king_deltas) {
            Square target = Square(s + delta);
            if (target >= A1 && target <= H8 && square_distance(s, target) <= 1) {
                util::set_bit(KingAttacks[s], target);
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