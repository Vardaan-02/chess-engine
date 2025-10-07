#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include "types.h"
#include <string>

constexpr uint64_t ONE = 1ULL;

// ---------- Board state ----------
class Board {
public:
    // --- Bitboards: per-piece & color
    // 1 -> WP, 2 -> WN, 3 -> WB, 4 -> WR, 5 -> WQ, 6 -> WK 
    // 9 -> BP, 10 -> BN, 11-> BB, 12 -> BR, 13 -> BQ, 14 -> BK
    uint64_t bitboard[16];  // 1..6 white, 9..14 black

    // --- Square array for O(1) lookup
    int8_t board_array[64];

    // --- Side to move
    bool white_to_move;

    // --- Castling rights (4-bit mask) KQkq
    chess::CastlingRights castle_rights;

    // --- En-passant target square (square index 0..63 or -1 if none)
    chess::Square en_passant_sq;

    // --- Halfmove clock & fullmove number
    uint16_t halfmove_clock;
    uint32_t fullmove_number;

    // --- King squares (cached) for quick check detection
    chess::Square white_king_sq;
    chess::Square black_king_sq;

    // --- Zobrist hash
    uint64_t zobrist_key;
    uint64_t zobrist_pawn_key; // optional pawn hash
    int32_t material_white;
    int32_t material_black;

    // --- Undo stack
    std::vector<chess::Undo> undo_stack;

    // Cached occupancies
    uint64_t white_occupied;
    uint64_t black_occupied;
    uint64_t occupied;

    // ---------- API / helper prototypes ----------

    inline uint64_t get_white() { 
        update_occupancies(); 
        return white_occupied; 
    }

    inline uint64_t get_black() { 
        update_occupancies(); 
        return black_occupied; 
    }

    inline uint64_t get_occupied() { 
        update_occupancies(); 
        return occupied; 
    }
    
    inline uint64_t get_empty() { 
        update_occupancies(); 
        return ~occupied; 
    }
    
    inline bool is_square_occupied(chess::Square sq){
        return ((ONE << sq) & occupied);
    }

    inline bool is_square_occupied_by(chess::Square sq, bool byWhite)
    {
        return ((ONE << sq) & ((byWhite) ? white_occupied : black_occupied));
    }

    inline uint64_t piece_bb(int piece_index) const { return bitboard[piece_index]; }
    
    inline void update_king_squares_from_bitboards() {
        white_king_sq = bitboard[chess::WK] ? (chess::Square)__builtin_ctzll(bitboard[chess::WK]) : chess::SQUARE_NONE;
        black_king_sq = bitboard[chess::BK] ? (chess::Square)__builtin_ctzll(bitboard[chess::BK]) : chess::SQUARE_NONE;
    }

public:
    Board();
    void clear();
    void set_fen(std::string &fen_cstr);
    std::string to_fen() const;

    // Debug
    void print_board() const;

    // Make/unmake
    void make_move(const chess::Move &mv);
    void unmake_move(const chess::Move &mv);

    // Queries
    bool isempty(chess::Square sq) const { return board_array[sq] == chess::NO_PIECE; }
    int piece_on_sq(chess::Square sq) const { return board_array[sq]; }
    bool square_attacked(chess::Square sq, bool by_white) const; // uses attack tables

private:
    //assumes to_sq is empty
    inline void move_piece_bb(chess::Piece piece, chess::Square from_sq, chess::Square to_sq) {
        bitboard[piece] ^= (ONE << from_sq) | (ONE << to_sq);
        board_array[from_sq] = chess::NO_PIECE;
        board_array[to_sq] = piece;
        update_occupancies();
        if (piece == chess::WK || piece == chess::BK) update_king_squares_from_bitboards();
    }

    inline void restore_piece_bb(chess::Piece piece, chess::Square from_sq, chess::Square to_sq) {
        bitboard[piece] ^= (ONE << from_sq) | (ONE << to_sq);
        board_array[from_sq] = piece;
        board_array[to_sq] = chess::NO_PIECE;
        update_occupancies();
        if (piece == chess::WK || piece == chess::BK) update_king_squares_from_bitboards();
    }

    inline void update_material(chess::Piece piece, bool add) {
        int val = 0;
        switch (chess::type_of(piece)) {
            case 1: val = 100; break; // Pawn
            case 2: val = 320; break; // Knight
            case 3: val = 330; break; // Bishop
            case 4: val = 500; break; // Rook
            case 5: val = 900; break; // Queen
            case 6: val = 0; break;   // King
        }
        if (chess::color_of(piece) == chess::WHITE) material_white += add ? val : -val;
        else           material_black += add ? val : -val;
    }

    inline void update_occupancies() {
        white_occupied = bitboard[chess::WP] | bitboard[chess::WN] | bitboard[chess::WB] | bitboard[chess::WR] | bitboard[chess::WQ] | bitboard[chess::WK];
        black_occupied = bitboard[chess::BP] | bitboard[chess::BN] | bitboard[chess::BB] | bitboard[chess::BR] | bitboard[chess::BQ] | bitboard[chess::BK];
        occupied   = white_occupied | black_occupied;
    }
    //Assumes 0-Based indexing of the board, a1 = 0 (from bottom left). 0-based indexing for rank and files too
    inline chess::Square get_square_from_rank_file(int8_t rank, int8_t file) { return (chess::Square)(8 * rank + file); }
};
