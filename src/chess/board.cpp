#include "chess/board.h"
#include "chess/bitboard.h"
#include "chess/zobrist.h"
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <iostream>

// ----------------- Constructor -----------------
Board::Board() {
    clear();
}

// ----------------- Clear board -----------------
void Board::clear() {
    std::memset(bitboard, 0, sizeof(bitboard));
    std::memset(board_array, 0, sizeof(board_array));
    white_to_move = true;
    castle_rights = chess::NO_CASTLING;
    en_passant_sq = chess::SQUARE_NONE;
    halfmove_clock = 0;
    fullmove_number = 1;
    pinned = 0ULL;
    checks = 0ULL;
    check_mask = 0ULL;
    double_check = false;
    white_king_sq = black_king_sq = chess::SQUARE_NONE;
    zobrist_key = zobrist_pawn_key = 0;
    material_white = material_black = 0;
    white_occupied = black_occupied = occupied = 0;
    undo_stack.clear();
}

// ----------------- FEN parsing -----------------
void Board::set_fen(std::string &fen_cstr) {
    clear();
    std::string fen(fen_cstr);
    std::istringstream iss(fen);
    std::string board_part, side_part, castle_part, ep_part;
    int fullmove = 1;

    iss >> board_part >> side_part >> castle_part >> ep_part >> halfmove_clock >> fullmove;
    
    int sq = chess::A8; // start at A8
    for (char c : board_part) {
        if (c == '/') { sq -= 16; continue; }
        if (std::isdigit(c)) { sq += (c - '0'); continue; }
        chess::Piece piece = chess::NO_PIECE;
        switch(c) {
            case 'P': piece = chess::WP; break;
            case 'N': piece = chess::WN; break;
            case 'B': piece = chess::WB; break;
            case 'R': piece = chess::WR; break;
            case 'Q': piece = chess::WQ; break;
            case 'K': piece = chess::WK; break;
            case 'p': piece = chess::BP; break;
            case 'n': piece = chess::BN; break;
            case 'b': piece = chess::BB; break;
            case 'r': piece = chess::BR; break;
            case 'q': piece = chess::BQ; break;
            case 'k': piece = chess::BK; break;
            default: throw std::runtime_error("Invalid FEN char");
        }
        util::set_bit(bitboard[piece], (chess::Square)sq);
        board_array[sq] = piece;
        sq++;
    }

    white_to_move = (side_part == "w");
    
    castle_rights = chess::NO_CASTLING;
    for (char c : castle_part) {
        switch(c) {
            case 'K': castle_rights |= chess::WHITE_KINGSIDE; break;
            case 'Q': castle_rights |= chess::WHITE_QUEENSIDE; break;
            case 'k': castle_rights |= chess::BLACK_KINGSIDE; break;
            case 'q': castle_rights |= chess::BLACK_QUEENSIDE; break;
        }
    }

    en_passant_sq = chess::SQUARE_NONE;

    if(ep_part != "-")
    {
        int file = ep_part[0] - 'a';
        int rank = ep_part[1] - '1';
        en_passant_sq = get_square_from_rank_file(rank, file);
    }

    fullmove_number = fullmove;
    update_king_squares_from_bitboards();
    update_occupancies();
    update_game_phase();
    compute_pins_and_checks();
    zobrist_key = Zobrist::calculate_zobrist_hash(*this);
}

// ----------------- FEN serialization -----------------
std::string Board::to_fen() const {
    std::string fen;
    for (int rank = 7; rank >= 0; --rank) {
        int empty = 0;
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            int p = board_array[sq];
            if (p == chess::NO_PIECE) { empty++; }
            else {
                if (empty) { fen += ('0' + empty); empty = 0; }
                switch(p) {
                    case chess::WP: fen += 'P'; break;
                    case chess::WN: fen += 'N'; break;
                    case chess::WB: fen += 'B'; break;
                    case chess::WR: fen += 'R'; break;
                    case chess::WQ: fen += 'Q'; break;
                    case chess::WK: fen += 'K'; break;
                    case chess::BP: fen += 'p'; break;
                    case chess::BN: fen += 'n'; break;
                    case chess::BB: fen += 'b'; break;
                    case chess::BR: fen += 'r'; break;
                    case chess::BQ: fen += 'q'; break;
                    case chess::BK: fen += 'k'; break;
                }
            }
        }
        if (empty) fen += ('0' + empty);
        if (rank != 0) fen += '/';
    }
    fen += ' ';
    fen += (white_to_move ? 'w' : 'b');
    fen += ' ';
    if (castle_rights == 0) fen += '-';
    else {
        if (castle_rights & chess::WHITE_KINGSIDE) fen += 'K';
        if (castle_rights & chess::WHITE_QUEENSIDE) fen += 'Q';
        if (castle_rights & chess::BLACK_KINGSIDE) fen += 'k';
        if (castle_rights & chess::BLACK_QUEENSIDE) fen += 'q';
    }
    fen += ' ';
    int file = en_passant_sq % 8;
    int rank = en_passant_sq / 8;
    fen += (en_passant_sq == chess::SQUARE_NONE ? "-" : (std::string(1, 'a' + file) + std::string(1, '1' + rank))); 
    fen += ' ';
    fen += std::to_string(halfmove_clock);
    fen += ' ';
    fen += std::to_string(fullmove_number);
    return fen;
}

