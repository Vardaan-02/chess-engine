#include "movegen.h"

void MoveGen::init(const Board& B, std::vector<chess::Move>& moveList){
    if(B.double_check){
        generate_king_moves(B, moveList);
        return;
    }

    generate_pawn_moves(B, moveList);
    generate_knight_moves(B, moveList);
    generate_orthogonal_sliders_moves(B, moveList);
    generate_diagonal_sliders_moves(B, moveList);
    generate_king_moves(B, moveList);
}