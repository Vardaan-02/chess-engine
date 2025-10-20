#include "chess/bitboard.h"
#include "chess/types.h"
#include "chess/zobrist.h"
#include <iomanip>

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


uint64_t files[] = {util::FileA, util::FileB, util::FileC, util::FileD, util::FileE, util::FileF, util::FileG, util::FileH};
uint64_t ranks[] = {util::Rank1, util::Rank2, util::Rank3, util::Rank4, util::Rank5, util::Rank6, util::Rank7, util::Rank8};

//-----------------------------------------------------------------------------
// ANONYMOUS NAMESPACE FOR HELPER FUNCTIONS
//
// These functions are only used within this file to perform the one-time
// initialization of the attack tables.
//-----------------------------------------------------------------------------
namespace {

const uint64_t magicmoves_r_magics[SQUARE_NB] = {
    0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
    0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
    0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
    0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
    0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010002000ULL, 0x0000808008001000ULL,
    0x0000808004000800ULL, 0x0000808002000400ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
    0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
    0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
    0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
    0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
    0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
    0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
    0x00FFFCDDFCED714AULL, 0x007FFCDDFCED714AULL, 0x003FFFCDFFD88096ULL, 0x0000040810002101ULL,
    0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001FFFAABFAD1A2ULL
};

const uint64_t magicmoves_b_magics[SQUARE_NB] = {
    0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
    0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
    0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
    0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
    0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
    0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
    0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
    0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
    0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
    0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
    0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
    0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
    0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
    0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
    0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
    0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL
};

const uint8_t magicmoves_r_shifts[SQUARE_NB] = 
{
	52, 53, 53, 53, 53, 53, 53, 52,
	53, 54, 54, 54, 54, 54, 54, 53,
	53, 54, 54, 54, 54, 54, 54, 53,
	53, 54, 54, 54, 54, 54, 54, 53,
	53, 54, 54, 54, 54, 54, 54, 53,
	53, 54, 54, 54, 54, 54, 54, 53,
	53, 54, 54, 54, 54, 54, 54, 53,
	53, 54, 54, 53, 53, 53, 53, 53
};

const uint8_t magicmoves_b_shifts[SQUARE_NB] = 
{
	58, 59, 59, 59, 59, 59, 59, 58,
	59, 59, 59, 59, 59, 59, 59, 59,
	59, 59, 57, 57, 57, 57, 59, 59,
	59, 59, 57, 55, 55, 57, 59, 59,
	59, 59, 57, 55, 55, 57, 59, 59,
	59, 59, 57, 57, 57, 57, 59, 59,
	59, 59, 59, 59, 59, 59, 59, 59,
	58, 59, 59, 59, 59, 59, 59, 58
};


const uint64_t rook_masks[SQUARE_NB] = {
    0x01010101010101FEULL, 0x02020202020202FDULL, 0x04040404040404FBULL, 0x08080808080808F7ULL,
    0x10101010101010EFULL, 0x20202020202020DFULL, 0x40404040404040BFULL, 0x808080808080807FULL,
    0x010101010101FE01ULL, 0x020202020202FD02ULL, 0x040404040404FB04ULL, 0x080808080808F708ULL,
    0x101010101010EF10ULL, 0x202020202020DF20ULL, 0x404040404040BF40ULL, 0x8080808080807F80ULL,
    0x0101010101FE0101ULL, 0x0202020202FD0202ULL, 0x0404040404FB0404ULL, 0x0808080808F70808ULL,
    0x1010101010EF1010ULL, 0x2020202020DF2020ULL, 0x4040404040BF4040ULL, 0x80808080807F8080ULL,
    0x01010101FE010101ULL, 0x02020202FD020202ULL, 0x04040404FB040404ULL, 0x08080808F7080808ULL,
    0x10101010EF101010ULL, 0x20202020DF202020ULL, 0x40404040BF404040ULL, 0x808080807F808080ULL,
    0x010101FE01010101ULL, 0x020202FD02020202ULL, 0x040404FB04040404ULL, 0x080808F708080808ULL,
    0x101010EF10101010ULL, 0x202020DF20202020ULL, 0x404040BF40404040ULL, 0x8080807F80808080ULL,
    0x0101FE0101010101ULL, 0x0202FD0202020202ULL, 0x0404FB0404040404ULL, 0x0808F70808080808ULL,
    0x1010EF1010101010ULL, 0x2020DF2020202020ULL, 0x4040BF4040404040ULL, 0x80807F8080808080ULL,
    0x01FE010101010101ULL, 0x02FD020202020202ULL, 0x04FB040404040404ULL, 0x08F7080808080808ULL,
    0x10EF101010101010ULL, 0x20DF202020202020ULL, 0x40BF404040404040ULL, 0x807F808080808080ULL,
    0xFE01010101010101ULL, 0xFD02020202020202ULL, 0xFB04040404040404ULL, 0xF708080808080808ULL,
    0xEF10101010101010ULL, 0xDF20202020202020ULL, 0xBF40404040404040ULL, 0x7F80808080808080ULL
};

const uint64_t bishop_masks[SQUARE_NB] = {
    0x40201008040200ULL,   0x402010080500ULL,     0x4020110a00ULL,       0x41221400ULL,
    0x102442800ULL,        0x10204085000ULL,      0x1020408102000ULL,    0x2040810204000ULL,
    0x20100804020002ULL,   0x40201008050005ULL,   0x4020110a000aULL,     0x4122140014ULL,
    0x10244280028ULL,      0x1020408500050ULL,    0x2040810200020ULL,    0x4081020400040ULL,
    0x10080402000204ULL,   0x20100805000508ULL,   0x4020110a000a11ULL,   0x412214001422ULL,
    0x1024428002844ULL,    0x2040850005008ULL,    0x4081020002010ULL,    0x8102040004020ULL,
    0x8040200020408ULL,    0x10080500050810ULL,   0x20110a000a1120ULL,   0x41221400142241ULL,
    0x2442800284402ULL,    0x4085000500804ULL,    0x8102000201008ULL,    0x10204000402010ULL,
    0x4020002040810ULL,    0x8050005081020ULL,    0x110a000a112040ULL,   0x22140014224100ULL,
    0x44280028440201ULL,   0x8500050080402ULL,    0x10200020100804ULL,   0x20400040201008ULL,
    0x2000204081020ULL,    0x5000508102040ULL,    0xa000a11204000ULL,    0x14001422410000ULL,
    0x28002844020100ULL,   0x50005008040201ULL,   0x20002010080402ULL,   0x40004020100804ULL,
    0x20408102040ULL,      0x50810204000ULL,      0xa1120400000ULL,      0x142241000000ULL,

    0x284402010000ULL,     0x500804020100ULL,     0x201008040201ULL,     0x402010080402ULL,
    0x2040810204000ULL,    0x5081020400000ULL,    0xa112040000000ULL,    0x14224100000000ULL,
    0x28440201000000ULL,   0x50080402010000ULL,   0x20100804020100ULL,   0x40201008040201ULL
};



// Pre-computed magic numbers for rooks and bishops. These are standard values.
// Initialize the ROOK_MAGIC_INIT array of structs
const Magic ROOK_MAGICS_INIT[SQUARE_NB] = {
    {rook_masks[ 0], magicmoves_r_magics[ 0], magicmoves_r_shifts[ 0]},
    {rook_masks[ 1], magicmoves_r_magics[ 1], magicmoves_r_shifts[ 1]},
    {rook_masks[ 2], magicmoves_r_magics[ 2], magicmoves_r_shifts[ 2]},
    {rook_masks[ 3], magicmoves_r_magics[ 3], magicmoves_r_shifts[ 3]},
    {rook_masks[ 4], magicmoves_r_magics[ 4], magicmoves_r_shifts[ 4]},
    {rook_masks[ 5], magicmoves_r_magics[ 5], magicmoves_r_shifts[ 5]},
    {rook_masks[ 6], magicmoves_r_magics[ 6], magicmoves_r_shifts[ 6]},
    {rook_masks[ 7], magicmoves_r_magics[ 7], magicmoves_r_shifts[ 7]},
    {rook_masks[ 8], magicmoves_r_magics[ 8], magicmoves_r_shifts[ 8]},
    {rook_masks[ 9], magicmoves_r_magics[ 9], magicmoves_r_shifts[ 9]},
    {rook_masks[10], magicmoves_r_magics[10], magicmoves_r_shifts[10]},
    {rook_masks[11], magicmoves_r_magics[11], magicmoves_r_shifts[11]},
    {rook_masks[12], magicmoves_r_magics[12], magicmoves_r_shifts[12]},
    {rook_masks[13], magicmoves_r_magics[13], magicmoves_r_shifts[13]},
    {rook_masks[14], magicmoves_r_magics[14], magicmoves_r_shifts[14]},
    {rook_masks[15], magicmoves_r_magics[15], magicmoves_r_shifts[15]},
    {rook_masks[16], magicmoves_r_magics[16], magicmoves_r_shifts[16]},
    {rook_masks[17], magicmoves_r_magics[17], magicmoves_r_shifts[17]},
    {rook_masks[18], magicmoves_r_magics[18], magicmoves_r_shifts[18]},
    {rook_masks[19], magicmoves_r_magics[19], magicmoves_r_shifts[19]},
    {rook_masks[20], magicmoves_r_magics[20], magicmoves_r_shifts[20]},
    {rook_masks[21], magicmoves_r_magics[21], magicmoves_r_shifts[21]},
    {rook_masks[22], magicmoves_r_magics[22], magicmoves_r_shifts[22]},
    {rook_masks[23], magicmoves_r_magics[23], magicmoves_r_shifts[23]},
    {rook_masks[24], magicmoves_r_magics[24], magicmoves_r_shifts[24]},
    {rook_masks[25], magicmoves_r_magics[25], magicmoves_r_shifts[25]},
    {rook_masks[26], magicmoves_r_magics[26], magicmoves_r_shifts[26]},
    {rook_masks[27], magicmoves_r_magics[27], magicmoves_r_shifts[27]},
    {rook_masks[28], magicmoves_r_magics[28], magicmoves_r_shifts[28]},
    {rook_masks[29], magicmoves_r_magics[29], magicmoves_r_shifts[29]},
    {rook_masks[30], magicmoves_r_magics[30], magicmoves_r_shifts[30]},
    {rook_masks[31], magicmoves_r_magics[31], magicmoves_r_shifts[31]},
    {rook_masks[32], magicmoves_r_magics[32], magicmoves_r_shifts[32]},
    {rook_masks[33], magicmoves_r_magics[33], magicmoves_r_shifts[33]},
    {rook_masks[34], magicmoves_r_magics[34], magicmoves_r_shifts[34]},
    {rook_masks[35], magicmoves_r_magics[35], magicmoves_r_shifts[35]},
    {rook_masks[36], magicmoves_r_magics[36], magicmoves_r_shifts[36]},
    {rook_masks[37], magicmoves_r_magics[37], magicmoves_r_shifts[37]},
    {rook_masks[38], magicmoves_r_magics[38], magicmoves_r_shifts[38]},
    {rook_masks[39], magicmoves_r_magics[39], magicmoves_r_shifts[39]},
    {rook_masks[40], magicmoves_r_magics[40], magicmoves_r_shifts[40]},
    {rook_masks[41], magicmoves_r_magics[41], magicmoves_r_shifts[41]},
    {rook_masks[42], magicmoves_r_magics[42], magicmoves_r_shifts[42]},
    {rook_masks[43], magicmoves_r_magics[43], magicmoves_r_shifts[43]},
    {rook_masks[44], magicmoves_r_magics[44], magicmoves_r_shifts[44]},
    {rook_masks[45], magicmoves_r_magics[45], magicmoves_r_shifts[45]},
    {rook_masks[46], magicmoves_r_magics[46], magicmoves_r_shifts[46]},
    {rook_masks[47], magicmoves_r_magics[47], magicmoves_r_shifts[47]},
    {rook_masks[48], magicmoves_r_magics[48], magicmoves_r_shifts[48]},
    {rook_masks[49], magicmoves_r_magics[49], magicmoves_r_shifts[49]},
    {rook_masks[50], magicmoves_r_magics[50], magicmoves_r_shifts[50]},
    {rook_masks[51], magicmoves_r_magics[51], magicmoves_r_shifts[51]},
    {rook_masks[52], magicmoves_r_magics[52], magicmoves_r_shifts[52]},
    {rook_masks[53], magicmoves_r_magics[53], magicmoves_r_shifts[53]},
    {rook_masks[54], magicmoves_r_magics[54], magicmoves_r_shifts[54]},
    {rook_masks[55], magicmoves_r_magics[55], magicmoves_r_shifts[55]},
    {rook_masks[56], magicmoves_r_magics[56], magicmoves_r_shifts[56]},
    {rook_masks[57], magicmoves_r_magics[57], magicmoves_r_shifts[57]},
    {rook_masks[58], magicmoves_r_magics[58], magicmoves_r_shifts[58]},
    {rook_masks[59], magicmoves_r_magics[59], magicmoves_r_shifts[59]},
    {rook_masks[60], magicmoves_r_magics[60], magicmoves_r_shifts[60]},
    {rook_masks[61], magicmoves_r_magics[61], magicmoves_r_shifts[61]},
    {rook_masks[62], magicmoves_r_magics[62], magicmoves_r_shifts[62]},
    {rook_masks[63], magicmoves_r_magics[63], magicmoves_r_shifts[63]},
};

const Magic BISHOP_MAGICS_INIT[SQUARE_NB] = {
    {bishop_masks[ 0], magicmoves_b_magics[ 0], magicmoves_b_shifts[ 0]},
    {bishop_masks[ 1], magicmoves_b_magics[ 1], magicmoves_b_shifts[ 1]},
    {bishop_masks[ 2], magicmoves_b_magics[ 2], magicmoves_b_shifts[ 2]},
    {bishop_masks[ 3], magicmoves_b_magics[ 3], magicmoves_b_shifts[ 3]},
    {bishop_masks[ 4], magicmoves_b_magics[ 4], magicmoves_b_shifts[ 4]},
    {bishop_masks[ 5], magicmoves_b_magics[ 5], magicmoves_b_shifts[ 5]},
    {bishop_masks[ 6], magicmoves_b_magics[ 6], magicmoves_b_shifts[ 6]},
    {bishop_masks[ 7], magicmoves_b_magics[ 7], magicmoves_b_shifts[ 7]},
    {bishop_masks[ 8], magicmoves_b_magics[ 8], magicmoves_b_shifts[ 8]},
    {bishop_masks[ 9], magicmoves_b_magics[ 9], magicmoves_b_shifts[ 9]},
    {bishop_masks[10], magicmoves_b_magics[10], magicmoves_b_shifts[10]},
    {bishop_masks[11], magicmoves_b_magics[11], magicmoves_b_shifts[11]},
    {bishop_masks[12], magicmoves_b_magics[12], magicmoves_b_shifts[12]},
    {bishop_masks[13], magicmoves_b_magics[13], magicmoves_b_shifts[13]},
    {bishop_masks[14], magicmoves_b_magics[14], magicmoves_b_shifts[14]},
    {bishop_masks[15], magicmoves_b_magics[15], magicmoves_b_shifts[15]},
    {bishop_masks[16], magicmoves_b_magics[16], magicmoves_b_shifts[16]},
    {bishop_masks[17], magicmoves_b_magics[17], magicmoves_b_shifts[17]},
    {bishop_masks[18], magicmoves_b_magics[18], magicmoves_b_shifts[18]},
    {bishop_masks[19], magicmoves_b_magics[19], magicmoves_b_shifts[19]},
    {bishop_masks[20], magicmoves_b_magics[20], magicmoves_b_shifts[20]},
    {bishop_masks[21], magicmoves_b_magics[21], magicmoves_b_shifts[21]},
    {bishop_masks[22], magicmoves_b_magics[22], magicmoves_b_shifts[22]},
    {bishop_masks[23], magicmoves_b_magics[23], magicmoves_b_shifts[23]},
    {bishop_masks[24], magicmoves_b_magics[24], magicmoves_b_shifts[24]},
    {bishop_masks[25], magicmoves_b_magics[25], magicmoves_b_shifts[25]},
    {bishop_masks[26], magicmoves_b_magics[26], magicmoves_b_shifts[26]},
    {bishop_masks[27], magicmoves_b_magics[27], magicmoves_b_shifts[27]},
    {bishop_masks[28], magicmoves_b_magics[28], magicmoves_b_shifts[28]},
    {bishop_masks[29], magicmoves_b_magics[29], magicmoves_b_shifts[29]},
    {bishop_masks[30], magicmoves_b_magics[30], magicmoves_b_shifts[30]},
    {bishop_masks[31], magicmoves_b_magics[31], magicmoves_b_shifts[31]},
    {bishop_masks[32], magicmoves_b_magics[32], magicmoves_b_shifts[32]},
    {bishop_masks[33], magicmoves_b_magics[33], magicmoves_b_shifts[33]},
    {bishop_masks[34], magicmoves_b_magics[34], magicmoves_b_shifts[34]},
    {bishop_masks[35], magicmoves_b_magics[35], magicmoves_b_shifts[35]},
    {bishop_masks[36], magicmoves_b_magics[36], magicmoves_b_shifts[36]},
    {bishop_masks[37], magicmoves_b_magics[37], magicmoves_b_shifts[37]},
    {bishop_masks[38], magicmoves_b_magics[38], magicmoves_b_shifts[38]},
    {bishop_masks[39], magicmoves_b_magics[39], magicmoves_b_shifts[39]},
    {bishop_masks[40], magicmoves_b_magics[40], magicmoves_b_shifts[40]},
    {bishop_masks[41], magicmoves_b_magics[41], magicmoves_b_shifts[41]},
    {bishop_masks[42], magicmoves_b_magics[42], magicmoves_b_shifts[42]},
    {bishop_masks[43], magicmoves_b_magics[43], magicmoves_b_shifts[43]},
    {bishop_masks[44], magicmoves_b_magics[44], magicmoves_b_shifts[44]},
    {bishop_masks[45], magicmoves_b_magics[45], magicmoves_b_shifts[45]},
    {bishop_masks[46], magicmoves_b_magics[46], magicmoves_b_shifts[46]},
    {bishop_masks[47], magicmoves_b_magics[47], magicmoves_b_shifts[47]},
    {bishop_masks[48], magicmoves_b_magics[48], magicmoves_b_shifts[48]},
    {bishop_masks[49], magicmoves_b_magics[49], magicmoves_b_shifts[49]},
    {bishop_masks[50], magicmoves_b_magics[50], magicmoves_b_shifts[50]},
    {bishop_masks[51], magicmoves_b_magics[51], magicmoves_b_shifts[51]},
    {bishop_masks[52], magicmoves_b_magics[52], magicmoves_b_shifts[52]},
    {bishop_masks[53], magicmoves_b_magics[53], magicmoves_b_shifts[53]},
    {bishop_masks[54], magicmoves_b_magics[54], magicmoves_b_shifts[54]},
    {bishop_masks[55], magicmoves_b_magics[55], magicmoves_b_shifts[55]},
    {bishop_masks[56], magicmoves_b_magics[56], magicmoves_b_shifts[56]},
    {bishop_masks[57], magicmoves_b_magics[57], magicmoves_b_shifts[57]},
    {bishop_masks[58], magicmoves_b_magics[58], magicmoves_b_shifts[58]},
    {bishop_masks[59], magicmoves_b_magics[59], magicmoves_b_shifts[59]},
    {bishop_masks[60], magicmoves_b_magics[60], magicmoves_b_shifts[60]},
    {bishop_masks[61], magicmoves_b_magics[61], magicmoves_b_shifts[61]},
    {bishop_masks[62], magicmoves_b_magics[62], magicmoves_b_shifts[62]},
    {bishop_masks[63], magicmoves_b_magics[63], magicmoves_b_shifts[63]},
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
    
    // std::cout << "Bishop Masks for Sq: " << (int)s << '\n';
    // chess::print_bitboard(BishopMagics[s].mask);

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

uint64_t Between[chess::SQUARE_NB][chess::SQUARE_NB];
uint64_t Rays[chess::SQUARE_NB][chess::SQUARE_NB];

void generate_between_and_ray_tables() noexcept {
    // Initialize all tables to 0ULL (empty bitboard)
    for (int s1 = 0; s1 < 64; ++s1) {
        for (int s2 = 0; s2 < 64; ++s2) {
            Between[s1][s2] = 0ULL;
            Rays[s1][s2] = 0ULL;
        }
    }

    for (int s1 = 0; s1 < 64; ++s1) {
        for (int s2 = 0; s2 < 64; ++s2) {
            if (s1 == s2) continue; // No squares between a square and itself

            // Get original coordinates without swapping
            int s1_rank = s1 / 8;
            int s1_file = s1 % 8;
            int s2_rank = s2 / 8;
            int s2_file = s2 % 8;

            // Calculate the raw difference to check for alignment
            int rank_diff = s2_rank - s1_rank;
            int file_diff = s2_file - s1_file;

            // Check if the squares are aligned on a rank, file, or diagonal
            // This is a much clearer way to check than the original logic
            if ((rank_diff != 0 && file_diff != 0) && (std::abs(rank_diff) != std::abs(file_diff))) {
                continue; // Not a straight line, so no ray/between squares
            }

            // FIX: Calculate the unit direction vector (the "step")
            // This will be -1, 0, or 1 for both rank and file
            int dr = (rank_diff > 0) ? 1 : (rank_diff < 0) ? -1 : 0;
            int df = (file_diff > 0) ? 1 : (file_diff < 0) ? -1 : 0;

            uint64_t between_mask = 0ULL;

            // FIX: Start "crawling" from s1 towards s2 one step at a time
            int current_rank = s1_rank + dr;
            int current_file = s1_file + df;

            while (current_rank != s2_rank || current_file != s2_file) {
                int current_square = current_rank * 8 + current_file;
                between_mask |= (1ULL << current_square);
                current_rank += dr;
                current_file += df;
            }

            // The squares BETWEEN s1 and s2
            Between[s1][s2] = between_mask;
            
            // The RAY includes the squares between AND the destination square s2
            Rays[s1][s2] = between_mask | (1ULL << s2);
        }
    }
}


//-----------------------------------------------------------------------------
// MAIN INITIALIZATION FUNCTION
//-----------------------------------------------------------------------------
void init() {
    init_leaper_attacks();
    init_magics();
    generate_between_and_ray_tables();
}
};
 // namespace chess