#pragma once
#include <string>
#include <cstdint>
#include "board.h"

namespace FEN {

    // Parses a FEN string into a BoardState
    Chess::Board parse_fen(const std::string& fen);

    // Converts a BoardState back to a FEN string
    std::string to_fen(const Chess::Board & board);

}
