#include "engine/transposition.h"
#include <cstring>

TranspositionTable::TranspositionTable(size_t size_mb) : locks(NumLocks)
{
    num_entries = (size_mb * 1024 * 1024) / sizeof(TTEntry);
    table = std::make_unique<TTEntry[]>(num_entries);
    clear();
}

void TranspositionTable::clear()
{
    std::memset(table.get(), 0, num_entries * sizeof(TTEntry));
}

void TranspositionTable::store(const TTEntry& entry)
{
    uint64_t index = entry.key % num_entries;
    
    std::lock_guard<std::mutex> guard(locks[entry.key % NumLocks]);

    // Replacement strategy: Always replace if the new entry is from a deeper search.
    // Also replace if the slot is empty (key == 0) to fill the table.
    if (entry.depth >= table[index].depth || table[index].key == 0)
    {
        table[index] = entry;
    }
}

bool TranspositionTable::probe(uint64_t key, TTEntry& entry)
{
    uint64_t index = key % num_entries;

    std::lock_guard<std::mutex> guard(locks[key % NumLocks]);

    entry = table[index];

    if (entry.key == key) {
        return true;
    }
    
    return false;
}