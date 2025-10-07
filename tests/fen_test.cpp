// Compile using this :: g++ -I../include/chess -o fen_test.out fen_test.cpp ../src/chess/board.cpp ../src/chess/bitboard.cpp 

#include <iostream>
#include <vector>
#include <string>
#include "../include/chess/board.h"

int main() {
    Board board;

    // List of FENs to test (starting positions + tactical/combinational positions)
    std::vector<std::string> fens = {
        // Existing basic positions
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r1bq1rk1/ppp2ppp/2n2n2/3pp3/3P4/2P1PN2/PP3PPP/RNBQ1RK1 w - d6 0 8",
        "8/8/8/3k4/8/8/3K4/8 w - - 0 45",
        "4k3/1P6/8/8/8/8/5K2/8 w - - 0 55",
        "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPP2PPP/RNBQKBNR b KQkq e6 0 3",
        "r3k2r/pp1b1ppp/2n1pn2/3p4/3P4/2N1PN2/PP3PPP/R3K2R w KQkq - 4 15",
        "8/8/3n4/3b4/3N4/3B4/8/8 b - - 10 20",

        // Tactical/combinational positions you provided
        "2rqk2r/pb1nbp1p/4p1p1/1B1n4/Np1N4/7Q/PP3PPP/R1B1R1K1 w kq - 0 1",
        "r1bq1rk1/3nbppp/p2pp3/6PQ/1p1BP2P/2NB4/PPP2P2/2KR3R w - - 0 1",
        "2kr4/ppq2pp1/2b1pn2/2P4r/2P5/3BQN1P/P4PP1/R4RK1 b - - 0 1",
        "r1bqr1k1/pp1n1ppp/5b2/4N1B1/3p3P/8/PPPQ1PP1/2K1RB1R w - - 0 1",
        "3r4/2r5/p3nkp1/1p3p2/1P1pbP2/P2B3R/2PRN1P1/6K1 b - - 0 1",
        "6k1/p3b1np/6pr/6P1/1B2p2Q/K7/7P/8 w - - 0 1",
        "3b4/p3P1q1/P1n2pr1/4p3/2B1n1Pk/1P1R4/P1p3KN/1N6 w - - 0 1",
        "7r/8/pB1p1R2/4k2q/1p6/1Pr5/P5Q1/6K1 w - - 0 1",
        "3r1r1k/1b4pp/ppn1p3/4Pp1R/Pn5P/3P4/4QP2/1qB1NKR1 w - - 0 1",
        "1k2r2r/pbb2p2/2qn2p1/8/PP6/2P2N2/1Q2NPB1/R4RK1 b - - 0 1",
        "r6k/6R1/p4p1p/2p2P1P/1pq1PN2/6P1/1PP5/2KR4 w - - 0 1",
        "3rr1k1/1b1q1p1p/p2b1npB/2pP4/1p2n3/4N1P1/PPQ1NPBP/R2R2K1 b - - 0 1",
        "7Q/kp3ppp/8/8/q3p3/P5PP/5b2/R1R3K1 w - - 0 1",
        "r5k1/1bbq1ppp/p6r/P1Nnp3/1p1B4/5B2/1PPQ2PP/R4R1K b - - 0 1",
        "1k4r1/3r1p1q/1p2pQpP/pPp1P1B1/P1Pp2P1/3P1R2/2n1KR2/8 w - - 0 1",
        "1r3kr1/2p2p2/2Q4p/1NPPbp2/4p2q/6P1/5P1P/3RR1K1 w - - 0 1",
        "r1b1kb1r/1p1n1ppp/p2ppn2/6BB/2qNP3/2N5/PPP2PPP/R2Q1RK1 w kq - 0 1",
        "r1bqk2r/pp4p1/2pb1n2/3p1p2/2PPp3/PP2P1P1/3N2BP/RN1QR1K1 b kq - 0 1",
        "r3r1k1/1b2qppp/pp1b4/3p1BPP/2pPn3/2N1PN2/PPQ2P2/2K3RR w - - 0 1",
        "r1bq1rk1/1p2bppp/p3p3/n3P3/4N3/1P1P1N2/PB4PP/R3QR1K w - - 0 1",
        "r4r1k/2qbnpbp/p2p2p1/nppPp1N1/4P1PP/2P1N3/PPBBQP1K/R5R1 w - - 0 1",
        "1rr3k1/2q1ppb1/p2pbnp1/6Q1/1p1BP1P1/P1N2P2/1PP4R/2K2B1R w - - 0 1"
    };

    bool allPassed = true;

    for (size_t i = 0; i < fens.size(); i++) {
        board.set_fen(fens[i]);
        std::string resultFEN = board.to_fen();

        if (resultFEN == fens[i]) {
            // add for passed if needed
        } else {
            std::cout << "Test " << i + 1 << ": FAILED" << std::endl;
            allPassed = false;
        }
    }

    if (allPassed) {
        std::cout << "All FEN tests PASSED!" << std::endl;
    } else {
        std::cout << "Some FEN tests FAILED!" << std::endl;
    }

    return 0;
}