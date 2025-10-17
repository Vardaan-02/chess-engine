#include "chess/zobrist.h"
#include "chess/board.h"

uint64_t Zobrist::piecesArray[16][64];
uint64_t Zobrist::castlingRights[16];
uint64_t Zobrist::enPassantKey[64];
uint64_t Zobrist::sideToMove;

void Zobrist::init(){
    static std::mt19937_64 gen(123456789ULL); // Use a fixed seed for reproducibility
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 64; ++j)
            piecesArray[i][j] = dist(gen);

    for (int i = 0; i < 64; ++i)
        enPassantKey[i] = dist(gen);

    for (int i = 0; i < 16; ++i)
        castlingRights[i] = dist(gen);

    sideToMove = dist(gen);
}

uint64_t Zobrist::calculate_zobrist_hash(const Board& B)
{
    uint64_t hash{};

    for(int p = chess::WP; p <= chess::BK; ++p)
    {
        uint64_t pieceBitboard = B.bitboard[p];
        while(pieceBitboard)
        {
            chess::Square sq = util::pop_lsb(pieceBitboard);
            hash ^= piecesArray[p][sq];
        }
    }

    if(B.en_passant_sq != chess::SQUARE_NONE) hash ^= enPassantKey[B.en_passant_sq];
    hash ^= castlingRights[B.castle_rights];

    if(!B.white_to_move) hash ^= sideToMove;

    return hash;
}
