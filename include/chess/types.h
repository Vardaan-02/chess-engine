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

// We use an 8-bit integer to represent a square. An enum provides type safety
// and makes the code more readable by allowing us to use names like E4 instead of numbers.
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
// We can represent all 12 pieces (6 types * 2 colors) with a single integer.
// This layout allows for easy calculation of piece type and color.
enum Piece : int8_t {
    NO_PIECE, // 0
    W_PAWN,   // 1
    B_PAWN,   // 2
    W_KNIGHT, // 3
    B_KNIGHT, // 4
    W_BISHOP, // 5
    B_BISHOP, // 6
    W_ROOK,   // 7
    B_ROOK,   // 8
    W_QUEEN,  // 9
    B_QUEEN,  // 10
    W_KING,   // 11
    B_KING,   // 12

    PIECE_NB = 13
};

// Helper function to get the PieceType of a Piece
constexpr PieceType type_of(Piece p) {
    if (p == NO_PIECE) return NO_PIECE_TYPE;
    return PieceType((p + 1) / 2); // (1,2)->1, (3,4)->2, etc.
}

// Helper function to get the Color of a Piece
constexpr Color color_of(Piece p) {
    if (p == NO_PIECE) return COLOR_NONE;
    return Color((p - 1) % 2); // 1,3,5... -> 0 (WHITE), 2,4,6... -> 1 (BLACK)
}

// Helper function to construct a Piece from a PieceType and Color
constexpr Piece make_piece(Color c, PieceType pt) {
    if (c == COLOR_NONE || pt == NO_PIECE_TYPE) return NO_PIECE;
    return Piece(pt * 2 - (1 - c));
}

//-----------------------------------------------------------------------------
// CASTLING RIGHTS
//-----------------------------------------------------------------------------

// Castling rights are stored as a 4-bit bitmask within a single byte.
// This allows for efficient updates and checks using bitwise operations.
enum CastlingRights : uint8_t {
    NO_CASTLING    = 0,
    WHITE_KINGSIDE = 1,          // 0001
    WHITE_QUEENSIDE = 2,         // 0010
    BLACK_KINGSIDE = 4,          // 0100
    BLACK_QUEENSIDE = 8,         // 1000

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

// A move is encoded into a 16-bit integer for memory efficiency.
//
// Bits | Description
// -----|------------------------------------------------
// 0-5  | "From" square (6 bits, 0-63)
// 6-11 | "To" square (6 bits, 0-63)
// 12-13| Promotion piece type (2 bits: N, B, R, Q)
// 14-15| Special move flags (2 bits)
//
// Special Flags:
// 00: Quiet move
// 01: Capture
// 10: Special 1 (En Passant, Castling)
// 11: Special 2 (Currently unused, could be for null moves)
using Move = uint16_t;

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

// Common evaluation constants
const Score VALUE_PAWN   = { 100, 120 };
const Score VALUE_KNIGHT = { 320, 320 };
const Score VALUE_BISHOP = { 330, 330 };
const Score VALUE_ROOK   = { 500, 500 };
const Score VALUE_QUEEN  = { 975, 975 };

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