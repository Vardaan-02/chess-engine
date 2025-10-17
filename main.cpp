#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include "chess/board.h"
#include "engine/search.h"
#include "chess/movegen.h"
#include "chess/util.h"

// Helper function to find a move in the legal move list that matches a UCI move string
// This version correctly handles promotion moves.
chess::Move parse_move(Board& board, const std::string& move_string) {
    std::vector<chess::Move> legal_moves;
    MoveGen::init(board, legal_moves, false);

    for (const auto& move : legal_moves) {
        std::string generated_move_str = util::move_to_string(move);

        // If the legal move is a promotion, append the piece character to our
        // generated string before comparing it to the incoming UCI move string.
        if (move.flags() & chess::FLAG_PROMO) {
            switch (chess::type_of((chess::Piece)move.promo())) {
                case chess::QUEEN:  generated_move_str += 'q'; break;
                case chess::ROOK:   generated_move_str += 'r'; break;
                case chess::BISHOP: generated_move_str += 'b'; break;
                case chess::KNIGHT: generated_move_str += 'n'; break;
                default: break;
            }
        }
        
        if (generated_move_str == move_string) {
            return move;
        }
    }
    return {}; // Return a null move if not found
}

// Function to run the search in a separate thread
// This version correctly formats the output string for promotion moves.
void start_search_thread(Board board, Search* search_agent, int depth) {
    chess::Move best_move = search_agent->start_search(board, depth);

    std::string move_str = util::move_to_string(best_move);

    // If the best move is a promotion, append the correct character for UCI.
    if (best_move.flags() & chess::FLAG_PROMO) {
        switch (chess::type_of((chess::Piece)best_move.promo())) {
            case chess::QUEEN:  move_str += 'q'; break;
            case chess::ROOK:   move_str += 'r'; break;
            case chess::BISHOP: move_str += 'b'; break;
            case chess::KNIGHT: move_str += 'n'; break;
            default: break; // Should not happen
        }
    }

    std::cout << "bestmove " << move_str << std::endl;
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    Board board;
    Search search_agent(128); // 128 MB transposition table
    std::thread search_thread;

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "uci") {
            std::cout << "id name Hagnus-Carlsen" << std::endl;
            std::cout << "id author Vardaan-Harshit" << std::endl;
            std::cout << "uciok" << std::endl;
        } else if (token == "isready") {
            chess::init(); // Initialize bitboards and other pre-computed data
            std::cout << "readyok" << std::endl;
        } else if (token == "ucinewgame") {
            search_agent.TT.clear(); // Clear the transposition table for a new game
        } else if (token == "position") {
            std::string pos_type;
            iss >> pos_type;
            std::string fen;
            std::string moves_token;

            if (pos_type == "startpos") {
                fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
                board.set_fen(fen);
                iss >> moves_token; // This should be "moves", or the stream will be empty
            } else if (pos_type == "fen") {
                std::string fen_part;
                // Read all parts of the FEN string until we hit "moves" or the end of the line
                while (iss >> fen_part && fen_part != "moves") {
                    fen += fen_part + " ";
                }
                board.set_fen(fen);

                // If the loop stopped because we found "moves", then moves_token is "moves"
                if (fen_part == "moves") {
                    moves_token = "moves";
                }
            }

            // If the "moves" token was found, apply the moves
            if (moves_token == "moves") {
                std::string move_str;
                while (iss >> move_str) {
                    chess::Move m = parse_move(board, move_str);
                    if (!m.is_null()) {
                        board.make_move(m);
                    }
                }
            }
        } else if (token == "go") {
            // Ensure any previous search is stopped and the thread is joined
            if (search_thread.joinable()) {
                search_agent.stopSearch.store(true);
                search_thread.join();
            }
            
            int depth = 8; // Default search depth
            std::string go_param;
            while(iss >> go_param) {
                if (go_param == "depth") {
                    iss >> depth;
                }
            }
            
            search_agent.stopSearch.store(false);
            // Launch the search in a new thread
            search_thread = std::thread(start_search_thread, board, &search_agent, depth);

        } else if (token == "stop") {
            search_agent.stopSearch.store(true);
            if (search_thread.joinable()) {
                search_thread.join();
            }
        } else if (token == "quit") {
            search_agent.stopSearch.store(true);
            if (search_thread.joinable()) {
                search_thread.join();
            }
            break; // Exit the loop and terminate the program
        }
    }

    return 0;
}

