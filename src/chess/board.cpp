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
    castle_rights = chess::NO_CASTLING;
    en_passant_sq = chess::SQUARE_NONE;
    halfmove_clock = 0;
    fullmove_number = 1;
    white_king_sq = black_king_sq = chess::SQUARE_NONE;
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
    
    int sq = chess::A8; // start at A8
    for (char c : board_part) {
        if (c == '/') { sq -= 16; continue; }
        if (std::isdigit(c)) { sq += (c - '0'); continue; }
        int8_t piece = chess::NO_PIECE;
        switch(c) {
            case 'P': piece = chess::WP; break;
            case 'N': piece = chess::WN; break;
            case 'B': piece = chess::WB; break;
            case 'R': piece = chess::WR; break;
            case 'Q': piece = chess::WQ; break;
            case 'K': piece = chess::WK; break;
            case 'p': piece = chess::BP; break;
            case 'n': piece = chess::BN; break;
            case 'b': piece = chess::BB; break;
            case 'r': piece = chess::BR; break;
            case 'q': piece = chess::BQ; break;
            case 'k': piece = chess::BK; break;
            default: throw std::runtime_error("Invalid FEN char");
        }
        bitboard[piece] |= ONE << sq;
        board_array[sq] = piece;
        if (piece == chess::WK) white_king_sq = (chess::Square)sq; //Overload assignment operator for square
        if (piece == chess::BK) black_king_sq = (chess::Square)sq;
        sq++;
    }

    white_to_move = (side_part == "w");
    
    castle_rights = chess::NO_CASTLING;
    for (char c : castle_part) {
        switch(c) {
            case 'K': castle_rights |= chess::WHITE_KINGSIDE; break;
            case 'Q': castle_rights |= chess::WHITE_QUEENSIDE; break;
            case 'k': castle_rights |= chess::BLACK_KINGSIDE; break;
            case 'q': castle_rights |= chess::BLACK_QUEENSIDE; break;
        }
    }

    en_passant_sq = chess::SQUARE_NONE;

    if(ep_part != "-")
    {
        int file = ep_part[0] - 'a';
        int rank = ep_part[1] - '1';
        en_passant_sq = get_square_from_rank_file(rank, file);
    }

    fullmove_number = fullmove;

    update_occupancies();

    // // Optional: compute material
    // for (int i = chess::WP; i <= chess::WK; ++i) update_material(i, true);
    // for (int i = chess::BP; i <= chess::BK; ++i) update_material(i, true);
}

// ----------------- FEN serialization -----------------
std::string Board::to_fen() const {
    std::string fen;
    for (int rank = 7; rank >= 0; --rank) {
        int empty = 0;
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            int p = board_array[sq];
            if (p == chess::NO_PIECE) { empty++; }
            else {
                if (empty) { fen += ('0' + empty); empty = 0; }
                switch(p) {
                    case chess::WP: fen += 'P'; break;
                    case chess::WN: fen += 'N'; break;
                    case chess::WB: fen += 'B'; break;
                    case chess::WR: fen += 'R'; break;
                    case chess::WQ: fen += 'Q'; break;
                    case chess::WK: fen += 'K'; break;
                    case chess::BP: fen += 'p'; break;
                    case chess::BN: fen += 'n'; break;
                    case chess::BB: fen += 'b'; break;
                    case chess::BR: fen += 'r'; break;
                    case chess::BQ: fen += 'q'; break;
                    case chess::BK: fen += 'k'; break;
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
        if (castle_rights & chess::WHITE_KINGSIDE) fen += 'K';
        if (castle_rights & chess::WHITE_QUEENSIDE) fen += 'Q';
        if (castle_rights & chess::BLACK_KINGSIDE) fen += 'k';
        if (castle_rights & chess::BLACK_QUEENSIDE) fen += 'q';
    }
    fen += ' ';
    fen += (en_passant_sq == -1 ? "-" : "a1"); // TODO: map en_passant_sq to algebraic
    fen += ' ';
    fen += std::to_string(halfmove_clock);
    fen += ' ';
    fen += std::to_string(fullmove_number);
    return fen;
}
