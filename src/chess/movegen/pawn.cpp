#include "chess/movegen.h"

void add_pawn_promotion_moves(const Board& B, const chess::Square currSq, const chess::Square dstSq, const chess::MoveFlag flags, std::vector<chess::Move>& moveList)
{
    const std::vector<chess::PieceType> pieces = {chess::KNIGHT, chess::BISHOP, chess::ROOK, chess::QUEEN};
    const chess::Color color = B.white_to_move ? chess::WHITE : chess::BLACK;

    for (const auto piece : pieces)
    {
        chess::Move m(currSq, dstSq, flags, chess::make_piece(color, piece));
        moveList.push_back(m);
    }
}

void generate_pawn_single_push(const Board& B, std::vector<chess::Move>& moveList)
{
    const uint64_t our_pawns = B.white_to_move ? B.bitboard[chess::WP] : B.bitboard[chess::BP];
    const uint64_t empty_squares = ~B.occupied;
    const chess::Direction push_dir = B.white_to_move ? chess::NORTH : chess::SOUTH;
    const chess::Direction pull_dir = B.white_to_move ? chess::SOUTH : chess::NORTH;

    const uint64_t pawns_to_push = B.white_to_move ? (our_pawns & ~util::Rank7) : (our_pawns & ~util::Rank2);

    uint64_t destinations = util::shift_board(pawns_to_push, push_dir) & empty_squares;

    while (destinations)
    {
        const chess::Square to = util::pop_lsb(destinations);
        const chess::Square from = util::shift_square(to, pull_dir);
        moveList.push_back(chess::Move(from, to, chess::FLAG_QUIET, chess::NO_PIECE));
    }
}

void generate_push_double_push(const Board& B, std::vector<chess::Move>& moveList)
{
    const uint64_t our_pawns = B.white_to_move ? B.bitboard[chess::WP] : B.bitboard[chess::BP];
    const uint64_t empty_squares = ~B.occupied;
    const chess::Direction push_dir = B.white_to_move ? chess::NORTH : chess::SOUTH;
    const chess::Direction pull_dir = B.white_to_move ? chess::SOUTH : chess::NORTH;

    const uint64_t pawns_on_start_rank = B.white_to_move ? (our_pawns & util::Rank2) : (our_pawns & util::Rank7);

    const uint64_t pushes1 = util::shift_board(pawns_on_start_rank, push_dir) & empty_squares;
    uint64_t destinations = util::shift_board(pushes1, push_dir) & empty_squares;

    while (destinations)
    {
        const chess::Square to = util::pop_lsb(destinations);
        const chess::Square from = util::shift_square(util::shift_square(to, pull_dir), pull_dir);
        moveList.push_back(chess::Move(from, to, chess::FLAG_DOUBLE_PUSH, chess::NO_PIECE));
    }
}

void generate_pawn_captures(const Board& B, std::vector<chess::Move>& moveList)
{
    const uint64_t our_pawns = B.white_to_move ? B.bitboard[chess::WP] : B.bitboard[chess::BP];
    const uint64_t opponent_pieces = B.white_to_move ? B.black_occupied
                                                     : B.white_occupied;

    const uint64_t pawns_to_capture = B.white_to_move ? (our_pawns & ~util::Rank7)
                                                     : (our_pawns & ~util::Rank2);

    const chess::Direction dir1 = B.white_to_move ? chess::NORTH_WEST : chess::SOUTH_WEST;
    const chess::Direction pull_dir1 = B.white_to_move ? chess::SOUTH_EAST : chess::NORTH_EAST;
    uint64_t captures1 = util::shift_board(pawns_to_capture, dir1) & opponent_pieces;

    while (captures1)
    {
        const chess::Square to = util::pop_lsb(captures1);
        const chess::Square from = util::shift_square(to, pull_dir1);
        moveList.push_back(chess::Move(from, to, chess::FLAG_CAPTURE, chess::NO_PIECE));
    }

    const chess::Direction dir2 = B.white_to_move ? chess::NORTH_EAST : chess::SOUTH_EAST;
    const chess::Direction pull_dir2 = B.white_to_move ? chess::SOUTH_WEST : chess::NORTH_WEST;
    uint64_t captures2 = util::shift_board(pawns_to_capture, dir2) & opponent_pieces;

    while (captures2)
    {
        const chess::Square to = util::pop_lsb(captures2);
        const chess::Square from = util::shift_square(to, pull_dir2);
        moveList.push_back(chess::Move(from, to, chess::FLAG_CAPTURE, chess::NO_PIECE));
    }
}

