#include "board.h"
#include "fen.h"
#include <iostream>

int main() {
    Chess::Board board;
    std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    board = FEN::parse_fen(start_fen);

    std::cout << "Board loaded from FEN:\n";
    board.print_board();

    std::cout << "\nWhite to move: " << (board.white_to_move ? "Yes":"No") << "\n";
    std::cout << "Castling rights bits: " << board.castling_rights << "\n";
    std::cout << "En passant square: " << board.en_passant_square << "\n";
    std::cout << "Halfmove clock: " << board.halfmove_clock << "\n";
    std::cout << "Fullmove number: " << board.fullmove_number << "\n";
}
