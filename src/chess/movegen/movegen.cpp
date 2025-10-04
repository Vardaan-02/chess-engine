#include "movegen.h"

uint64_t all_king_moves[NUM_SQUARES];
uint64_t all_knight_moves[NUM_SQUARES];

// Generate king moves from square
uint64_t all_king_moves_from(int sq) {
    uint64_t currPos = (ONE << sq);

    uint64_t up         = currPos << 8;
    uint64_t down       = currPos >> 8;
    uint64_t left       = (currPos >> 1) & ~FILE_H;
    uint64_t right      = (currPos << 1) & ~FILE_A;
    uint64_t up_left    = (currPos << 7) & ~FILE_H;
    uint64_t up_right   = (currPos << 9) & ~FILE_A;
    uint64_t down_left  = (currPos >> 9) & ~FILE_H;
    uint64_t down_right = (currPos >> 7) & ~FILE_A;

    return up | down | left | right | up_left | up_right | down_left | down_right;
}

// Generate knight moves from square
uint64_t all_knight_moves_from(int sq) {
    uint64_t currPos = (ONE << sq);

    uint64_t right1 = (currPos << 1) & (~FILE_A);
    uint64_t right2 = (currPos << 2) & (~(FILE_A | FILE_B));

    uint64_t left1 = (currPos >> 1) & (~FILE_H);
    uint64_t left2 = (currPos >> 2) & (~(FILE_H | FILE_G));

    return ((left1 >> 16) | (left1 << 16) | (right1 >> 16) | (right1 << 16) |
            (left2 >> 8)  | (left2 << 8)  | (right2 >> 8)  | (right2 << 8));
}

// Initialize leaper lookup tables
void init_leapers() {
    for(int sq_no = 0 ; sq_no < NUM_SQUARES ; ++sq_no) {
        all_king_moves[sq_no]   = all_king_moves_from(sq_no);
        all_knight_moves[sq_no] = all_knight_moves_from(sq_no);
    }
}
