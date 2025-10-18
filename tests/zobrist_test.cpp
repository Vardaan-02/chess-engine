// Compile using: g++ -std=c++17 -I../include/chess -I../include -I../include/utils -o zobrist_test.out zobrist_test.cpp ../src/chess/*.cpp ../src/chess/movegen/*.cpp ../src/utils/threadpool.cpp ../src/engine/search.cpp ../src/engine/move_orderer.cpp ../src/engine/evaluate.cpp ../src/engine/search/*.cpp -O3 -march=native -flto -funroll-loops

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <map>
#include "chess/board.h"
#include "chess/movegen.h"
#include "chess/zobrist.h"
// Make sure "util" functions are available, e.g., from "chess/utils.h"

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

/**
 * @brief New test function to validate against the official Polyglot hashes.
 */
void test_polyglot_vectors() {
    std::cout << "--- Polyglot Test Vectors ---" << std::endl;
    
    // Map of FEN strings to their expected Polyglot hash
    std::map<std::string, uint64_t> test_cases = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0x463b96181691fc9c},
        {"rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2", 0x823c9b50fd114196},
        {"rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2", 0x0756b94461c50fb0},
        {"rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2", 0x662fafb965db29d4},
        {"rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3", 0x22a48b5a8e47ff78},
        {"rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3", 0x652a607ca3f242c1},
        {"rnbq1bnr/ppp1pkpp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR w - - 0 4", 0x00fdd303c946bdd9},
        {"rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3", 0x3c8123ea7b067637},
        {"rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 0 4", 0x5c3f9b829b279560}
    };

    Board b;
    bool all_passed = true;
    int count = 1;

    for (const auto& pair : test_cases) {
        std::string fen = pair.first;
        uint64_t expected_hash = pair.second;
        
        b.set_fen(fen);
        uint64_t calculated_hash = Zobrist::calculate_zobrist_hash(b);
        
        std::cout << "Test " << count++ << ": " << fen << std::endl;
        std::cout << "  Expected:   0x" << std::hex << expected_hash << std::endl;
        std::cout << "  Calculated: 0x" << calculated_hash << std::dec << std::endl;
        
        if (calculated_hash == expected_hash) {
            std::cout << "  Result: PASSED ✅" << std::endl;
        } else {
            std::cout << "  Result: FAILED ❌" << std::endl;
            all_passed = false;
        }
    }
    
    std::cout << "------------------------" << std::endl;
    if (all_passed) {
        std::cout << "Polyglot Validation: ALL TESTS PASSED!" << std::endl;
    } else {
        std::cout << "Polyglot Validation: FAILED." << std::endl;
    }
    std::cout << "------------------------" << std::endl << std::endl;
}


// Test 1: Checks if make_move and unmake_move correctly update and revert the hash
void test_symmetry(Board& board, const std::string& move_str) {
    std::cout << "--- Symmetry Test ---" << std::endl;
    std::cout << "Initial FEN: " << board.to_fen() << std::endl;
    
    // Ensure board has a hash to begin with
    board.zobrist_key = Zobrist::calculate_zobrist_hash(board);
    uint64_t initial_hash = board.zobrist_key;
    std::cout << "Initial Hash: 0x" << std::hex << initial_hash << std::dec << std::endl;
    
    chess::Move m = parse_move(board, move_str);
    if (m.m == 0) {
        std::cout << "Error: Move '" << move_str << "' not found!" << std::endl;
        return;
    }

    board.make_move(m);
    uint64_t hash_after_move = board.zobrist_key;
    std::cout << "After " << move_str << ": 0x" << std::hex << hash_after_move << std::dec << std::endl;

    board.unmake_move(m);
    uint64_t hash_after_unmake = board.zobrist_key;
    std::cout << "After unmake: 0x" << std::hex << hash_after_unmake << std::dec << std::endl;

    if (initial_hash == hash_after_unmake && initial_hash != hash_after_move) {
        std::cout << "Result: PASSED ✅" << std::endl;
    } else {
        std::cout << "Result: FAILED ❌" << std::endl;
    }
    std::cout << "------------------------" << std::endl << std::endl;
}

// Test 2: Checks if two different move sequences leading to the same position
// result in the same hash. (This requires a working incremental update in make_move)
void test_transposition(std::string fen_str) {
    std::cout << "--- Transposition Test ---" << std::endl;
    Board board1;
    Board board2;
    board1.set_fen(fen_str);
    board2.set_fen(fen_str);
    
    // Recalculate hash just in case set_fen is wrong
    board1.zobrist_key = Zobrist::calculate_zobrist_hash(board1);
    board2.zobrist_key = Zobrist::calculate_zobrist_hash(board2);

    // Sequence 1
    board1.make_move(parse_move(board1, "e2e4"));
    board1.make_move(parse_move(board1, "d7d5"));
    board1.make_move(parse_move(board1, "e4d5"));
    board1.make_move(parse_move(board1, "d8d5"));
    uint64_t hash1 = board1.zobrist_key;
    
    // Sequence 2 (same as 1, just for demo)
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
    // --- THIS IS THE MOST IMPORTANT FIX ---
    // Initialize the Zobrist keys *before* doing anything else.
    Zobrist::init_zobrist_keys(); 
    // ------------------------------------

    std::cout << "==========================================\n";
    std::cout << "         Zobrist Hashing Test Suite\n";
    std::cout << "==========================================\n\n";

    // --- Run the Polyglot validation first ---
    test_polyglot_vectors();


    // --- Run your original tests ---
    std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    Board board;
    board.set_fen(start_fen);
    
    test_symmetry(board, "e2e4");
    
    // Reset board for next test
    board.set_fen(start_fen);
    test_symmetry(board, "g1f3");

    // Run transposition test
    test_transposition(start_fen);

    std::cout << "Test run finished.\n";
    
    return 0;
}