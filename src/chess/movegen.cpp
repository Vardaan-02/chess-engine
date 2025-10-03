#include "movegen.h"

#define ONE 1ULL

// All positions where king can move from sq (even illegal ones)
uint64_t all_king_moves_from(int sq)
{
    uint64_t currPos = (ONE << sq);

    uint64_t right = (currPos << 1) & ~(FILE_A);            //Prevents going out of the board
    uint64_t left = (currPos >> 1) & ~(FILE_H);

    return (right | left | (currPos >> 8) | (currPos << 8) | (left >> 8) | (left << 8) | (right >> 8) | (right << 8));
}

uint64_t all_knight_moves_from(int sq)
{
    uint64_t currPos = (ONE << sq);

    uint64_t right1 = (currPos << 1) & ~(FILE_A);           //Prevents going out of the board
    uint64_t right2 = (currPos << 2) & ~(FILE_A | FILE_B);

    uint64_t left1 = (currPos >> 1) & ~(FILE_H);
    uint64_t left2 = (currPos >> 2) & ~(FILE_H | FILE_G);

    return ((left1 >> 16) | (left1 << 16) | (right1 >> 16) | (right1 << 16) | (left2 >> 8) | (left2 << 8) | (right2 >> 8) | (right2 << 8));   
}

// all possible moves of knight and king added in array
void init_leapers()
{
    for(int sq_no = 0 ; sq_no < NUM_SQUARES ; ++sq_no)
    {
        all_king_moves[sq_no] = all_king_moves_from(sq_no);
        all_knight_moves[sq_no] = all_knight_moves_from(sq_no);
    }
}


// getting all possible pawn moves with promotion 
void MoveGen::gen_white_pawn_single_push(const Chess::Board &B, std::vector<MoveType> &moves)
{
    uint64_t whitePawns = B.WB;

    while(whitePawns) {
        int currPawnIndex = pop_lsb(whitePawns);
        
        uint64_t occupied = B.getOccupied(); 
        
        uint64_t currPawnSquare = (ONE << currPawnIndex); // current position

        uint64_t possibleNextPawnSquare = (currPawnIndex << 8);
        
        if( possibleNextPawnSquare & (~occupied) != 0){ // if move possible
            MoveType m = Move::encode(currPawnIndex, currPawnIndex + 8, static_cast<int>(Move::Piece::WP), 0, B.getRank(currPawnIndex + 8) == 7, 0);
            moves.push_back(m);
        }
    }
}

