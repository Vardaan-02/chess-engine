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
    Search search_agent(512); // 512 MB transposition table
    std::thread search_thread;

    OpeningBook white_book = read_book("/home/vardaan-02/Shared/low-level-development/chess-engine/data/opening_database/white.bin");
    OpeningBook black_book = read_book("/home/vardaan-02/Shared/low-level-development/chess-engine/data/opening_database/black.bin");

    // for (auto i:white_book.entries){
    //     std::cout << i.key << " " << i.learn << " " << i.move << " " << i.weight << std::endl;
    // }

    uci(board, search_agent, search_thread, white_book, black_book);

    return 0;
}

