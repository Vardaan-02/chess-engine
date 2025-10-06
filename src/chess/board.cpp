#include "board.h"
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <cctype>

// ----------------- Constructor -----------------
Board::Board() {
    clear();
}

// ----------------- Clear board -----------------
void Board::clear() {
    std::memset(bitboard, 0, sizeof(bitboard));
    std::memset(board_array, 0, sizeof(board_array));
    white_to_move = true;
    castle_rights = 0;
    en_passant_sq = -1;
    halfmove_clock = 0;
    fullmove_number = 1;
    white_king_sq = black_king_sq = -1;
    zobrist_key = zobrist_pawn_key = 0;
    material_white = material_black = 0;
    white_occupied = black_occupied = occupied = 0;
    undo_stack.clear();
}

// ----------------- FEN parsing -----------------
void Board::set_fen(const char *fen_cstr) {
    clear();
    std::string fen(fen_cstr);
    std::istringstream iss(fen);
    std::string board_part, side_part, castle_part, ep_part;
    int fullmove = 1;

    iss >> board_part >> side_part >> castle_part >> ep_part >> halfmove_clock >> fullmove;
    
    int sq = 56; // start at A8
    for (char c : board_part) {
        if (c == '/') { sq -= 16; continue; }
        if (std::isdigit(c)) { sq += (c - '0'); continue; }
        int piece = NO_PIECE;
        switch(c) {
            case 'P': piece = WP; break;
            case 'N': piece = WN; break;
            case 'B': piece = WB; break;
            case 'R': piece = WR; break;
            case 'Q': piece = WQ; break;
            case 'K': piece = WK; break;
            case 'p': piece = BP; break;
            case 'n': piece = BN; break;
            case 'b': piece = BB; break;
            case 'r': piece = BR; break;
            case 'q': piece = BQ; break;
            case 'k': piece = BK; break;
            default: throw std::runtime_error("Invalid FEN char");
        }
        bitboard[piece] |= ONE << sq;
        board_array[sq] = piece;
        if (piece == WK) white_king_sq = sq;
        if (piece == BK) black_king_sq = sq;
        sq++;
    }

    white_to_move = (side_part == "w");
    
    castle_rights = 0;
    for (char c : castle_part) {
        switch(c) {
            case 'K': castle_rights |= CASTLE_WHITE_K; break;
            case 'Q': castle_rights |= CASTLE_WHITE_Q; break;
            case 'k': castle_rights |= CASTLE_BLACK_K; break;
            case 'q': castle_rights |= CASTLE_BLACK_Q; break;
        }
    }

    en_passant_sq = -1; // TODO: convert ep square string if needed

    fullmove_number = fullmove;

    update_occupancies();

    // Optional: compute material
    for (int i = WP; i <= WK; ++i) update_material(i, true);
    for (int i = BP; i <= BK; ++i) update_material(i, true);
}

// ----------------- FEN serialization -----------------
std::string Board::to_fen() const {
    std::string fen;
    for (int rank = 7; rank >= 0; --rank) {
        int empty = 0;
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            int p = board_array[sq];
            if (p == NO_PIECE) { empty++; }
            else {
                if (empty) { fen += ('0' + empty); empty = 0; }
                switch(p) {
                    case WP: fen += 'P'; break;
                    case WN: fen += 'N'; break;
                    case WB: fen += 'B'; break;
                    case WR: fen += 'R'; break;
                    case WQ: fen += 'Q'; break;
                    case WK: fen += 'K'; break;
                    case BP: fen += 'p'; break;
                    case BN: fen += 'n'; break;
                    case BB: fen += 'b'; break;
                    case BR: fen += 'r'; break;
                    case BQ: fen += 'q'; break;
                    case BK: fen += 'k'; break;
                }
            }
        }
        if (empty) fen += ('0' + empty);
        if (rank != 0) fen += '/';
    }
    fen += ' ';
    fen += (white_to_move ? 'w' : 'b');
    fen += ' ';
    if (castle_rights == 0) fen += '-';
    else {
        if (castle_rights & CASTLE_WHITE_K) fen += 'K';
        if (castle_rights & CASTLE_WHITE_Q) fen += 'Q';
        if (castle_rights & CASTLE_BLACK_K) fen += 'k';
        if (castle_rights & CASTLE_BLACK_Q) fen += 'q';
    }
    fen += ' ';
    fen += (en_passant_sq == -1 ? "-" : "a1"); // TODO: map en_passant_sq to algebraic
    fen += ' ';
    fen += std::to_string(halfmove_clock);
    fen += ' ';
    fen += std::to_string(fullmove_number);
    return fen;
}

// ----------------- Make move -----------------
void Board::make_move(const Move &mv) {
    // TODO: implement full move handling (captures, promotions, castling, en-passant)
    int from = mv.from();
    int to   = mv.to();
    int piece = board_array[from];

    move_piece_bb(piece, from, to);
}

// ----------------- Unmake move -----------------
void Board::unmake_move(const Move &mv) {
    // TODO: implement full unmake logic
    int from = mv.from();
    int to   = mv.to();
    int piece = board_array[to];

    restore_piece_bb(piece, from, to);
}

// ----------------- Square attacked (stub) -----------------
bool Board::square_attacked(int sq, bool by_white) const {
    // TODO: implement using precomputed attack tables
    return false;
}
