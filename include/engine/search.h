#pragma once

#include <atomic>
#include <cstdint>
#include "board.h"
#include "chess/types.h"
#include "chess/movegen.h"

#define DRAW_EVAL 0
#define CHECKMATE_EVAL -30000
#define INFINITY_EVAL -31000

class Search {
public:
    // Constructor
    Search();

    /**
     * @brief The main entry point to begin a search.
     * This will be expanded later to handle time management and iterative deepening.
     * For now, it performs a simple fixed-depth search.
     * @param board The starting position for the search.
     * @param depth The fixed depth to search to.
     * @return The best move found for the current position.
     */
    chess::Move start_search(Board& board, int depth);

    // Publicly accessible search statistics
    uint64_t nodes_searched;

private:
    /**
     * @brief The core Negamax search function with Alpha-Beta pruning.
     * @param board The current board state.
     * @param depth Remaining depth to search.
     * @param alpha The lower bound for the score (best score for maximizing player).
     * @param beta The upper bound for the score (best score for minimizing player).
     * @return The evaluation of the position from the side-to-move's perspective.
     */
    int negamax(Board& board, int depth, int alpha, int beta);

    /**
     * @brief Quiescence search to stabilize the evaluation at horizon nodes.
     * This search only considers "non-quiet" moves like captures to avoid the horizon effect.
     * @param board The current board state.
     * @param alpha The lower bound for the score.
     * @param beta The upper bound for the score.
     * @return The stabilized evaluation of the position.
     */
    int search_captures_only(Board& board, int alpha, int beta);

    /**
     * @brief Evaluates the board from the perspective of the side to move.
     * Initially, this will be a simple material count.
     * @param board The board state to evaluate.
     * @return The score in centipawns. Positive is good for the current player.
     */

    int evaluate(const Board& b);
};