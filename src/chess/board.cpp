#include "board.h"

namespace Chess {

// Constructor
Board::Board()
    : WP(0), WN(0), WB(0), WR(0), WQ(0), WK(0),
      BP(0), BN(0), BB(0), BR(0), BQ(0), BK(0),
      white_to_move(true), castling_rights(0),
      en_passant_index(-1), halfmove_clock(0),
      fullmove_number(1) {}

// Set a piece on a square
void Board::set_piece(uint64_t &bb, int sq) {
    bb |= (1ULL << sq);
}

// Remove a piece from a square
void Board::remove_piece(uint64_t &bb, int sq) {
    bb &= ~(1ULL << sq);
}

// Check if square is occupied
bool Board::is_occupied(uint64_t bb, int sq) const {
    return bb & (1ULL << sq);
}

// Convert rank/file to square index
int Board::square_index(int rank, int file) const {
    return rank * 8 + file;
}


//To be implemented
bool Board::is_attacked(uint64_t square) const{
    return false;
}

// Print ASCII board
void Board::print_board() const {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int sq = square_index(rank, file);
            char c = '.';
            if (is_occupied(WP, sq)) c = 'P';
            else if (is_occupied(WN, sq)) c = 'N';
            else if (is_occupied(WB, sq)) c = 'B';
            else if (is_occupied(WR, sq)) c = 'R';
            else if (is_occupied(WQ, sq)) c = 'Q';
            else if (is_occupied(WK, sq)) c = 'K';
            else if (is_occupied(BP, sq)) c = 'p';
            else if (is_occupied(BN, sq)) c = 'n';
            else if (is_occupied(BB, sq)) c = 'b';
            else if (is_occupied(BR, sq)) c = 'r';
            else if (is_occupied(BQ, sq)) c = 'q';
            else if (is_occupied(BK, sq)) c = 'k';
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
}

} // namespace Chess
