#pragma once
#include <cstdint>
#include <string>

// ---------------- Move Encoding ----------------
// We will store moves as a single 32-bit integer:
//  0-5   : from square (0-63)
//  6-11  : to square (0-63)
//  12-15 : piece moved (pawn=1, knight=2, ...)
//  16-19 : captured piece (0 if none)
//  20-23 : promotion piece (0 if none)
//  24-27 : flags (en-passant, castling, etc.)
//  28-31 : reserved (future use)

using MoveType = uint32_t;

namespace Move
{

    // Piece codes
    enum class Piece : uint8_t
    {
        EMPTY = 0,
        WP = 1,
        WN,
        WB,
        WR,
        WQ,
        WK,
        BP = 9,
        BN,
        BB,
        BR,
        BQ,
        BK
    };

    // Flags
    enum class Flags : uint8_t
    {
        QUIET = 0,
        DOUBLE_PUSH = 1,
        KING_CASTLE = 2,
        QUEEN_CASTLE = 3,
        CAPTURE = 4,
        EP_CAPTURE = 5,
        PROMOTION = 6
    };

    // ---------------- Encoding / Decoding ----------------
    inline MoveType encode(int from, int to, int piece, int captured = 0, int promo = 0, int flags = 0)
    {
        return (from & 0x3F) |
               ((to & 0x3F) << 6) |
               ((piece & 0xF) << 12) |
               ((captured & 0xF) << 16) |
               ((promo & 0xF) << 20) |
               ((flags & 0xF) << 24);
    }

    inline int from_sq(MoveType m) { return m & 0x3F; }
    inline int to_sq(MoveType m) { return (m >> 6) & 0x3F; }
    inline int moved_piece(MoveType m) { return (m >> 12) & 0xF; }
    inline int captured_piece(MoveType m) { return (m >> 16) & 0xF; }
    inline int promo_piece(MoveType m) { return (m >> 20) & 0xF; }
    inline int move_flags(MoveType m) { return (m >> 24) & 0xF; }

    // Optional: human-readable move string (e.g., "e2e4", "e7e8q")
    inline std::string to_string(MoveType m)
    {
        static const char *files = "abcdefgh";
        static const char *ranks = "12345678";

        int f = from_sq(m);
        int t = to_sq(m);
        int promo = promo_piece(m);

        std::string str;
        str += files[f % 8];
        str += ranks[f / 8];
        str += files[t % 8];
        str += ranks[t / 8];

        if (promo != 0)
        {
            char p = " pnbrqkPNBRQK"[promo];
            str += tolower(p);
        }

        return str;
    }

} // namespace Move
