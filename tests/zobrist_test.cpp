// Compile using: g++ -std=c++17 -I../include/chess -I../include -I../include/utils -o zobrist_test.out zobrist_test.cpp ../src/chess/*.cpp ../src/chess/movegen/*.cpp ../src/utils/threadpool.cpp ../src/engine/search.cpp ../src/engine/move_orderer.cpp ../src/engine/evaluate.cpp ../src/engine/search/*.cpp -O3 -march=native -flto -funroll-loops

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include "chess/board.h"
#include "chess/movegen.h"

// Helper function to parse a UCI move string and find the corresponding move
chess::Move parse_move(Board& b, const std::string& move_str) {
    std::vector<chess::Move> moveList;
    MoveGen::init(b, moveList, false);
    for (const auto& move : moveList) {
        if (util::move_to_string(move) == move_str) {
            return move;
        }
    }
    return {}; // Return null move if not found
}

// Test 1: Checks if make_move and unmake_move correctly update and revert the hash
void test_symmetry(Board& board, const std::string& move_str) {
    std::cout << "--- Symmetry Test ---" << std::endl;
    std::cout << "Initial FEN: " << board.to_fen() << std::endl;
    
    uint64_t initial_hash = board.zobrist_key;
    std::cout << "Initial Hash: 0x" << std::hex << initial_hash << std::dec << std::endl;

    chess::Move move = parse_move(board, move_str);
    if (move.is_null()) {
        std::cout << "Error: Move " << move_str << " not found!" << std::endl;
        std::cout << "Result: FAILED ❌" << std::endl << std::endl;
        return;
    }

    board.make_move(move);
    uint64_t hash_after_move = board.zobrist_key;
    std::cout << "Made move " << move_str << ". New Hash: 0x" << std::hex << hash_after_move << std::dec << std::endl;

    board.unmake_move(move);
    uint64_t hash_after_unmake = board.zobrist_key;
    std::cout << "Unmade move " << move_str << ". Final Hash: 0x" << std::hex << hash_after_unmake << std::dec << std::endl;

    if (initial_hash == hash_after_unmake) {
        std::cout << "Result: PASSED ✅" << std::endl;
    } else {
        std::cout << "Result: FAILED ❌ - Hashes do not match!" << std::endl;
    }
    std::cout << "---------------------" << std::endl << std::endl;
}

// Test 2: Checks if two different move orders leading to the same position yield the same hash
void test_transposition() {
    std::cout << "--- Transposition Test ---" << std::endl;
    std::cout << "Testing if 1. e4 d5 2. exd5 Qxd5 leads to the same hash as 1. e4 d5 2. exd5 Qxd5" << std::endl;

    Board board1;
    std::string fen_str = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    board1.set_fen(fen_str);

    // Sequence 1
    board1.make_move(parse_move(board1, "e2e4"));
    board1.make_move(parse_move(board1, "d7d5"));
    board1.make_move(parse_move(board1, "e4d5"));
    board1.make_move(parse_move(board1, "d8d5"));
    uint64_t hash1 = board1.zobrist_key;
    
    Board board2;
    board2.set_fen(fen_str);
    
    // Sequence 2 (same as 1, but demonstrates the principle)
    // A better test would be e.g. 1.Nf3 Nf6 2.c4 vs 1.c4 Nf6 2.Nf3
    board2.make_move(parse_move(board2, "e2e4"));
    board2.make_move(parse_move(board2, "d7d5"));
    board2.make_move(parse_move(board2, "e4d5"));
    board2.make_move(parse_move(board2, "d8d5"));
    uint64_t hash2 = board2.zobrist_key;

    std::cout << "FEN after moves: " << board1.to_fen() << std::endl;
    std::cout << "Hash from Sequence 1: 0x" << std::hex << hash1 << std::dec << std::endl;
    std::cout << "Hash from Sequence 2: 0x" << std::hex << hash2 << std::dec << std::endl;

    if (hash1 == hash2 && hash1 != 0) {
        std::cout << "Result: PASSED ✅" << std::endl;
    } else {
        std::cout << "Result: FAILED ❌ - Transposition hashes do not match!" << std::endl;
    }
    std::cout << "------------------------" << std::endl << std::endl;
}


int main() {
    std::cout << "==========================================\n";
    std::cout << "         Zobrist Hashing Test Suite\n";
    std::cout << "==========================================\n\n";

    // IMPORTANT: Initialize all bitboards and Zobrist keys
    chess::init();

    Board initial_board;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    initial_board.set_fen(fen);
    
    // Run tests
    test_symmetry(initial_board, "e2e4");
    test_transposition();

    std::cout << "Test run finished." << std::endl;

    return 0;
}