void Board::print_board() const {
    std::cout << "\n    +------------------------+\n";

    // Loop ranks from top (8) to bottom (1)
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << " " << (rank + 1) << " | ";
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            int p = board_array[sq];
            char c = '.';

            switch (p) {
                case chess::WP: c = 'P'; break;
                case chess::WN: c = 'N'; break;
                case chess::WB: c = 'B'; break;
                case chess::WR: c = 'R'; break;
                case chess::WQ: c = 'Q'; break;
                case chess::WK: c = 'K'; break;
                case chess::BP: c = 'p'; break;
                case chess::BN: c = 'n'; break;
                case chess::BB: c = 'b'; break;
                case chess::BR: c = 'r'; break;
                case chess::BQ: c = 'q'; break;
                case chess::BK: c = 'k'; break;
                default: break;
            }

            std::cout << c << ' ';
        }
        std::cout << "|\n";
    }

    std::cout << "    +------------------------+\n";
    std::cout << "      a b c d e f g h\n\n";

    // Extra board state context — clutch for debugging
    std::cout << "Side to move: " << (white_to_move ? "White" : "Black") << "\n";

    std::cout << "Castling rights: ";
    if (castle_rights == 0) std::cout << "-";
    else {
        if (castle_rights & chess::WHITE_KINGSIDE)  std::cout << "K";
        if (castle_rights & chess::WHITE_QUEENSIDE) std::cout << "Q";
        if (castle_rights & chess::BLACK_KINGSIDE)  std::cout << "k";
        if (castle_rights & chess::BLACK_QUEENSIDE) std::cout << "q";
    }
    std::cout << "\n";

    std::cout << "En passant: ";
    if (en_passant_sq == chess::SQUARE_NONE) std::cout << "-";
    else {
        int file = en_passant_sq % 8;
        int rank = en_passant_sq / 8;
        std::cout << static_cast<char>('a' + file) << static_cast<char>('1' + rank);
    }
    std::cout << "\n";

    std::cout << "Halfmove clock: " << halfmove_clock << "\n";
    std::cout << "Fullmove number: " << fullmove_number << "\n";
    std::cout << "Zobrist key: 0x" << std::hex << zobrist_key << std::dec << "\n";
    std::cout << "Game Phase: " << game_phase << "\n";
    std::cout << "Material (W/B): " << material_white << " / " << material_black << "\n\n";
}

