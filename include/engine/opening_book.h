#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <random>
#include <unordered_map>
#include <fstream>
#include "chess/board.h"
#include "chess/zobrist.h"
#include "uci.h" // We need parse_move from here

#include "json.hpp"
using json = nlohmann::json;

class OpeningBook {
public:
    // This function will pre-process the JSON file and build the map.
    void load_from_json(const std::string& filepath) {
        Zobrist::init();
        try {
            
            std::ifstream f(filepath);
            if (!f.is_open()) {
                std::cout << "info string Could not open opening book file: " << filepath << std::endl;
                return;
            }

            json openings_json = json::parse(f);
            Board board; // A temporary board to play through lines

            for (const auto& line : openings_json) {
                std::string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
                board.set_fen(starting_fen); // Reset board for each opening line
                std::vector<std::string> moves = line.get<std::vector<std::string>>();

                for (const std::string& move_str : moves) {
                    uint64_t current_hash = board.zobrist_key;
                    chess::Move move = parse_move(board, move_str);

                    if (move.is_null()) {
                        break; 
                    }
                    
                    // Add this move to the list of possible moves for the current position's hash
                    // Avoid adding duplicate moves for a given position
                    bool found = false;
                    for(const auto& m : book[current_hash]){
                        if (m == move_str) {
                            found = true;
                            break;
                        }
                    }
                    if(!found) {
                        book[current_hash].push_back(move_str);
                    }

                    board.make_move(move);
                }
            }
            std::cout << "info string Book loaded. " << book.size() << " unique positions." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "info string Error loading opening book: " << e.what() << std::endl;
        }
    }

    // The query function now takes the board's current Zobrist hash.
    std::optional<std::string> getRandomMove(uint64_t current_hash) {
        auto it = book.find(current_hash);

        // Check if the position exists in the book
        if (it == book.end() || it->second.empty()) {
            return std::nullopt;
        }

        const std::vector<std::string>& possibleMoves = it->second;

        // --- Select a random move from the possibilities ---
        static thread_local std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<> distrib(0, possibleMoves.size() - 1);

        return possibleMoves[distrib(gen)];
    }

    // The book is now a hash map from Zobrist key to a vector of move strings
    std::unordered_map<uint64_t, std::vector<std::string>> book;
};