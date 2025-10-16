#pragma once

#include <cstdint>
#include <memory>
#include "chess/board.h"
#include "chess/types.h"
#include <memory>
#include <vector>
#include <mutex>

// This is the structure for a single entry in the Transposition Table.
struct TTEntry {
    enum Bound : uint8_t {
        EXACT,
        LOWER_BOUND,
        UPPER_BOUND
    };

    uint64_t key;
    uint8_t depth;
    int64_t score;
    Bound bound;
    chess::Move best_move;
};


class TranspositionTable {
private:
    std::unique_ptr<TTEntry[]> table;
    size_t num_entries;
    
    // We use a vector of mutexes instead of a single one to reduce lock contention.
    // This allows different threads to write to different parts of the table simultaneously.
    std::vector<std::mutex> locks;
    static const size_t NumLocks = 256; // A power of 2 is common for easy hashing

public:
    // Constructor initializes the table and locks.
    TranspositionTable(size_t size_mb);

    // Clears the table of all entries.
    void clear();

    // Stores a new entry in the table, handling potential collisions.
    void store(const TTEntry& entry);
    
    // Probes the table for an existing entry with the given key.
    bool probe(uint64_t key, TTEntry& entry);
};