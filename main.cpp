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
chess::Move parse_move(Board& board, const std::string& move_string) {
    std::vector<chess::Move> legal_moves;
    MoveGen::init(board, legal_moves, false);

    for (const auto& move : legal_moves) {
        if (util::move_to_string(move) == move_string) {
            return move;
        }
        // Handle promotions (e.g., "e7e8q")
        if (move.flags() & chess::FLAG_PROMO) {
            std::string promo_str = util::move_to_string(move);
            char promo_char = ' ';
            switch (chess::type_of((chess::Piece)move.promo())) {
                case chess::QUEEN:  promo_char = 'q'; break;
                case chess::ROOK:   promo_char = 'r'; break;
                case chess::BISHOP: promo_char = 'b'; break;
                case chess::KNIGHT: promo_char = 'n'; break;
                default: break;
            }
            if (promo_str + promo_char == move_string) {
                return move;
            }
        }
    }
    return {}; // Return a null move if not found
}

void start_search_thread(Board& board, Search& search_agent, int depth) {
    chess::Move best_move = search_agent.start_search(board, depth);
    std::cout << "bestmove " << util::move_to_string(best_move) << std::endl;
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

            if (pos_type == "startpos") {
                std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
                board.set_fen(start_fen);
            } else if (pos_type == "fen") {
                std::string fen_part;
                std::string fen;
                while (iss >> fen_part && fen_part != "moves") {
                    fen += fen_part + " ";
                }
                board.set_fen(fen);
                // If the last token read was not "moves", we need to put it back
                 if (iss.eof() && fen_part != "moves") {
                    // This case is for "position fen <fen>" without a "moves" part.
                 } else if (fen_part != "moves"){
                    // This should not happen with well-formed UCI, but as a safeguard:
                    // A better implementation would handle the stream state more carefully.
                 }
            }

            std::string moves_token;
            // The logic above consumes the token that might be "moves", so we re-check
            if (line.find("moves") != std::string::npos) {
                 // Fast-forward the string stream to after "moves"
                std::string temp_token;
                while(iss >> temp_token && temp_token != "moves");
                
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
                // Here you would parse other "go" parameters like wtime, btime, etc.
            }
            
            // Launch the search in a new thread
            search_thread = std::thread(start_search_thread, std::ref(board), std::ref(search_agent), depth);

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
