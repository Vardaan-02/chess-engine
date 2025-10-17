#include <vector>
#include <thread>
#include "chess/board.h"
#include "engine/search.h"
#include "engine/uci.h"
#include "engine/opening_book.h"

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    Board board;
    Search search_agent(128); // 128 MB transposition table
    std::thread search_thread;

    OpeningBook book;
    book.load_from_json("/home/vardaan-02/Shared/low-level-development/chess-engine/data/opening_database/openings.json");

    // std::string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // board.set_fen(starting_fen);
    // std::cout << board.zobrist_key << "\n";

    uci(board, search_agent, search_thread, book);

    return 0;
}

