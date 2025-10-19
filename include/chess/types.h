#pragma once

/**
 * @file types.h
 * @brief Contains all fundamental type definitions for the chess engine.
 *
 * This header is the foundational layer of the entire engine. It defines the basic
 * data types for squares, pieces, colors, moves, and evaluation scores. By
 * centralizing these definitions, we ensure consistency and prevent circular
 * dependencies between other modules.
 */

#include <cstdint>
#include <string>

namespace chess {

//-----------------------------------------------------------------------------
// SQUARES
//-----------------------------------------------------------------------------

// We use an 8-bit integer to represent a square.
enum Square : int8_t {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    
    SQUARE_NB = 64,      // Total number of squares
    SQUARE_NONE = 65     // Represents an invalid or off-board square
};

inline Square& operator++(Square& s) {
    if (s < SQUARE_NB - 1)  // stay in range
        s = static_cast<Square>(static_cast<int>(s) + 1);
    else
        s = SQUARE_NONE; // overflow protection, wrap to invalid
    return s;
}

inline Square operator++(Square& s, int) {
    Square old = s;
    ++s;
    return old;
}


//-----------------------------------------------------------------------------
// COLORS
//-----------------------------------------------------------------------------

enum Color : int8_t {
    WHITE,
    BLACK,

    COLOR_NB = 2,        // Number of colors
    COLOR_NONE = 3
};

// Overload the '~' operator to easily flip a color.
// E.g., Color opponent = ~my_color;
constexpr Color operator~(Color c) {
    return Color(c ^ BLACK); // XOR with BLACK (1) flips the color
}

//-----------------------------------------------------------------------------
// PIECE TYPES & PIECES
//-----------------------------------------------------------------------------

enum PieceType : int8_t {
    NO_PIECE_TYPE,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,

    PIECE_TYPE_NB = 7
};

// A Piece is a combination of a PieceType and a Color.
enum Piece : int8_t {
    NO_PIECE = 0,
    WP = 1, WN = 2, WB = 3, WR = 4, WQ = 5, WK = 6,
    BP = 9, BN = 10, BB = 11, BR = 12, BQ = 13, BK = 14 // 4 th bit for color 1,2 and 3 for piece type
};

constexpr uint8_t colorMask = 0b1000;
constexpr uint8_t typeMask = 0b0111;

// Helper function to get the PieceType of a Piece
constexpr PieceType type_of(Piece p) {
    if (p == NO_PIECE) return NO_PIECE_TYPE;
    return (PieceType)(p & typeMask);
}


// Helper function to get the Color of a Piece
constexpr Color color_of(Piece p) {
    if (p == NO_PIECE) return COLOR_NONE;
    return Color(p & colorMask); // 4th bit = 0 -> White, 1 -> Black
}

// Helper function to construct a Piece from a PieceType and Color
constexpr Piece make_piece(Color c, PieceType pt) {
    if (c == COLOR_NONE || pt == NO_PIECE_TYPE) return NO_PIECE;
    return Piece((c << 3) | pt);
}

//-----------------------------------------------------------------------------
// CASTLING RIGHTS
//-----------------------------------------------------------------------------

// Castling rights are stored as a 4-bit bitmask within a single byte.
enum CastlingRights : uint8_t {
    NO_CASTLING    = 0,
    BLACK_QUEENSIDE = 1,          // 0001
    BLACK_KINGSIDE = 2,         // 0010
    WHITE_QUEENSIDE = 4,          // 0100
    WHITE_KINGSIDE = 8,         // 1000

