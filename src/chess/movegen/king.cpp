#include "chess/movegen.h"

void generate_king_moves_no_castle(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly){
    const chess::Color color = B.white_to_move ? chess::WHITE : chess::BLACK;
    uint64_t kingBitboard = B.bitboard[chess::make_piece(color, chess::KING)];
    while (kingBitboard){
        const chess::Square currKingSquare = util::pop_lsb(kingBitboard);
        uint64_t attacks = chess::KingAttacks[currKingSquare];
        uint64_t quietMoves = attacks & (~B.occupied);

        if(!capturesOnly)
        {
            while (quietMoves){
                const chess::Square destinationKingSquare = util::pop_lsb(quietMoves);
                if (B.square_attacked(destinationKingSquare, color)) continue;
                chess::Move m(currKingSquare, destinationKingSquare, chess::FLAG_QUIET, chess::NO_PIECE);
                moveList.push_back(m);
            }
        }

        uint64_t captures = (attacks & (color ? B.white_occupied : B.black_occupied));

        while (captures){
            const chess::Square destinationKingSquare = util::pop_lsb(captures);
            if (B.square_attacked(destinationKingSquare, color)) continue;
            chess::Move m(currKingSquare, destinationKingSquare, chess::FLAG_CAPTURE, chess::NO_PIECE);
            moveList.push_back(m);
        }
    }
}

void generate_king_moves_castle(const Board& B, std::vector<chess::Move>& moveList) {
    const chess::Color color = B.white_to_move ? chess::WHITE : chess::BLACK;

    const chess::Square king_start_sq = (color == chess::WHITE) ? chess::E1 : chess::E8;
    
    const chess::CastlingRights kside_right = (color == chess::WHITE) ? chess::WHITE_KINGSIDE : chess::BLACK_KINGSIDE;
    const uint64_t kside_empty_mask = (color == chess::WHITE) ? util::whiteKingSideBitboard : util::blackKingSideBitboard;
    const chess::Square kside_transit_sq = (color == chess::WHITE) ? chess::F1 : chess::F8;
    const chess::Square kside_dest_sq = (color == chess::WHITE) ? chess::G1 : chess::G8;

    const chess::CastlingRights qside_right = (color == chess::WHITE) ? chess::WHITE_QUEENSIDE : chess::BLACK_QUEENSIDE;
    const uint64_t qside_empty_mask = (color == chess::WHITE) ? util::whiteQueenSideBitboard : util::blackQueenSideBitboard;
    const chess::Square qside_transit_sq1 = (color == chess::WHITE) ? chess::D1 : chess::D8;
    const chess::Square qside_transit_sq2 = (color == chess::WHITE) ? chess::C1 : chess::C8; 

    //You can not castle out of check
    if (B.square_attacked(king_start_sq, color)) {
        return;
    }
    
    // Kingside Castle
    if ((B.castle_rights & kside_right) && ((B.occupied & kside_empty_mask) == 0)) {
        if (!B.square_attacked(kside_transit_sq, color) && !B.square_attacked(kside_dest_sq, color)) {
            moveList.push_back(chess::Move(king_start_sq, kside_dest_sq, chess::FLAG_CASTLE, chess::NO_PIECE));
        }
    }

    // Queenside Castle
    if ((B.castle_rights & qside_right) && ((B.occupied & qside_empty_mask) == 0)) {
        if (!B.square_attacked(qside_transit_sq1, color) && !B.square_attacked(qside_transit_sq2, color)) {
            moveList.push_back(chess::Move(king_start_sq, qside_transit_sq2, chess::FLAG_CASTLE, chess::NO_PIECE));
        }
    }
}

void MoveGen::generate_king_moves(const Board& B, std::vector<chess::Move>& moveList, bool capturesOnly){
    generate_king_moves_no_castle(B,moveList,capturesOnly);
    if(!capturesOnly) generate_king_moves_castle(B,moveList);
}
