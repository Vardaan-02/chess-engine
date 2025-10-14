#include "transposition.h"
#include <cstring>

TranspositionTable::TranspositionTable(size_t size_mb)
{
    num_entries = (size_mb * 1024 * 1024) / sizeof(TTEntry);

    table = std::make_unique<TTEntry[]>(num_entries);

    clear(); //Clean the table to start with
}

void TranspositionTable::clear()
{
    std::memset(table.get(), 0, num_entries * sizeof(TTEntry));
}

void TranspositionTable::store(const TTEntry& entry)
{
    uint64_t index = entry.key % num_entries;

    if(entry.depth >= table[index].depth)
    {
        table[index] = entry;
    }
}

bool TranspositionTable::probe(uint64_t key, TTEntry& entry)
{
    uint64_t index = key % num_entries;
    entry = table[index];
    //if same key is present there
    if(entry.key == key) return true;
    //either a collision or empty
    return false;
}


