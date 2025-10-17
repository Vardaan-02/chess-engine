#include "chess/movegen.h"

void MoveGen::init(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly){
    if(B.double_check){
        generate_king_moves(B, moveList, capturesOnly);
        return;
    }

    generate_pawn_moves(B, moveList, capturesOnly);
    generate_knight_moves(B, moveList, capturesOnly);
    generate_orthogonal_sliders_moves(B, moveList, capturesOnly);
    generate_diagonal_sliders_moves(B, moveList, capturesOnly);
    generate_king_moves(B, moveList, capturesOnly);
}