//-----------------------------------------------------------------------------
// MAKE MOVE
//-----------------------------------------------------------------------------
void Board::make_move(const chess::Move &mv) {
    // 1. Save current state for unmaking the move later
    chess::Undo undo;
    undo.prev_castle_rights = castle_rights;
    undo.prev_en_passant_sq = en_passant_sq;
    undo.captured_piece_and_halfmove = (halfmove_clock << 4) | chess::NO_PIECE;
    undo.check_mask = check_mask;
    undo.checks  = checks;
    undo.pinned = pinned;
    undo.double_check = double_check;
    undo.zobrist_before = zobrist_key;
    undo.game_phase = game_phase;

    // 2. Extract move details
    const chess::Square from = (chess::Square)mv.from();
    const chess::Square to = (chess::Square)mv.to();
    const uint16_t flags = mv.flags();
    
    const chess::Piece moving_piece = (chess::Piece)board_array[from];  //remove the moved piece

    zobrist_key ^= Zobrist::piecesArray[moving_piece][from];

    chess::Piece captured_piece = (flags & chess::FLAG_EP) 
        ? (white_to_move ? chess::BP : chess::WP)
        : (chess::Piece)board_array[to];

    if(captured_piece != chess::NO_PIECE){ 
        if(flags & chess::FLAG_EP){
            zobrist_key ^= Zobrist::piecesArray[captured_piece][white_to_move ? to - 8 : to + 8];  //remove captured piece
        }
        else zobrist_key ^= Zobrist::piecesArray[captured_piece][to];  //remove captured piece
    }
    zobrist_key ^= Zobrist::piecesArray[moving_piece][to];  //add the moved piece

    // Reset halfmove clock if it's a pawn move or capture
    if (chess::type_of(moving_piece) == chess::PAWN || captured_piece != chess::NO_PIECE) {
        halfmove_clock = 0;
    } else {
        halfmove_clock++;
    }

    // Update captured piece in undo info
    if (captured_piece != chess::NO_PIECE) {
        undo.captured_piece_and_halfmove = (undo.captured_piece_and_halfmove & 0xFFF0) | captured_piece;
    }

    // Clear en passant square
    en_passant_sq = chess::SQUARE_NONE;
    
    // 3. Handle move types
    if (flags == chess::FLAG_QUIET) {
        move_piece_bb(moving_piece, from, to);
    } 
    else if (flags == chess::FLAG_CAPTURE) {
        util::pop_bit(bitboard[captured_piece], to);
        move_piece_bb(moving_piece, from, to);
    }
    else if (flags & chess::FLAG_PROMO) {
        const chess::Piece promo_piece = (chess::Piece)mv.promo();
        // Remove the pawn
        util::pop_bit(bitboard[moving_piece], from);
        board_array[from] = chess::NO_PIECE;
        // If it was a capture, remove the captured piece
        if (flags & chess::FLAG_CAPTURE) {
            util::pop_bit(bitboard[captured_piece], to);
        }
        // Place the new piece
        util::set_bit(bitboard[promo_piece], to);
        board_array[to] = promo_piece;

        zobrist_key ^= Zobrist::piecesArray[moving_piece][to]; //since the pawn is no longer at the destination square
        zobrist_key ^= Zobrist::piecesArray[promo_piece][to];
    }
    else if (flags == chess::FLAG_EP) {
        move_piece_bb(moving_piece, from, to);
        const chess::Square captured_pawn_sq = white_to_move ? (chess::Square)(to - 8) : (chess::Square)(to + 8);
        util::pop_bit(bitboard[captured_piece], captured_pawn_sq);
        board_array[captured_pawn_sq] = chess::NO_PIECE;
    }
    else if (flags == chess::FLAG_CASTLE) {
        move_piece_bb(moving_piece, from, to); // Move king
        chess::Square rook_from, rook_to;
        if (to == chess::G1) { rook_from = chess::H1; rook_to = chess::F1; }
        else if (to == chess::C1) { rook_from = chess::A1; rook_to = chess::D1; }
        else if (to == chess::G8) { rook_from = chess::H8; rook_to = chess::F8; }
        else /* (to == C8) */ { rook_from = chess::A8; rook_to = chess::D8; }
        move_piece_bb((chess::Piece)board_array[rook_from], rook_from, rook_to);

        zobrist_key ^= Zobrist::piecesArray[chess::make_piece(white_to_move ? chess::WHITE : chess::BLACK,chess::ROOK)][rook_from];
        zobrist_key ^= Zobrist::piecesArray[chess::make_piece(white_to_move ? chess::WHITE : chess::BLACK,chess::ROOK)][rook_to];
    }
    // Handle pawn double push to set en passant square
    else if (flags == chess::FLAG_DOUBLE_PUSH) {
        move_piece_bb(moving_piece, from, to);
        en_passant_sq = white_to_move ? (chess::Square)(from + 8) : (chess::Square)(from - 8);
    }

    // 4. Update castling rights with simple if statements
    if (moving_piece == chess::WK) {
        castle_rights &= chess::CastlingRights(~chess::WHITE_CASTLING);
    } else if (moving_piece == chess::BK) {
        castle_rights &= chess::CastlingRights(~chess::BLACK_CASTLING);
    }

    if (from == chess::A1 || to == chess::A1) {
        castle_rights &= chess::CastlingRights(~chess::WHITE_QUEENSIDE);
    }
    if (from == chess::H1 || to == chess::H1) {
        castle_rights &= chess::CastlingRights(~chess::WHITE_KINGSIDE);
    }
    if (from == chess::A8 || to == chess::A8) {
        castle_rights &= chess::CastlingRights(~chess::BLACK_QUEENSIDE);
    }
    if (from == chess::H8 || to == chess::H8) {
        castle_rights &= chess::CastlingRights(~chess::BLACK_KINGSIDE);
    }

    zobrist_key ^= Zobrist::castlingRights[undo.prev_castle_rights];
    zobrist_key ^= Zobrist::castlingRights[castle_rights];
    
    if(undo.prev_en_passant_sq != chess::SQUARE_NONE)
    {
       zobrist_key ^= Zobrist::enPassantFile[undo.prev_en_passant_sq];
    }
    if (en_passant_sq != chess::SQUARE_NONE) {
        zobrist_key ^= Zobrist::enPassantFile[en_passant_sq];
    }
    
    // 5. Update king square if it moved
    if (moving_piece == chess::WK) white_king_sq = to;
    if (moving_piece == chess::BK) black_king_sq = to;

    // 6. Update fullmove number and switch side
    if (!white_to_move) fullmove_number++;
    white_to_move = !white_to_move;

    zobrist_key ^= Zobrist::sideToMove; //Always flip this

    // 7. Update combined bitboards
    update_occupancies();
    update_game_phase();
    compute_pins_and_checks();
    zobrist_key = Zobrist::calculate_zobrist_hash(*this);

    // 8. Push state to undo stack
    undo_stack.push_back(undo);
}


