#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <string>

constexpr uint64_t ONE = 1ULL;
constexpr int NUM_SQUARES = 64;

// Castling bitmask
constexpr uint8_t CASTLE_WHITE_K = 1;  // white kingside
constexpr uint8_t CASTLE_WHITE_Q = 2;  // white queenside
constexpr uint8_t CASTLE_BLACK_K = 4;  // black kingside
constexpr uint8_t CASTLE_BLACK_Q = 8;  // black queenside

// Piece encoding (for piece lists / captured type)
enum Piece : int8_t {
    NO_PIECE = 0,
    WP = 1, WN = 2, WB = 3, WR = 4, WQ = 5, WK = 6,
    BP = 9, BN = 10, BB = 11, BR = 12, BQ = 13, BK = 14 // 4 th bit for color 1,2 and 3 for piece type
};

// Move flags small bitfield (16-bit)
enum MoveFlag : uint16_t {
    FLAG_NONE       = 0,
    FLAG_CAPTURE    = 1 << 0,
    FLAG_PROMO      = 1 << 1,
    FLAG_EP         = 1 << 2,
    FLAG_CASTLE     = 1 << 3
};

// Compact move representation (32-bit)
struct Move {
    uint32_t m; // packed: from(6) | to(6) | flags(6) | promo(4) | unused
    Move() : m(0) {}
    Move(int from, int to, uint16_t flags=0, int promo=0) {
        m = (uint32_t)(from & 0x3F)
          | ((uint32_t)(to & 0x3F) << 6)
          | ((uint32_t)(flags & 0x3F) << 12)
          | ((uint32_t)(promo & 0xF) << 18);
    }
    int from() const { return m & 0x3F; }
    int to() const { return (m >> 6) & 0x3F; }
    uint16_t flags() const { return (m >> 12) & 0x3F; }
    int promo() const { return (m >> 18) & 0xF; }
};

// ---------- Minimal undo record (compact) ----------
struct Undo {
    uint64_t zobrist_before;      // full hash
    uint32_t captured_piece_and_halfmove; 
        // lower 8 bits: captured piece code
        // upper 24 bits: halfmove clock
    int8_t prev_en_passant_sq;    // -1 if none
    uint8_t prev_castle_rights;   // 4-bit mask
    int8_t promoted_to;           // 0 if none
    int8_t prev_white_king_sq;    // for incremental king position restore
    int8_t prev_black_king_sq;
    Undo() = default;
};

// ---------- Board state ----------
struct Board {
    // --- Bitboards: per-piece & color
    uint64_t bitboard[16];  // 0..5 white, 9..14 black

    // --- Square array for O(1) lookup
    int8_t board_array[64];

    // --- Side to move
    bool white_to_move;

    // --- Castling rights (4-bit mask) KQkq
    uint8_t castle_rights;

    // --- En-passant target square (square index 0..63 or -1 if none)
    int8_t en_passant_sq;

    // --- Halfmove clock & fullmove number
    uint32_t halfmove_clock;
    uint32_t fullmove_number;

    // --- King squares (cached) for quick check detection
    int8_t white_king_sq;
    int8_t black_king_sq;

    // --- Zobrist hash
    uint64_t zobrist_key;
    uint64_t zobrist_pawn_key; // optional pawn hash
    int32_t material_white;
    int32_t material_black;

    // --- Undo stack
    std::vector<Undo> undo_stack;

    // Cached occupancies
    uint64_t white_occupied;
    uint64_t black_occupied;
    uint64_t occupied;

    // ---------- API / helper prototypes ----------

    inline uint64_t get_white() const { return white_occupied; }
    inline uint64_t get_black() const { return black_occupied; }
    inline uint64_t get_occupied() const { return occupied; }
    inline uint64_t get_empty() const { return ~occupied; }
    inline uint64_t piece_bb(int piece_index) const { return bitboard[piece_index]; }

    inline void update_king_squares_from_bitboards() {
        white_king_sq = bitboard[WK] ? __builtin_ctzll(bitboard[WK]) : -1;
        black_king_sq = bitboard[BK] ? __builtin_ctzll(bitboard[BK]) : -1;
    }


    Board();
    void clear();
    void set_fen(const char *fen_cstr);
    std::string to_fen() const;

    // Make/unmake
    void make_move(const Move &mv);
    void unmake_move(const Move &mv);

    // Queries
    bool isempty(int sq) const { return board_array[sq] == NO_PIECE; }
    int piece_on_sq(int sq) const { return board_array[sq]; }
    bool square_attacked(int sq, bool by_white) const; // uses attack tables

private:
    inline void move_piece_bb(int piece, int from_sq, int to_sq) {
        bitboard[piece] ^= (ONE << from_sq) | (ONE << to_sq);
        board_array[from_sq] = NO_PIECE;
        board_array[to_sq] = piece;
        update_occupancies();
        if (piece == WK || piece == BK) update_king_squares_from_bitboards();
    }

    inline void restore_piece_bb(int piece, int from_sq, int to_sq) {
        bitboard[piece] ^= (ONE << from_sq) | (ONE << to_sq);
        board_array[from_sq] = piece;
        board_array[to_sq] = NO_PIECE;
        update_occupancies();
        if (piece == WK || piece == BK) update_king_squares_from_bitboards();
    }

    inline void update_material(int piece, bool add) {
        int val = 0;
        switch (piece % 6) {
            case 0: val = 100; break; // Pawn
            case 1: val = 320; break; // Knight
            case 2: val = 330; break; // Bishop
            case 3: val = 500; break; // Rook
            case 4: val = 900; break; // Queen
            case 5: val = 0; break;   // King
        }
        if (piece < 6) material_white += add ? val : -val;
        else           material_black += add ? val : -val;
    }

    inline void update_occupancies() {
        white_occupied = bitboard[WP] | bitboard[WN] | bitboard[WB] | bitboard[WR] | bitboard[WQ] | bitboard[WK];
        black_occupied = bitboard[BP] | bitboard[BN] | bitboard[BB] | bitboard[BR] | bitboard[BQ] | bitboard[BK];
        occupied   = white_occupied | black_occupied;
    }
};
