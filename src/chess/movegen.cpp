#include "movegen.h"

std::vector<chess::Move> moveList;

void MoveGen::init(Board& B, std::vector<chess::Move>& moveList){
    
    generate_king_moves(B, moveList);

    if(!B.inDoubleCheck){
        generate_pawn_moves(B, moveList);
        generate_knight_moves(B, moveList);
        generate_orthogonal_sliders_moves(B, moveList);
        generate_diagonal_sliders_moves(B, moveList);
    }
}
