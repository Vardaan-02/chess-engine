#include "movegen.h"

//Does not consider single pushes that lead to promotions
void generate_pawn_single_push(Board B, std::vector<chess::Move>& moveList)
{
    uint64_t pawnBitboard = B.white_to_move ? B.bitboard[chess::WP] : B.bitboard[chess::BP];

    pawnBitboard &= (B.white_to_move) ? ~chess::Bitboard::Rank7 : ~chess::Bitboard::Rank2;
    
    while(pawnBitboard)
    {
        chess::Square currPawnSquare = util::pop_lsb(pawnBitboard);
        chess::Square destinationSquare = util::shift_square(currPawnSquare, B.white_to_move ? chess::NORTH : chess::SOUTH);

        chess::Move m(currPawnSquare, destinationSquare,chess::FLAG_QUIET,chess::NO_PIECE);
        moveList.push_back(m);
    }
}

void generate_push_double_push(Board B, std::vector<chess::Move>& moveList)
{
    uint64_t pawnBitboard = B.white_to_move ? B.bitboard[chess::WP] : B.bitboard[chess::BP];

    pawnBitboard &= (B.white_to_move) ? chess::Bitboard::Rank2 : chess::Bitboard::Rank7;
    
    while(pawnBitboard)
    {
        chess::Square currPawnSquare = util::pop_lsb(pawnBitboard);

        chess::Square middlePawnSquare = util::shift_square(currPawnSquare, B.white_to_move ? chess::NORTH : chess::SOUTH);
        chess::Square destinationSquare =  util::shift_square(middlePawnSquare, B.white_to_move ? chess::NORTH : chess::SOUTH);

        chess::Move m(currPawnSquare, destinationSquare,chess::FLAG_QUIET,chess::NO_PIECE);
        moveList.push_back(m);
    }
}

//Does not consider EP Capture and Captures that lead to promotions
void generate_pawn_captures(Board B, std::vector<chess::Move>& moveList) 
{
    uint64_t pawnBitboard = B.white_to_move ? B.bitboard[chess::WP] : B.bitboard[chess::BP];

    pawnBitboard &= (B.white_to_move) ? ~chess::Bitboard::Rank7 : ~chess::Bitboard::Rank2;
    
    while(pawnBitboard)
    {
        chess::Square currPawnSquare = util::pop_lsb(pawnBitboard);
        uint64_t attackBitboard = chess::PawnAttacks[!B.white_to_move][currPawnSquare];

        while(attackBitboard)
        {
            chess::Square currAttackSquare = util::pop_lsb(attackBitboard);

            if(!B.is_square_occupied_by(currAttackSquare, !B.white_to_move)) continue;
            
            chess::Move m(currPawnSquare, currAttackSquare, chess::FLAG_CAPTURE, chess::NO_PIECE);
            moveList.push_back(m);
        }

    }
}

//Does not consider capture + promotion
void generate_pawn_promotion(Board B, std::vector<chess::Move>& moveList)
{

}

void generate_pawn_ep_captures(Board B, std::vector<chess::Move>& moveList)
{

}

void generate_pawn_promotion_captures(Board B, std::vector<chess::Move>& moveList)
{

}

void MoveGen::generate_pawn_moves(Board B, std::vector<chess::Move>& moveList)
{
       
}
