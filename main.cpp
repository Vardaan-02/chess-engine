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
    Search search_agent(512); // 128 MB transposition table
    std::thread search_thread;

    OpeningBook book;
    book.load_from_json("/home/harshit/code/chess-engine/data/opening_database/openings.json");

    uci(board, search_agent, search_thread, book);

    return 0;
}

