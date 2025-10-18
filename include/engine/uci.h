#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <fstream>
#include "chess/board.h"
#include "engine/search.h"
#include "chess/movegen.h"
#include "chess/util.h"

class OpeningBook;

chess::Move parse_move(Board& board, const std::string& move_string);

void start_search_thread(Board board, Search* search_agent, int depth, int movetime, int wtime, int btime, int winc, int binc);

void uci(Board &board, Search& search_agent, std::thread& search_thread, OpeningBook& white_book, OpeningBook& black_book);