//-----------------------------------------------------------------------------
// UNMAKE MOVE
//-----------------------------------------------------------------------------
void Board::unmake_move(const chess::Move &mv) {
    // 1. Pop the last state from the undo stack
    chess::Undo undo = undo_stack.back();
    undo_stack.pop_back();

    // 2. Extract move details
    const chess::Square from = (chess::Square)mv.from();
    const chess::Square to = (chess::Square)mv.to();
    const uint16_t flags = mv.flags();

    // Restore board state from undo info
    castle_rights = (chess::CastlingRights)undo.prev_castle_rights;
    en_passant_sq = (chess::Square)undo.prev_en_passant_sq;
    halfmove_clock = undo.captured_piece_and_halfmove >> 4;
    check_mask = undo.check_mask;
    checks  = undo.checks;
    pinned = undo.pinned;
    double_check = undo.double_check;
    zobrist_key = undo.zobrist_before;
    game_phase = undo.game_phase;

    // Switch side back
    white_to_move = !white_to_move;
    if (!white_to_move) fullmove_number--;

    chess::Piece moving_piece = (chess::Piece)board_array[to];
    const chess::Piece captured_piece = (chess::Piece)(undo.captured_piece_and_halfmove & 0xF);

    // 3. Handle move types in reverse
    if (flags & chess::FLAG_PROMO) {
        moving_piece = white_to_move ? chess::WP : chess::BP; // It was a pawn before promoting
        const chess::Piece promo_piece = (chess::Piece)mv.promo();
        
        // Remove promoted piece
        util::pop_bit(bitboard[promo_piece], to);
        // Place pawn back
        util::set_bit(bitboard[moving_piece], from);
        board_array[from] = moving_piece;
        board_array[to] = chess::NO_PIECE; // Clear the to square first
        
        // If it was a capture, restore the captured piece
        if (flags & chess::FLAG_CAPTURE) {
            util::set_bit(bitboard[captured_piece], to);
            board_array[to] = captured_piece;
        }
    }
    else if (flags == chess::FLAG_QUIET || flags == chess::FLAG_DOUBLE_PUSH) {
        restore_piece_bb(moving_piece, from, to);
    }
    else if (flags == chess::FLAG_CAPTURE) {
        restore_piece_bb(moving_piece, from, to);
        // Restore the captured piece
        util::set_bit(bitboard[captured_piece], to);
        board_array[to] = captured_piece;
    }
    else if (flags == chess::FLAG_EP) {
        restore_piece_bb(moving_piece, from, to);
        const chess::Square captured_pawn_sq = white_to_move ? (chess::Square)(to - 8) : (chess::Square)(to + 8);
        // Restore captured pawn
        util::set_bit(bitboard[captured_piece], captured_pawn_sq);
        board_array[captured_pawn_sq] = captured_piece;
    }
    else if (flags == chess::FLAG_CASTLE) {
        restore_piece_bb(moving_piece, from, to); // Move king back
        chess::Square rook_from, rook_to;
        if (to == chess::G1) { rook_from = chess::H1; rook_to = chess::F1; }
        else if (to == chess::C1) { rook_from = chess::A1; rook_to = chess::D1; }
        else if (to == chess::G8) { rook_from = chess::H8; rook_to = chess::F8; }
        else /* (to == C8) */ { rook_from = chess::A8; rook_to = chess::D8; }
        // Note: rook_from/rook_to are from the perspective of make_move
        restore_piece_bb((chess::Piece)board_array[rook_to], rook_from, rook_to);
    }

    // 4. Update king square if it moved
    if (moving_piece == chess::WK) white_king_sq = from;
    if (moving_piece == chess::BK) black_king_sq = from;

    // 5. Update combined bitboards
    update_occupancies();
}

