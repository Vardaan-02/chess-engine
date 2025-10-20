#include "engine/search.h"
#include "chess/movegen.h"
#include "engine/move_orderer.h"
#include "utils/threadpool.h"
#include <vector>
#include <algorithm>

Search::Search(size_t s): nodes_searched(0), TT(s), stopSearch(false), pool(std::max(1u, std::thread::hardware_concurrency())) { }

void move_to_front(std::vector<chess::Move>& moves, const chess::Move& move_to_find) {
    auto it = std::find_if(moves.begin(), moves.end(), [&](const chess::Move& m) { return m.m == move_to_find.m; });
    if (it != moves.end()) {
        std::rotate(moves.begin(), it, it + 1);
    }
}

chess::Move Search::start_search(Board& board, int depth, int movetime, int wtime, int btime, int winc, int binc) {    
    stopSearch.store(false);
    TT.clear();

    // for (int i = 0; i < 15; ++i) {
    //     for (int j = 0; j < 64; ++j) {
    //         history_scores[i][j] /= 2; // Halve all scores
    //     }
    // }
    int time_for_move_ms;
    auto now = std::chrono::steady_clock::now();

    if (movetime > 0) {
        // A fixed time search was requested.
        time_for_move_ms = movetime;
        searchEndTime = now + std::chrono::milliseconds(time_for_move_ms);
    }
    else if (wtime > 0 || btime > 0) {
        int remaining_time = board.white_to_move ? wtime : btime;
        int increment      = board.white_to_move ? winc  : binc;

        double phase = std::clamp((board.game_phase*1.0) / util::TOTAL_PHASE, 0.0, 1.0);

        double base_time = (remaining_time / 25.0) + increment;

        int time_for_move_ms = static_cast<int>(base_time);

        if (phase > 0.15) time_for_move_ms = std::min(time_for_move_ms, 3500); // cap 3.5 sec max
        if (phase > 0.05) time_for_move_ms = std::min(time_for_move_ms, 8000); // cap 8 sec max
        else time_for_move_ms = std::min(time_for_move_ms, 15000); // cap 15 sec max

        std::cout << "phase: " << phase << " " << " time_for_move in seconds: " << (double)((time_for_move_ms*1.0)/1000) << "\n";

        if (remaining_time < 3 * 60 * 1000) {
            time_for_move_ms = 3000;
        }
        
        if(remaining_time < 1*60*1000) time_for_move_ms = 1000;

        time_for_move_ms = std::min(time_for_move_ms, remaining_time - 50);

        searchEndTime = now + std::chrono::milliseconds(time_for_move_ms);
    } else {
        // If no time is given, we assume 5 seconds
        searchEndTime = std::chrono::steady_clock::now() + std::chrono::seconds(5); 
    }
    
    chess::Move best_move_overall{};
    int64_t last_score = 0;

    for (int i = 1; i <= 60; ++i) {

        if (std::chrono::steady_clock::now() >= searchEndTime) {
            break;
        }

        nodes_searched = 0;
        
        int64_t alpha, beta;
        if (i > 4) {
            int64_t delta = 50;
            alpha = last_score - delta;
            beta = last_score + delta;
        } else {
            alpha = CHECKMATE_EVAL;
            beta = -CHECKMATE_EVAL;
        }

        while(true) {
            std::vector<chess::Move> moveList;
            MoveGen::init(board, moveList, false);
            if (!best_move_overall.is_null()) {
                move_to_front(moveList, best_move_overall);
            }

            int64_t current_alpha = alpha;
            chess::Move best_move_this_iter{};

            if (!moveList.empty()) {
                chess::Move m = moveList[0];
                board.make_move(m);
                if (board.is_position_legal()) {
                    int64_t s = -negamax(board, i - 1, 1, -beta, -current_alpha);
                    if (s > current_alpha) {
                        current_alpha = s;
                        best_move_this_iter = m;
                    }
                }
                board.unmake_move(m);
            }
            if (stopSearch.load()) break;

            std::vector<std::pair<std::future<int64_t>, chess::Move>> futures;
            for (size_t j = 1; j < moveList.size(); ++j) {
                Board b_copy = board;
                b_copy.make_move(moveList[j]);
                if (!b_copy.is_position_legal()) continue;
                futures.push_back({pool.enqueue(&Search::negamax, this, b_copy, i - 1, 1, -beta, -current_alpha), moveList[j]});
            }
            
            for (auto& [future, move] : futures) {
                if (stopSearch.load()) break;
                int64_t s = -future.get();

                if (s > current_alpha) {
                    current_alpha = s;
                    best_move_this_iter = move;
                }

                // std::cout << s << ": " << util::move_to_string(move) << '\n';
            }
            
            if (stopSearch.load()) break;

            if (current_alpha <= alpha) { // Fail-low
                alpha = CHECKMATE_EVAL;
                continue;
            }
            if (current_alpha >= beta) { // Fail-high
                beta = -CHECKMATE_EVAL;
                continue;
            }
            
            last_score = current_alpha;
            if (!best_move_this_iter.is_null()) {
                best_move_overall = best_move_this_iter;
            }
            TTEntry entry = { board.zobrist_key, (uint8_t)i, last_score, TTEntry::EXACT, best_move_overall };
            TT.store(entry);
            break; 
        }

        std::cout << "info depth " << i << " score cp " << last_score
        << " nodes " << nodes_searched << " pv " << util::move_to_string(best_move_overall) << std::endl;


        if (stopSearch.load()) break;
    }
    
    return best_move_overall;
}