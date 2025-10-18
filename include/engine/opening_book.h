#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <optional>
#include <algorithm> // Required for std::sort
#include <random>

// Use built-in functions for byte swapping, which are highly optimized
#if defined(__GNUC__) || defined(__clang__)
    #define BSWAP16(x) __builtin_bswap16(x)
    #define BSWAP32(x) __builtin_bswap32(x)
    #define BSWAP64(x) __builtin_bswap64(x)
#elif defined(_MSC_VER)
    #include <cstdlib>
    #define BSWAP16(x) _byteswap_ushort(x)
    #define BSWAP32(x) _byteswap_ulong(x)
    #define BSWAP64(x) _byteswap_uint64(x)
#else
    // A simple fallback for other compilers
    inline uint16_t BSWAP16(uint16_t x) { return (x >> 8) | (x << 8); }
    inline uint32_t BSWAP32(uint32_t x) { return (BSWAP16(x >> 16)) | (BSWAP16(x) << 16); }
    inline uint64_t BSWAP64(uint64_t x) { return ((uint64_t)BSWAP32(x >> 32)) | ((uint64_t)BSWAP32(x) << 32); }
#endif


// Each entry in a Polyglot book is 16 bytes.
struct BookEntry {
    uint64_t key;
    uint16_t move;
    uint16_t weight;
    uint32_t learn;
} __attribute__((packed));


class OpeningBook {
private:
    std::vector<BookEntry> entries;

    static std::string polyglot_move_to_uci(uint16_t move) {
        int from_sq = (move >> 6) & 63;
        int to_sq = move & 63;
        int promo_piece = (move >> 12) & 7;

        std::string uci_move;
        uci_move += (char)('a' + (from_sq % 8));
        uci_move += (char)('1' + (from_sq / 8));
        uci_move += (char)('a' + (to_sq % 8));
        uci_move += (char)('1' + (to_sq / 8));

        if (promo_piece != 0) {
            switch (promo_piece) {
                case 1: uci_move += 'n'; break;
                case 2: uci_move += 'b'; break;
                case 3: uci_move += 'r'; break;
                case 4: uci_move += 'q'; break;
            }
        }
        return uci_move;
    }

public:
    std::optional<std::string> getRandomMove(uint64_t hash) {
        std::cout << "info string [Book] Looking up hash: " << hash << std::endl;

        auto it = std::lower_bound(entries.begin(), entries.end(), hash,
            [](const BookEntry& entry, uint64_t key) {
                return entry.key < key;
            });

        if (it == entries.end() || it->key != hash) {
            std::cout << "info string [Book] No entry found for this position." << std::endl;
            return std::nullopt;
        }

        std::vector<const BookEntry*> move_options;
        uint32_t total_weight = 0;
        while (it != entries.end() && it->key == hash) {
            move_options.push_back(&(*it));
            total_weight += it->weight;
            it++;
        }

        if (move_options.empty()) {
            return std::nullopt;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> distrib(1, total_weight);
        uint32_t random_weight = distrib(gen);

        for (const auto& move_entry : move_options) {
            if (random_weight <= move_entry->weight) {
                std::string chosen_move = polyglot_move_to_uci(move_entry->move);
                std::cout << "info string [Book] Found move: " << chosen_move << std::endl;
                return chosen_move;
            }
            random_weight -= move_entry->weight;
        }
        
        std::string fallback_move = polyglot_move_to_uci(move_options[0]->move);
        std::cout << "info string [Book] Found move (fallback): " << fallback_move << std::endl;
        return fallback_move;
    }

    friend OpeningBook read_book(const std::string& path);
};


// Reads a Polyglot .bin file from the given path
inline OpeningBook read_book(const std::string& path) {
    OpeningBook book;
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open opening book file: " << path << std::endl;
        return book;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    long num_entries = size / sizeof(BookEntry);
    book.entries.resize(num_entries);

    if (num_entries > 0) {
        file.read(reinterpret_cast<char*>(book.entries.data()), size);
    }
    file.close();

    for (auto& entry : book.entries) {
        entry.key = BSWAP64(entry.key);
        entry.move = BSWAP16(entry.move);
        entry.weight = BSWAP16(entry.weight);
        entry.learn = BSWAP32(entry.learn);
    }

    // *** THE FIX ***
    // Re-sort the entries by key after byte-swapping.
    std::sort(book.entries.begin(), book.entries.end(), 
        [](const BookEntry& a, const BookEntry& b) {
            return a.key < b.key;
    });

    std::cout << "info string Book loaded: " << path << " with " << num_entries << " entries." << std::endl;

    return book;
}

#endif // OPENING_BOOK_H