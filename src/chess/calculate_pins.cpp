#include "board.h"

uint64_t get_line_between(chess::Square s1, chess::Square s2) {
    if (s1 == s2) return 0ULL;

    int s1_rank = s1 / 8, s1_file = s1 % 8;
    int s2_rank = s2 / 8, s2_file = s2 % 8;

    int dr = (s2_rank > s1_rank) - (s2_rank < s1_rank); // direction in rank (-1,0,1)
    int df = (s2_file > s1_file) - (s2_file < s1_file); // direction in file (-1,0,1)

    // must be same rank, file, or diagonal
    if (!((dr == 0) ^ (df == 0)) && !(std::abs(dr) == std::abs(df))) {
        return 0ULL;
    }

    uint64_t mask = 0ULL;
    int curr_rank = s1_rank + dr;
    int curr_file = s1_file + df;

    while (curr_rank != s2_rank || curr_file != s2_file) {
        mask |= (1ULL << (curr_rank * 8 + curr_file));
        curr_rank += dr;
        curr_file += df;
    }

    // remove endpoints
    mask &= ~((1ULL << s1) | (1ULL << s2));
    return mask;
}


inline uint64_t get_ray_between(chess::Square s1, chess::Square s2) {
    uint64_t line = get_line_between(s1,s2);
    return line | (util::create_bitboard_from_square(s1) | util::create_bitboard_from_square(s2));
}


void Board::calculate_orthogonal_pins(){
    chess::Color color = (white_to_move) ? chess::WHITE : chess::BLACK;
    chess::Color oppColor = (white_to_move) ? chess::BLACK : chess::WHITE;

    chess::Square friendly_king_square = (color == chess::WHITE) ? white_king_sq : black_king_sq;
    uint64_t entire_bitboard = occupied;
    uint64_t friendly_bitboard = (color == chess::WHITE) ? white_occupied : black_occupied;
    
    uint64_t opp_ortho_piece_bitboard = bitboard[chess::make_piece(oppColor, chess::ROOK)] | bitboard[chess::make_piece(oppColor, chess::QUEEN)];

    //Placing ortho piece at king square to remove irrelevant ortho pieces
    opp_ortho_piece_bitboard &= chess::get_orthogonal_slider_attacks(friendly_king_square, 0); 

    while(opp_ortho_piece_bitboard)
    {
        chess::Square opp_ortho_piece_sq = util::pop_lsb(opp_ortho_piece_bitboard);
        
        uint64_t line_between = get_line_between(friendly_king_square, opp_ortho_piece_sq);

        uint64_t pieces_on_line = line_between & entire_bitboard;

        // The condition for a pin is that there is EXACTLY ONE piece on this line.
        // And that piece must be friendly.
        if (util::count_bits(pieces_on_line) == 1 && (pieces_on_line & friendly_bitboard)) 
        {
            uint64_t pinned_piece_bb = pieces_on_line; // This is the bitboard of the pinned piece.

            pin_bitboard |= pinned_piece_bb;
            chess::Square piece_square = util::lsb(pinned_piece_bb);

            // Your pin ray calculation is correct.
            // pinRays[piece_square] = (line_between | util::create_bitboard_from_square(opp_ortho_piece_sq));
            pinRays[piece_square] = get_ray_between(friendly_king_square, opp_ortho_piece_sq);
        }
        else if(util::count_bits(pieces_on_line) == 0) { 
            checker_bitboard |= (util::create_bitboard_from_square(opp_ortho_piece_sq));
            checkRay |= get_ray_between(friendly_king_square, opp_ortho_piece_sq);
        }
    }
}

void Board::calculate_diagonal_pins(){
    chess::Color color = (white_to_move) ? chess::WHITE : chess::BLACK;
    chess::Color oppColor = (white_to_move) ? chess::BLACK : chess::WHITE;

    chess::Square friendly_king_square = (color == chess::WHITE) ? white_king_sq : black_king_sq;

    uint64_t entire_bitboard = occupied;
    uint64_t friendly_bitboard = (color == chess::WHITE) ? white_occupied : black_occupied;

    uint64_t opp_diag_piece_bitboard = bitboard[chess::make_piece(oppColor, chess::BISHOP)] | bitboard[chess::make_piece(oppColor, chess::QUEEN)];

    //Placing diagonal piece at king square to remove irrelevant diagonal pieces
    opp_diag_piece_bitboard &= chess::get_diagonal_slider_attacks(friendly_king_square, 0);  

    while(opp_diag_piece_bitboard)
    {
        chess::Square opp_diag_piece_sq = util::pop_lsb(opp_diag_piece_bitboard);
        
        uint64_t line_between = get_line_between(friendly_king_square, opp_diag_piece_sq);
        
        uint64_t pieces_on_line = line_between & entire_bitboard;

        // The condition for a pin is that there is EXACTLY ONE piece on this line.
        // And that piece must be friendly.
        if (util::count_bits(pieces_on_line) == 1 && (pieces_on_line & friendly_bitboard)) 
        {
            uint64_t pinned_piece_bb = pieces_on_line; // This is the bitboard of the pinned piece.

            pin_bitboard |= pinned_piece_bb;
            chess::Square piece_square = util::lsb(pinned_piece_bb);

            // Your pin ray calculation is correct.
            // pinRays[piece_square] = (util::get_line_between(friendly_king_square, opp_diag_piece_sq) | util::create_bitboard_from_square(opp_diag_piece_sq));
            pinRays[piece_square] = get_ray_between(friendly_king_square, opp_diag_piece_sq);

        }
        else if(util::count_bits(pieces_on_line) == 0) { 
            checker_bitboard |= (util::create_bitboard_from_square(opp_diag_piece_sq));
            checkRay |= get_ray_between(friendly_king_square, opp_diag_piece_sq);
        }
    }
}

void Board::calculate_pins()
{
    pin_bitboard = 0;
    for(int i = 0; i < 64; ++i) pinRays[i] = 0;
    checkRay = 0;
    checker_bitboard = 0;
    inDoubleCheck = false;    

    calculate_orthogonal_pins();
    calculate_diagonal_pins();

    const chess::Color oppColor = white_to_move ? chess::BLACK : chess::WHITE;
    const chess::Square king_sq = (white_to_move) ? white_king_sq : black_king_sq;
    checker_bitboard |= chess::KnightAttacks[king_sq] & bitboard[chess::make_piece(oppColor, chess::KNIGHT)];
    checker_bitboard |= chess::PawnAttacks[1-oppColor][king_sq] & bitboard[chess::make_piece(oppColor, chess::PAWN)];
    checkRay |= checker_bitboard;
    inDoubleCheck = (util::count_bits(checker_bitboard) > 1);
}