    KING_SIDE = WHITE_KINGSIDE | BLACK_KINGSIDE,
    QUEEN_SIDE = WHITE_QUEENSIDE | BLACK_QUEENSIDE,
    WHITE_CASTLING = WHITE_KINGSIDE | WHITE_QUEENSIDE,
    BLACK_CASTLING = BLACK_KINGSIDE | BLACK_QUEENSIDE,
    ALL_CASTLING = WHITE_CASTLING | BLACK_CASTLING
};

// Enable bitwise operations for CastlingRights enum
inline CastlingRights& operator|=(CastlingRights& a, CastlingRights b) { return a = CastlingRights(a | b); }
inline CastlingRights& operator&=(CastlingRights& a, CastlingRights b) { return a = CastlingRights(a & b); }

//-----------------------------------------------------------------------------
// MOVES
//-----------------------------------------------------------------------------

// A move is encoded into a 32-bit integer for memory efficiency.
// Move flags small bitfield (16-bit)
enum MoveFlag : uint16_t {
    FLAG_QUIET              = 0,
    FLAG_CAPTURE            = 1 << 0,
    FLAG_PROMO              = 1 << 1,
    FLAG_EP                 = 1 << 2,
    FLAG_CASTLE             = 1 << 3,
    FLAG_CAPTURE_PROMO      = (FLAG_CAPTURE | FLAG_PROMO),
    FLAG_DOUBLE_PUSH        = 1 << 4
};

enum Direction : uint8_t {
    NORTH = 0,
    SOUTH = 1,
    EAST = 2,
    WEST = 3,
    SOUTH_EAST = 4,
    SOUTH_WEST = 5,
    NORTH_EAST = 6,
    NORTH_WEST = 7
};

// Compact move representation (32-bit)
struct Move {   //          LSB                          MSB        
    uint32_t m; // packed: from(6) | to(6) | flags(16) | promoPiece(4)
    Move() : m(0) {}
    Move(int from, int to, uint16_t flags=0, int promo=0) {
        m = (uint32_t)(from & 0x3F)
          | ((uint32_t)(to & 0x3F) << 6)
          | ((uint32_t)(flags & 0xFFFF) << 12)
          | ((uint32_t)(promo & 0xF) << 28);
    }
    int from() const { return m & 0x3F; }
    int to() const { return (m >> 6) & 0x3F; }
    uint16_t flags() const { return (m >> 12) & 0xFFFF; }
    int promo() const { return (m >> 28) & 0xF; }
    bool is_null() const { return m == 0; }
};

// ---------- Minimal undo record (compact) ----------
struct Undo {
    uint64_t zobrist_before;      // full hash
    uint16_t captured_piece_and_halfmove; 
        // lower 4 bits: captured piece code
        // upper 12 bits: halfmove clock (halfmove clock <= 50)
    int8_t prev_en_passant_sq;    // -1 if none
    uint8_t prev_castle_rights;   // 4-bit mask
    int8_t promoted_to;           // 0 if none 
    int8_t prev_white_king_sq;    // for incremental king position restore
    int8_t prev_black_king_sq;
    uint64_t pinned;         // single pass using ray attacks
    uint64_t checks;       // squares of checking pieces
    bool double_check;
    uint64_t check_mask; // rays + checker squares
    int game_phase;
    Undo() = default;
};


//Will review later
//-----------------------------------------------------------------------------
// AI & EVALUATION TYPES
//-----------------------------------------------------------------------------

// Depth is measured in plies (one half-move).
// A signed 8-bit integer is more than sufficient.
using Depth = int8_t;

// Score is a 32-bit signed integer representing the evaluation in centipawns.
// We use a struct to hold both middlegame and endgame scores, as piece
// values and positional bonuses change throughout the game.
struct Score {
    int16_t mg;
    int16_t eg;
};

// Operator overloads for convenient Score arithmetic
inline Score operator+(Score s1, Score s2) { return { int16_t(s1.mg + s2.mg), int16_t(s1.eg + s2.eg) }; }
inline Score operator-(Score s1, Score s2) { return { int16_t(s1.mg - s2.mg), int16_t(s1.eg - s2.eg) }; }
inline Score operator*(int i, Score s) { return { int16_t(i * s.mg), int16_t(i * s.eg) }; }

inline constexpr int square_distance(Square s1, Square s2) {
    const int f1 = s1 % 8;
    const int r1 = s1 / 8;
    const int f2 = s2 % 8;
    const int r2 = s2 / 8;
    return std::max(std::abs(f1 - f2), std::abs(r1 - r2));
}


// A score indicating a forced mate. We leave a buffer to store the number
// of plies until mate (e.g., MATE - 5 for mate in 5).
const int MATE_SCORE = 30000;

//-----------------------------------------------------------------------------
// CONSTANTS
//-----------------------------------------------------------------------------

// Board and game constants
const int MAX_GAME_MOVES = 1024; // Used for static move arrays
const int MAX_PLY = 128;         // Max search depth

} // namespace chess