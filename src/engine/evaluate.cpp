#include "engine/search.h"
#include "engine/evaluate.h"

int Search::evaluate(const Board& b) {
    int mg_score = 0;
    int eg_score = 0;

    Search::pawn_evaluation(b, mg_score, eg_score);
    // std::cout << "Pawn Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    Search::knight_evaluation(b, mg_score, eg_score);
    // std::cout << "Knight Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    Search::bishop_evaluation(b, mg_score, eg_score);
    // std::cout << "Bishop Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    Search::rook_evaluation(b, mg_score, eg_score);
    // std::cout << "Rook Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    Search::queen_evaluation(b, mg_score, eg_score);
    // std::cout << "Queen Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;
    Search::king_evaluation(b, mg_score, eg_score);
    // std::cout << "King Eval: " << mg_score << " (MG), " << eg_score << " (EG)" << std::endl;

    int final_score = (mg_score * b.game_phase + eg_score * (util::TOTAL_PHASE - b.game_phase)) / util::TOTAL_PHASE;

    // return final_score;
    return b.white_to_move ? final_score : -final_score;

}