#pragma once

#include <cstdint>

// Forward-declaration of Board
class Board;

class Zobrist {
public:
    // --- PUBLIC METHODS ---

    /**
     * @brief Calculates a full Zobrist hash from scratch for a given board state.
     */
    static uint64_t calculate_zobrist_hash(const Board& B);

    /**
     * @brief Initializes the static Zobrist key arrays. 
     * MUST be called once at program startup.
     */
    static void init_zobrist_keys();

    // --- STATIC MEMBER VARIABLES (DECLARATIONS) ---
    // These tell the compiler that these variables exist.
    // The memory for them is allocated in zobrist.cpp.
    
    // [piece_index][square]
    static uint64_t piecesArray[12][64];
    
    // [0=WK, 1=WQ, 2=BK, 3=BQ]
    static uint64_t castlingRights[4];
    
    // [file]
    static uint64_t enPassantFile[8];
    
    // Hashed if it's white's turn
    static uint64_t sideToMove;
};