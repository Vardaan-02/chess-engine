#include "engine/search.h"
#include "chess/movegen.h"

int Search::evaluate(const Board& b) {
    int score = b.material_white - b.material_black;
    return b.white_to_move ? score : -score;
}