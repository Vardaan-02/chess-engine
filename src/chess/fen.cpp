#include "fen.h"
#include <sstream>
#include <cctype>
#include <iostream>

namespace FEN {

// Parse FEN into Chess::Board
Chess::Board parse_fen(const std::string& fen) {
    Chess::Board board;
    std::stringstream ss(fen);
    std::string board_part, side, castling, enpassant;
    int halfmove = 0, fullmove = 1;

    ss >> board_part >> side >> castling >> enpassant >> halfmove >> fullmove;

    int rank = 7; // FEN starts from rank 8
    int file = 0;

    for (char c : board_part) {
        if (c == '/') {
            rank--;
            file = 0;
            continue;
        }

        if (isdigit(c)) {
            file += c - '0'; // skip empty squares
            continue;
        }

        int sq = board.square_index(rank, file);

        switch(c) {
            case 'P': board.set_piece(board.WP, sq); break;
            case 'N': board.set_piece(board.WN, sq); break;
            case 'B': board.set_piece(board.WB, sq); break;
            case 'R': board.set_piece(board.WR, sq); break;
            case 'Q': board.set_piece(board.WQ, sq); break;
            case 'K': board.set_piece(board.WK, sq); break;
            case 'p': board.set_piece(board.BP, sq); break;
            case 'n': board.set_piece(board.BN, sq); break;
            case 'b': board.set_piece(board.BB, sq); break;
            case 'r': board.set_piece(board.BR, sq); break;
            case 'q': board.set_piece(board.BQ, sq); break;
            case 'k': board.set_piece(board.BK, sq); break;
        }
        file++;
    }

    // Side to move
    board.white_to_move = (side == "w");

    // Castling rights: 4-bit mask
    board.castling_rights = 0;
    for (char c : castling) {
        switch(c) {
            case 'K': board.castling_rights |= 1 << 0; break; // white kingside
            case 'Q': board.castling_rights |= 1 << 1; break; // white queenside
            case 'k': board.castling_rights |= 1 << 2; break; // black kingside
            case 'q': board.castling_rights |= 1 << 3; break; // black queenside
        }
    }

    // En passant
    if (enpassant == "-") board.en_passant_square = -1;
    else {
        int file = enpassant[0] - 'a';
        int rank = enpassant[1] - '1';
        board.en_passant_square = board.square_index(rank, file);
    }

    board.halfmove_clock = halfmove;
    board.fullmove_number = fullmove;

    return board;
}

// Convert Chess::Board to FEN string (simplified, for validation)
std::string to_fen(const Chess::Board& board) {
    std::string fen;

    for (int rank = 7; rank >= 0; rank--) {
        int empty = 0;
        for (int file = 0; file < 8; file++) {
            int sq = board.square_index(rank, file);
            char piece = '.';
            if (board.WP & (1ULL << sq)) piece = 'P';
            else if (board.WN & (1ULL << sq)) piece = 'N';
            else if (board.WB & (1ULL << sq)) piece = 'B';
            else if (board.WR & (1ULL << sq)) piece = 'R';
            else if (board.WQ & (1ULL << sq)) piece = 'Q';
            else if (board.WK & (1ULL << sq)) piece = 'K';
            else if (board.BP & (1ULL << sq)) piece = 'p';
            else if (board.BN & (1ULL << sq)) piece = 'n';
            else if (board.BB & (1ULL << sq)) piece = 'b';
            else if (board.BR & (1ULL << sq)) piece = 'r';
            else if (board.BQ & (1ULL << sq)) piece = 'q';
            else if (board.BK & (1ULL << sq)) piece = 'k';

            if (piece == '.') empty++;
            else {
                if (empty > 0) { fen += std::to_string(empty); empty = 0; }
                fen += piece;
            }
        }
        if (empty > 0) fen += std::to_string(empty);
        if (rank > 0) fen += '/';
    }

    // Side to move
    fen += ' ';
    fen += (board.white_to_move ? 'w' : 'b');

    // Castling rights
    fen += ' ';
    std::string cr;
    if (board.castling_rights & (1 << 0)) cr += 'K';
    if (board.castling_rights & (1 << 1)) cr += 'Q';
    if (board.castling_rights & (1 << 2)) cr += 'k';
    if (board.castling_rights & (1 << 3)) cr += 'q';
    if (cr.empty()) cr = "-";
    fen += cr;

    // En passant
    fen += ' ';
    if (board.en_passant_square == -1) fen += '-';
    else {
        int r = board.en_passant_square / 8;
        int f = board.en_passant_square % 8;
        fen += ('a' + f);
        fen += ('1' + r);
    }

    // Halfmove and fullmove
    fen += ' ' + std::to_string(board.halfmove_clock);
    fen += ' ' + std::to_string(board.fullmove_number);

    return fen;
}

}
