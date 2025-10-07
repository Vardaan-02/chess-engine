#include "movegen.h"

std::vector<chess::Move> moveList;

void MoveGen::init(const Board& B, std::vector<chess::Move>& moveList){
    generate_pawn_moves(B, moveList);
    generate_knight_moves(B, moveList);
    generate_king_moves(B, moveList);
    generate_orthogonal_sliders_moves(B, moveList);
    generate_diagonal_sliders_moves(B, moveList);
}
