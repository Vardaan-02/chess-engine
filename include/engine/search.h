#pragma once

#include <atomic>
#include <cstdint>
#include <chrono>
#include "chess/board.h"
#include "chess/types.h"
#include "chess/movegen.h"
#include "transposition.h"
#include "utils/threadpool.h"

#define DRAW_EVAL 0
#define CHECKMATE_EVAL -(int)1e7
#define NEG_INFINITY_EVAL (-(int)1e9)
#define MAX_PLY 64

class MoveOrderer;

class Search {
public:
    // Constructor
    Search(size_t size_of_tt_mb);

    /**
     * @brief The main entry point to begin a search.
     * This will be expanded later to handle time management and iterative deepening.
     * For now, it performs a simple fixed-depth search.
     * @param board The starting position for the search.
     * @param depth The fixed depth to search to.
     * @return The best move found for the current position.
     */
    chess::Move start_search(Board& board, int depth, int movetime, int wtime, int btime, int winc, int binc);

    // Publicly accessible search statistics
    uint64_t nodes_searched;
    chess::Move killer_moves[MAX_PLY][2];
    chess::Move pv_table[MAX_PLY][MAX_PLY];
    int history_scores[15][64]{}; // [piece][dest_sq]
    static int evaluate(const Board& b);
    TranspositionTable TT;
    std::atomic<bool> stopSearch;
    ThreadPool pool;
    std::chrono::steady_clock::time_point searchEndTime; 

private:
    /**
     * @brief The core Negamax search function with Alpha-Beta pruning.
     * @param board The current board state.
     * @param depth Remaining depth to search.
     * @param alpha The lower bound for the score (best score for maximizing player).
     * @param beta The upper bound for the score (best score for minimizing player).
     * @return The evaluation of the position from the side-to-move's perspective.
     */
    int64_t negamax(Board& board, int depth, int ply, int64_t alpha, int64_t beta);

    /**
     * @brief Quiescence search to stabilize the evaluation at horizon nodes.
     * This search only considers "non-quiet" moves like captures to avoid the horizon effect.
     * @param board The current board state.
     * @param alpha The lower bound for the score.
     * @param beta The upper bound for the score.
     * @return The stabilized evaluation of the position.
     */
    int64_t search_captures_only(Board& board, int ply, int64_t alpha, int64_t betas);

    /**
     * @brief Evaluates the board from the perspective of the side to move.
     * Initially, this will be a simple material count.
     * @param board The board state to evaluate.
     * @return The score in centipawns. Positive is good for the current player.
     */

    inline void update_killers(int ply, const chess::Move& move) {
        if (killer_moves[ply][0].m != move.m) {
            killer_moves[ply][1] = killer_moves[ply][0];
            killer_moves[ply][0] = move;
        }
    }

    inline void update_history(const Board& B, const chess::Move& move, int depth) {
        history_scores[B.board_array[move.from()]][move.to()] += depth*depth;  //depth * depth since we want cutoffs near the root
    }

};