void generate_pawn_promotion(const Board& B, std::vector<chess::Move>& moveList)
{
    const uint64_t our_pawns = B.white_to_move ? B.bitboard[chess::WP] : B.bitboard[chess::BP];
    const uint64_t empty_squares = ~B.occupied;
    const chess::Direction push_dir = B.white_to_move ? chess::NORTH : chess::SOUTH;
    const chess::Direction pull_dir = B.white_to_move ? chess::SOUTH : chess::NORTH;

    const uint64_t promoting_pawns = B.white_to_move ? (our_pawns & util::Rank7) : (our_pawns & util::Rank2);

    uint64_t destinations = util::shift_board(promoting_pawns, push_dir) & empty_squares;

    while (destinations)
    {
        const chess::Square to = util::pop_lsb(destinations);
        const chess::Square from = util::shift_square(to, pull_dir);
        add_pawn_promotion_moves(B, from, to, chess::FLAG_PROMO, moveList);
    }
}

void generate_pawn_ep_captures(const Board& B, std::vector<chess::Move>& moveList)
{
    if (B.en_passant_sq == chess::SQUARE_NONE) return;

    const uint64_t our_pawns = B.white_to_move ? B.bitboard[chess::WP] : B.bitboard[chess::BP];
    
    // Find which of our pawns attack the en passant square.
    // Putting an opposite color pawn in place of en passant square to get friendly pawns which can attack that square
    uint64_t attackers = chess::PawnAttacks[B.white_to_move][B.en_passant_sq];
    uint64_t attacking_pawns = attackers & our_pawns;

    while (attacking_pawns)
    {
        const chess::Square from = util::pop_lsb(attacking_pawns);
        moveList.push_back(chess::Move(from, B.en_passant_sq, chess::FLAG_EP, chess::NO_PIECE));
    }
}

void generate_pawn_promotion_captures(const Board& B, std::vector<chess::Move>& moveList)
{
    const uint64_t our_pawns = B.white_to_move ? B.bitboard[chess::WP] : B.bitboard[chess::BP];
    const uint64_t opponent_pieces = B.white_to_move ? B.black_occupied
                                                     : B.white_occupied;
    
    const uint64_t promoting_pawns = B.white_to_move ? (our_pawns & util::Rank7)
                                                     : (our_pawns & util::Rank2);

    const chess::Direction dir1 = B.white_to_move ? chess::NORTH_WEST : chess::SOUTH_WEST;
    const chess::Direction pull_dir1 = B.white_to_move ? chess::SOUTH_EAST : chess::NORTH_EAST;
    uint64_t captures1 = util::shift_board(promoting_pawns, dir1) & opponent_pieces;

    while (captures1)
    {
        const chess::Square to = util::pop_lsb(captures1);
        const chess::Square from = util::shift_square(to, pull_dir1);
        add_pawn_promotion_moves(B, from, to, chess::FLAG_CAPTURE_PROMO, moveList);
    }

    const chess::Direction dir2 = B.white_to_move ? chess::NORTH_EAST : chess::SOUTH_EAST;
    const chess::Direction pull_dir2 = B.white_to_move ? chess::SOUTH_WEST : chess::NORTH_WEST;
    uint64_t captures2 = util::shift_board(promoting_pawns, dir2) & opponent_pieces;

    while (captures2)
    {
        const chess::Square to = util::pop_lsb(captures2);
        const chess::Square from = util::shift_square(to, pull_dir2);
        add_pawn_promotion_moves(B, from, to, chess::FLAG_CAPTURE_PROMO, moveList);
    }
}

void MoveGen::generate_pawn_moves(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly)
{
    generate_pawn_captures(B, moveList);
    generate_pawn_ep_captures(B, moveList);
    generate_pawn_promotion_captures(B, moveList);

    if(!capturesOnly)
    {
        generate_pawn_single_push(B, moveList);
        generate_push_double_push(B, moveList);
        generate_pawn_promotion(B, moveList);
    }
}