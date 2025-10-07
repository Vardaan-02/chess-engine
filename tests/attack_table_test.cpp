// compile using : g++ -I../include/chess -o attack_table_test.out attack_table_test.cpp ../src/chess/bitboard.cpp

#include <bits/stdc++.h>
#include "../include/chess/bitboard.h"

using namespace std;

int main(){
    chess::init();

    // White Pawns
    // for (int i = chess::A1 ; i <= chess::H8 ; i++){
    //     chess::print_bitboard(chess::PawnAttacks[0][i]);
    // }

    // Black Pawns
    // for (int i = chess::A1 ; i <= chess::H8 ; i++){
    //     chess::print_bitboard(chess::PawnAttacks[1][i]);
    // }

    // King
    // for (int i = chess::A1 ; i <= chess::H8 ; i++){
    //     chess::print_bitboard(chess::KingAttacks[i]);
    // }

    // Knight
    // for (int i = chess::A1 ; i <= chess::H8 ; i++){
    //     chess::print_bitboard(chess::KnightAttacks[i]);
    // }

    return 0;
}