bool Board::square_attacked(chess::Square sq, bool by_white) const{
    
    chess::Color attackerColor = (by_white) ? chess::WHITE : chess::BLACK;
    // 1. Pawns
    if (chess::PawnAttacks[by_white][sq] & bitboard[chess::make_piece(attackerColor, chess::PAWN)]) return true;

    // 2. Knight
    if (chess::KnightAttacks[sq] & bitboard[chess::make_piece(attackerColor, chess::KNIGHT)]) return true;

    // 3. King
    if (chess::KingAttacks[sq] & bitboard[chess::make_piece(attackerColor, chess::KING)]) return true;

    uint64_t orthogonal_pieces = bitboard[chess::make_piece(attackerColor, chess::ROOK)] | bitboard[chess::make_piece(attackerColor, chess::QUEEN)];
    uint64_t diagonal_pieces = bitboard[chess::make_piece(attackerColor, chess::BISHOP)] | bitboard[chess::make_piece(attackerColor, chess::QUEEN)];

    // 4. Orthogonal Sliders
    if (chess::get_orthogonal_slider_attacks(sq, occupied) & orthogonal_pieces) return true;

    // 5. Diagnol Sliders
    if (chess::get_diagonal_slider_attacks(sq, occupied) & diagonal_pieces) return true;

    return false;
}

chess::PieceType Board::get_least_value_attacking_piece_type_on_sq(chess::Square sq, bool by_white) const {
    
    chess::Color attackerColor = (by_white) ? chess::WHITE : chess::BLACK;
    // 1. Pawns
    if (chess::PawnAttacks[by_white][sq] & bitboard[chess::make_piece(attackerColor, chess::PAWN)]) return chess::PAWN;

    // 2. Knight
    if (chess::KnightAttacks[sq] & bitboard[chess::make_piece(attackerColor, chess::KNIGHT)]) return chess::KNIGHT;

    // 3. Bishop
    uint64_t diagonal_pieces = bitboard[chess::make_piece(attackerColor, chess::BISHOP)];
    if (chess::get_diagonal_slider_attacks(sq, occupied) & diagonal_pieces) return chess::BISHOP;

    // 4. Rook
    uint64_t orthogonal_pieces = bitboard[chess::make_piece(attackerColor, chess::ROOK)];
    if (chess::get_orthogonal_slider_attacks(sq, occupied) & orthogonal_pieces) return chess::ROOK;
    
    // 5. Queen
    uint64_t queen = bitboard[chess::make_piece(attackerColor, chess::QUEEN)];
    if (chess::get_diagonal_slider_attacks(sq, occupied) & queen) return chess::QUEEN;
    if (chess::get_orthogonal_slider_attacks(sq, occupied) & queen) return chess::QUEEN;

    // 6. King
    if (chess::KingAttacks[sq] & bitboard[chess::make_piece(attackerColor, chess::KING)]) return chess::KING;

    return chess::NO_PIECE_TYPE;
}

uint64_t Board::attackers_to(chess::Square sq, bool by_white) const {
    uint64_t attackers_bitboard = 0;
    chess::Color attackerColor = by_white ? chess::WHITE : chess::BLACK;

    // 1. Pawns
    attackers_bitboard |= chess::PawnAttacks[by_white][sq] & bitboard[chess::make_piece(attackerColor, chess::PAWN)];

    // 2. Knights
    attackers_bitboard |= chess::KnightAttacks[sq] & bitboard[chess::make_piece(attackerColor, chess::KNIGHT)];

    // 3. King
    attackers_bitboard |= chess::KingAttacks[sq] & bitboard[chess::make_piece(attackerColor, chess::KING)];

    // Define bitboards for the attacker's sliding pieces
    uint64_t orthogonal_pieces = bitboard[chess::make_piece(attackerColor, chess::ROOK)] | bitboard[chess::make_piece(attackerColor, chess::QUEEN)];
    uint64_t diagonal_pieces = bitboard[chess::make_piece(attackerColor, chess::BISHOP)] | bitboard[chess::make_piece(attackerColor, chess::QUEEN)];

    // 4. Orthogonal Sliders (Rooks & Queens)
    attackers_bitboard |= chess::get_orthogonal_slider_attacks(sq, occupied) & orthogonal_pieces;

    // 5. Diagonal Sliders (Bishops & Queens)
    attackers_bitboard |= chess::get_diagonal_slider_attacks(sq, occupied) & diagonal_pieces;

    return attackers_bitboard;
}

bool Board::is_position_legal()
{
      chess::Square king_sq = white_to_move
            ? black_king_sq 
            : white_king_sq;

    return util::count_bits(square_attacked(king_sq, white_to_move)) == 0;
}