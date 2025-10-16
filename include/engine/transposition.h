#pragma once
#include <cstdint>
#include <memory>
#include "chess/board.h"
#include "chess/types.h"

struct TTEntry {
    uint64_t key;
    // The depth of the search that stored this entry.
    uint8_t depth;
    // The score of the position from the perspective of the side to move.
    int64_t score;
    // The type of score (exact, a lower bound, or an upper bound).
    enum Bound : uint8_t { EXACT, LOWER_BOUND, UPPER_BOUND } bound;
    // The best move found for this position.
    chess::Move best_move;
};

class TranspositionTable {
public:
    // Constructor: allocates a table of a given size in Megabytes.
    TranspositionTable(size_t size_mb);

    // Clears the table.
    void clear();

    // Probes the table for a given Zobrist key.
    // Returns true if a valid entry is found, and fills the 'entry' parameter.
    bool probe(uint64_t key, TTEntry& entry);

    // Stores an entry into the table.
    void store(const TTEntry& entry);

private:
    // A pointer to the block of memory for the table.
    std::unique_ptr<TTEntry[]> table;
    // The total number of entries in the table.
    size_t num_entries;
};