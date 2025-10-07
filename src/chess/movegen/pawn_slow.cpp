#include "movegen.h"

//Does not consider single pushes that lead to promotions
void generate_pawn_single_push(const Board& B, std::vector<chess::Move>& moveList)
{
    uint64_t pawnBitboard = B.white_to_move ? (B.bitboard[chess::WP] & (~chess::Bitboard::Rank7)) : (B.bitboard[chess::BP] & (~chess::Bitboard::Rank2));
    
    while(pawnBitboard)
    {
        chess::Square currPawnSquare = util::pop_lsb(pawnBitboard);
        chess::Square destinationSquare = util::shift_square(currPawnSquare, B.white_to_move ? chess::NORTH : chess::SOUTH);

        if(B.is_square_occupied(destinationSquare)) continue;

        chess::Move m(currPawnSquare, destinationSquare,chess::FLAG_QUIET,chess::NO_PIECE);
        moveList.push_back(m);
    }
}

void generate_push_double_push(const Board& B, std::vector<chess::Move>& moveList)
{
    uint64_t pawnBitboard = B.white_to_move ? (B.bitboard[chess::WP] & (chess::Bitboard::Rank2)) : (B.bitboard[chess::BP] & (chess::Bitboard::Rank7));
    
    while(pawnBitboard)
    {
        chess::Square currPawnSquare = util::pop_lsb(pawnBitboard);

        chess::Square middlePawnSquare = util::shift_square(currPawnSquare, B.white_to_move ? chess::NORTH : chess::SOUTH);
        chess::Square destinationSquare =  util::shift_square(middlePawnSquare, B.white_to_move ? chess::NORTH : chess::SOUTH);

        if(B.is_square_occupied(destinationSquare) || B.is_square_occupied(middlePawnSquare)) continue;

        chess::Move m(currPawnSquare, destinationSquare,chess::FLAG_QUIET,chess::NO_PIECE);
        moveList.push_back(m);
    }
}

//Does not consider EP Capture and Captures that lead to promotions
void generate_pawn_captures(const Board& B, std::vector<chess::Move>& moveList) 
{
    uint64_t pawnBitboard = B.white_to_move ? (B.bitboard[chess::WP] & (~chess::Bitboard::Rank7)) : (B.bitboard[chess::BP] & (~chess::Bitboard::Rank2));
    
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

void add_pawn_promotion_moves(const Board& B,const chess::Square currSq, const chess::Square dstSq, 
                                const chess::MoveFlag flags, std::vector<chess::Move>& moveList)
{
    std::vector<chess::PieceType> pieces = {chess::KNIGHT, chess::BISHOP, chess::ROOK, chess::QUEEN};
    chess::Color color = (B.white_to_move) ? chess::WHITE : chess::BLACK; 

    for(auto piece : pieces)
    {
        chess::Move m(currSq, dstSq, flags, chess::make_piece(color, piece));
        moveList.push_back(m);
    }
}

//Does not consider capture + promotion
void generate_pawn_promotion(const Board& B, std::vector<chess::Move>& moveList)
{
    uint64_t pawnBitboard = ((B.white_to_move) ? (B.bitboard[chess::WP] & (chess::Bitboard::Rank7)) : (B.bitboard[chess::BP] & (chess::Bitboard::Rank2)));

    while(pawnBitboard){
        chess::Square currPawnSquare = util::pop_lsb(pawnBitboard);
        chess::Square destinationSquare = util::shift_square(currPawnSquare, (B.white_to_move) ? chess::NORTH : chess::SOUTH);

        if(B.is_square_occupied(destinationSquare)) continue;

        add_pawn_promotion_moves(B, currPawnSquare, destinationSquare, chess::FLAG_PROMO, moveList);
    }
}

void generate_pawn_ep_captures(const Board& B, std::vector<chess::Move>& moveList)
{
    if(B.en_passant_sq == chess::SQUARE_NONE) return;
    
    uint64_t enPassantBitboard = util::create_bitboard_from_square(B.en_passant_sq);

    //Only the 5th Rank or 4th Rank as per the color
    uint64_t pawnBitboard = (B.white_to_move) ? (B.bitboard[chess::WP] & chess::Bitboard::Rank5) : (B.bitboard[chess::BP] & chess::Bitboard::Rank4);

    while(pawnBitboard)
    {
        chess::Square currPawnSquare = util::pop_lsb(pawnBitboard);
        
        //PawnAttacks[0][sq] for white masks and PawnAttacks[1][sq] for black pawns
        if(!(chess::PawnAttacks[!B.white_to_move][currPawnSquare] & enPassantBitboard)) continue;

        chess::Move m(currPawnSquare, B.en_passant_sq, chess::FLAG_EP, chess::NO_PIECE);
        moveList.push_back(m);
    }
}

void generate_pawn_promotion_captures(const Board& B, std::vector<chess::Move>& moveList)
{   
    //Get bitboard for pawns on 7th or 2nd rank
    uint64_t pawnBitboard = (B.white_to_move) ? (B.bitboard[chess::WP] & chess::Bitboard::Rank7) : (B.bitboard[chess::BP] & chess::Bitboard::Rank2);
    
    while(pawnBitboard)
    {
        chess::Square currPawnSquare = util::pop_lsb(pawnBitboard);
        uint64_t attackBitboard = chess::PawnAttacks[!B.white_to_move][currPawnSquare];

        while(attackBitboard)
        {
            chess::Square attackSquare = util::pop_lsb(attackBitboard);

            if(!B.is_square_occupied_by(attackSquare, !B.white_to_move)) continue;

            add_pawn_promotion_moves(B, currPawnSquare, attackSquare, chess::FLAG_CAPTURE_PROMO, moveList);
        }
    }
}

void MoveGen::generate_pawn_moves(const Board& B, std::vector<chess::Move>& moveList)
{
    generate_pawn_single_push(B,moveList);
    generate_push_double_push(B,moveList);
    generate_pawn_captures(B,moveList);
    generate_pawn_promotion(B,moveList);
    generate_pawn_ep_captures(B,moveList);
    generate_pawn_promotion_captures(B,moveList);
}
