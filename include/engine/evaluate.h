#pragma once
#include "chess/board.h"
#include "chess/util.h"
#include "chess/types.h"
#include <array>

namespace eval {
// Best Practice 1: Create a structure for values that change
// between the middlegame (mg) and endgame (eg). This is the
// core concept of "Tapered Evaluation".
struct TaperedScore {
    int mg = 0;
    int eg = 0;
};

// Best Practice 2: Encapsulate all evaluation parameters into a single,
// comprehensive structure. This makes the entire configuration a single object.
struct EvalData {

    // Material values for each piece type
    std::array<TaperedScore, chess::PIECE_TYPE_NB> material_values;

    // Piece-Square Tables (PSTs)
    // Indexed by [PieceType][Square]
    std::array<std::array<TaperedScore, 64>, chess::PIECE_TYPE_NB> psts;

    // Specific bonuses and penalties
    TaperedScore bishop_pair_bonus;
    TaperedScore rook_on_open_file_bonus;
    TaperedScore rook_on_semi_open_file_bonus;
    TaperedScore rook_on_7th_bonus;
    TaperedScore knight_outpost_bonus;
    TaperedScore bishop_center_control;
    TaperedScore good_bishop_bonus;
    TaperedScore controlled_square_bonus;
    std::array<std::array<TaperedScore, 32>, chess::PIECE_TYPE_NB> mobility_bonus;
    TaperedScore doubled_pawn_penalty;
    TaperedScore isolated_pawn_penalty;
    TaperedScore backward_pawn_penalty;
    std::array<TaperedScore, 8> pawn_chain_bonus;
    std::array<TaperedScore, 8> passed_pawn_bonus; // This might be an array per rank later
    TaperedScore passed_pawn_supported_bonus;
    TaperedScore passed_pawn_blocked_penalty;
    TaperedScore king_distance_from_center_penalty;
    TaperedScore opponent_king_distance_from_center_bonus;
    TaperedScore king_distance_from_opponent_king_penalty;
    TaperedScore pawn_majority_bonus;  // On one wing, potential for passed pawns
    TaperedScore rook_connected_bonus; // Rooks on same rank/file

    // King Safety Parameters

    // Penalty for pawns being too far from the king.
    std::array<TaperedScore, 3> pawn_shield_penalty;

    // Penalty for an open file in front of the king (pawn is missing)
    TaperedScore open_file_penalty;

    // Weights for pieces attacking the king zone.
    // Indexed by PieceType. PAWN, KNIGHT, BISHOP, ROOK, QUEEN.
    std::array<int, chess::PIECE_TYPE_NB> king_attack_weights;

    // A lookup table to convert the raw "attack score" into a final penalty.
    // The more attackers, the penalty increases exponentially.
    std::array<TaperedScore, 100> king_safety_table;

    

    std::array<uint64_t, 8> adjacent_files_masks;
};

// Best Practice 3: Create a single, compile-time constant instance of the configuration.
// This ensures all values are in one place and initialized safely.
constexpr EvalData eval_data = {
    // 1. Material Values
    .material_values = {{
        {0  , 0   },  // NO_PIECE_TYPE
        {80 , 100 },  // PAWN
        {320, 320 },  // KNIGHT
        {330, 360 },  // BISHOP
        {500, 600 },  // ROOK
        {900, 1000},  // QUEEN
        {0  , 0   }   // KING (material value is infinite/not counted)
    }},

    // 2. Piece-Square Tables (PSTs)
    .psts = {{
        // No Piece PST
        {{ //     A          B           C           D           E           F           G           H
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, // Rank 1
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, // Rank 2
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, // Rank 3
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, // Rank 4
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, // Rank 5
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, // Rank 6
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, // Rank 7
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}  // Rank 8
        }},
        // PAWN PST
        {{ //     A          B           C           D           E           F           G           H
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, // Rank 1
            {  5,  10}, { 10,  10}, { 10,  10}, {-20,  10}, {-20,  10}, { 10,  10}, { 10,  10}, {  5,  10}, // Rank 2
            { 15,  10}, {  5,  10}, {-10,  10}, {  0,  10}, {  0,  10}, {-10,  10}, {  5,  10}, { 15,  10}, // Rank 3
            {  0,  20}, {  0,  20}, {  0,  20}, { 20,  20}, { 20,  20}, {  0,  20}, {  0,  20}, {  0,  20}, // Rank 4
            {  5,  30}, {  5,  30}, { 10,  30}, { 25,  30}, { 25,  30}, { 10,  30}, {  5,  30}, {  5,  30}, // Rank 5
            { 10,  50}, { 10,  50}, { 20,  50}, { 30,  50}, { 30,  50}, { 20,  50}, { 10,  50}, { 10,  50}, // Rank 6
            { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, // Rank 7
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}  // Rank 8
        }},
        // KNIGHT PST
        {{ //     A          B           C           D           E           F           G           H
            {-50, -50}, {-40, -30}, {-30, -20}, {-30, -20}, {-30, -20}, {-30, -20}, {-40, -30}, {-50, -50}, // Rank 1
            {-40, -30}, {-20, -10}, {  0,   0}, {  0,   5}, {  0,   5}, {  0,   0}, {-20, -10}, {-40, -30}, // Rank 2
            {-30, -20}, {  0,   0}, { 10,  10}, { 15,  15}, { 15,  15}, { 10,  10}, {  0,   0}, {-30, -20}, // Rank 3
            {-30, -20}, {  5,   5}, { 15,  15}, { 20,  20}, { 20,  20}, { 15,  15}, {  5,   5}, {-30, -20}, // Rank 4
            {-30, -20}, {  0,   5}, { 15,  15}, { 20,  20}, { 20,  20}, { 15,  15}, {  0,   5}, {-30, -20}, // Rank 5
            {-30, -20}, {  5,   0}, { 10,  10}, { 15,  15}, { 15,  15}, { 10,  10}, {  5,   0}, {-30, -20}, // Rank 6
            {-40, -30}, {-20, -10}, {  0,   0}, {  5,   5}, {  5,   5}, {  0,   0}, {-20, -10}, {-40, -30}, // Rank 7
            {-50, -50}, {-40, -30}, {-30, -20}, {-30, -20}, {-30, -20}, {-30, -20}, {-40, -30}, {-50, -50}  // Rank 8
        }},
        // BISHOP PST
        {{ //     A          B           C           D           E           F           G           H
            {-20, -20}, {-10, -10}, {-10, -10}, {-10, -10}, {-10, -10}, {-10, -10}, {-10, -10}, {-20, -20}, // Rank 1
            {-10, -10}, { 30,  12}, {  0,   5}, {  5,   5}, {  5,   5}, {  0,   5}, { 30,  12}, {-10, -10}, // Rank 2
            {-10, -10}, {  0,   5}, {  8,  10}, { 10,  10}, { 10,  10}, {  8,  10}, {  0,   5}, {-10, -10}, // Rank 3
            {-10, -10}, {  5,   5}, { 10,  10}, { 12,  12}, { 12,  12}, { 10,  10}, {  5,   5}, {-10, -10}, // Rank 4
            {-10, -10}, {  5,   5}, { 10,  10}, { 12,  12}, { 12,  12}, { 10,  10}, {  5,   5}, {-10, -10}, // Rank 5
            {-10, -10}, {  0,   5}, {  8,  10}, { 10,  10}, { 10,  10}, {  8,  10}, {  0,   5}, {-10, -10}, // Rank 6
            {-10, -10}, { 12,  12}, {  0,   5}, {  5,   5}, {  5,   5}, {  0,   5}, { 12,  12}, {-10, -10}, // Rank 7
            {-20, -20}, {-10, -10}, {-10, -10}, {-10, -10}, {-10, -10}, {-10, -10}, {-10, -10}, {-20, -20}  // Rank 8
        }},
        // ROOK PST
        {{ //     A          B           C           D           E           F           G           H
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, // Rank 1
            { -5,  10}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, { -5,  10}, // Rank 2
            { -5,  10}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, { -5,  10}, // Rank 3
            { -5,  10}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, { -5,  10}, // Rank 4
            { -5,  10}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, { -5,  10}, // Rank 5
            { -5,  10}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, { -5,  10}, // Rank 6
            {  5,  10}, { 10,  15}, { 10,  15}, { 10,  15}, { 10,  15}, { 10,  15}, { 10,  15}, {  5,  10}, // Rank 7
            {  0,   0}, {  0,   0}, {  0,   0}, {  5,   0}, {  5,   0}, {  0,   0}, {  0,   0}, {  0,   0}  // Rank 8
        }},
        // QUEEN PST
        {{ //     A          B           C           D           E           F           G           H
            {-20, -30}, {-10, -20}, {-10, -10}, { -5, -10}, { -5, -10}, {-10, -10}, {-10, -20}, {-20, -30}, // Rank 1
            {-10, -20}, {  0, -10}, {  0,   0}, {  0,   5}, {  0,   5}, {  0,   0}, {  0, -10}, {-10, -20}, // Rank 2
            {-10, -10}, {  0,   0}, {  5,  10}, {  5,  15}, {  5,  15}, {  5,  10}, {  0,   0}, {-10, -10}, // Rank 3
            { -5, -10}, {  0,   5}, {  5,  15}, {  5,  20}, {  5,  20}, {  5,  15}, {  0,   5}, { -5, -10}, // Rank 4
            {  0, -10}, {  0,   5}, {  5,  15}, {  5,  20}, {  5,  20}, {  5,  15}, {  0,   5}, { -5, -10}, // Rank 5
            {-10, -10}, {  5,   0}, {  5,  10}, {  5,  15}, {  5,  15}, {  5,  10}, {  0,   0}, {-10, -10}, // Rank 6
            {-10, -20}, {  0, -10}, {  5,   0}, {  0,   5}, {  0,   5}, {  0,   0}, {  0, -10}, {-10, -20}, // Rank 7
            {-20, -30}, {-10, -20}, {-10, -10}, { -5, -10}, { -5, -10}, {-10, -10}, {-10, -20}, {-20, -30}  // Rank 8
        }},
        // KING PST
        {{ //     A          B           C           D           E           F           G           H
            { 10, -50}, { 30, -30}, { 10, -20}, {  0, -10}, {  0, -10}, { 10, -20}, { 30, -30}, { 10, -50}, // Rank 1
            {  5, -30}, { 20, -10}, {  0,   0}, {  0,  10}, {  0,   0}, {  0,   0}, { 20, -10}, {  5, -30}, // Rank 2
            {-10,   0}, {-20,   0}, {-20,   0}, {-20,   0}, {-20,   0}, {-20,   0}, {-20,   0}, {-10,   0}, // Rank 3
            {-20,   0}, {-30,   0}, {-30,   0}, {-40,   0}, {-40,   0}, {-30,   0}, {-30,   0}, {-20,   0}, // Rank 4
            {-30,   0}, {-40,   0}, {-40,   0}, {-50,   0}, {-50,   0}, {-40,   0}, {-40,   0}, {-30,   0}, // Rank 5
            {-30,   0}, {-40,   0}, {-40,   0}, {-50,   0}, {-50,   0}, {-40,   0}, {-40,   0}, {-30,   0}, // Rank 6
            {-30,   0}, {-40,   0}, {-40,   0}, {-50,   0}, {-50,   0}, {-40,   0}, {-40,   0}, {-30,   0}, // Rank 7
            {-30,   0}, {-40,   0}, {-40,   0}, {-50,   0}, {-50,   0}, {-40,   0}, {-40,   0}, {-30,   0}  // Rank 8
        }}
    }},

    // 3. Bonuses and Penalties
    // --- STRUCTURAL & PIECE SYNERGY BONUSES ---
    .bishop_pair_bonus              = {45, 70},     // Midgame: strong on open diagonals; Endgame: more mobility, pair dominates
    .rook_on_open_file_bonus        = {70, 10},     // Rooks thrive on open/semi-open files
    .rook_on_semi_open_file_bonus   = {55, 40},     // Rooks thrive on open/semi-open files
    .rook_on_7th_bonus              = {30, 50},     // Rook on 7th rank dominates enemy pawns
    .knight_outpost_bonus           = {45, 35},     // Knight anchored on a safe outpost
    .bishop_center_control          = {15, 25},     // Centralized bishop, good diagonals
    .good_bishop_bonus              = {10, 2 },     // Penalty for each friendly pawn on the same color square
    .controlled_square_bonus        = {5 , 2 },     // Space Bonus

    .mobility_bonus = {{
        // No Piece
        {{
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}
        }},
        // Pawn (No mobility bonus)
        {{
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}
        }},
        // Knight (Max 8 moves)
        {{ // MG, EG
            {-40, -80}, {-25, -60}, {-10, -50}, {0,  -10}, { 15,  0}, { 25,  10}, { 25,  20}, { 25,  30},{ 25, 40}
        }},
        // Bishop (Max 13 moves)
        {{ // MG, EG
            {-48, -80}, {-20, -60}, { 15, -40}, { 25, -20}, { 35,  0}, 
            { 45,  15}, { 45,  35}, { 45,  35}, { 45, 35} , { 45,  35}, { 45,  35}, { 45,  35}, { 60,  35}, {60, 35}
        }},
        // Rook (Max 14 moves)
        {{ // MG, EG
            {-40, -100}, {-20, -80}, { 0, 60}, { 10, 40}, { 10, -20}, { 10, -10}, { 15, 0}, 
            { 35,  15}, { 35, 15},   { 35, 15}, { 35, 15}, { 35, 15}, { 35, 15}, { 50, 25}, { 50, 25}
        }},
        // Queen (Max 27 moves)
        {{ // MG, EG
            {-40, -40}, {-40, -36}, { -30,  -32}, { -30, -28}, { -10, -24}, { -10, -20}, { 0, -16}, { 0, -12},
            { 10,  -8}, { 10,  -4}, {  10,    0}, {  10,   4}, {  20,   8}, {  20,  12}, { 20,  16}, { 20, 20},
            { 25,  24}, { 25,  28}, {  25,   32}, {  25,  40}, {  25,  40}, {  25,  40}, { 25, 40}, { 25, 40},
            { 30,  40}, { 30,  40}, {  30,   40}, {  30,  40}
        }},
        // King (No mobility bonus in the same way, handled by PSTs)
        {{
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
            {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}
        }},
    }},

    // --- PAWN STRUCTURE ---
    .doubled_pawn_penalty    = {25, 30},   // Structural liability, especially later
    .isolated_pawn_penalty   = {25, 30},   // Easier to attack in endgame
    .backward_pawn_penalty   = {15, 25},   // Lags behind, tough to defend
    .pawn_chain_bonus = {{
        {  25,  35 },    // 1 Link
        {  60,  80 },    // 2 Links
        { 100, 130 },    // 3 Links
        { 135, 170 },    // 4 Links
        { 160, 210 },    // 5 Links
        { 180, 250 },    // 6 Links
        { 200, 290 },    // 7 Links
        { 220, 330 }     // 8 Links (max possible)
    }},

    // --- PASSED PAWNS ---
    .passed_pawn_bonus = {{
        {   0,   0 },  // Rank 1 (impossible)
        {  15, -20 },  // Rank 2
        {  30, -10 },  // Rank 3
        {  40,  20 },  // Rank 4
        {  55,  50 },  // Rank 5
        {  70,  80 },  // Rank 6
        { 100, 150 },  // Rank 7
        {   0,   0 }   // Rank 8 (promotion)
    }},
    .passed_pawn_supported_bonus = {25, 45}, // Extra if protected by own pawn
    .passed_pawn_blocked_penalty = {40, 25}, // Penalize blocked passed pawns

    // --- KING ACTIVITY ---
    .king_distance_from_center_penalty = {0, 3},
    .opponent_king_distance_from_center_bonus = {0, 6},
    .king_distance_from_opponent_king_penalty = {0, 15},

    // --- MISC STRATEGIC FACTORS ---

    .pawn_majority_bonus = {15, 30},     // On one wing, potential for passed pawns
    .rook_connected_bonus = {40, 30},    // Rooks on same rank/file

    // 4. King Safety Values
    .pawn_shield_penalty = {{
        {0, 0},      // No penalty if pawn is on its starting rank
        {10, 15},  // Small penalty if pawn advanced one square
        {25, 30}   // Larger penalty if pawn advanced two+ squares
    }},

    .open_file_penalty = {100, 20}, // Penalty if a shielding pawn is missing entirely

    .king_attack_weights = {{
        0, // NO_PIECE
        2, // KNIGHT
        2, // BISHOP
        3, // ROOK
        5, // QUEEN
        0  // KING
    }},

    // This table maps an attack score to a penalty.
    // The index is the sum of king_attack_weights of all attackers.
    // These values are just examples; they need to be tuned.
    .king_safety_table = {{
        {0,0},    {-2,0},   {-5,-1},  {-10,-2},  {-14,-3},  {-21,-5}, {-29,-7}, {-36,-9}, {-43,-10}, {-50,-12},
        /* 10 - 19  */
        {-57,-14}, {-64,-16}, {-71,-18}, {-79,-20}, {-86,-21}, {-93,-23}, {-100,-25}, {-107,-27}, {-114,-29}, {-121,-30},
        /* 20 - 29  */
        {-129,-32}, {-136,-34}, {-143,-36}, {-150,-37}, {-157,-39}, {-164,-41}, {-171,-43}, {-179,-44}, {-186,-46}, {-193,-48},
        /* 30 - 39  */
        {-200,-50},{-207,-51},{-214,-53},{-221,-55},{-229,-57},{-236,-59},{-243,-60},{-250,-62},{-257,-64},{-264,-66},
        /* 40 - 49  */
        {-271,-68},{-279,-70},{-286,-71},{-293,-73},{-300,-75},{-307,-77},{-314,-79},{-321,-80},{-329,-82},{-336,-84},
        /* 50 - 59  */
        {-343,-86},{-350,-87},{-357,-89},{-364,-91},{-371,-93},{-379,-94},{-386,-96},{-393,-98},{-400,-100},{-407,-101},
        /* 60 - 69  */
        {-414,-103},{-421,-105},{-429,-107},{-433,-108},{-438,-110},{-443,-110},{-448,-112},{-452,-113},{-457,-114},{-462,-115},
        /* 70 - 79  */
        {-467,-117},{-471,-118},{-476,-119},{-481,-120},{-486,-121},{-490,-122},{-495,-124},{-500,-125},{-505,-126},{-510,-127},
        /* 80 - 89  */
        {-514,-129},{-519,-130},{-524,-131},{-529,-132},{-533,-133},{-538,-134},{-543,-136},{-548,-137},{-552,-138},{-557,-139},
        /* 90 - 99  */
        {-562,-140},{-567,-141},{-571,-143},{-576,-144},{-581,-145},{-586,-146},{-590,-148},{-595,-149},{-600,-150},{-605,-151}
    }},

    .adjacent_files_masks = {{
        util::FileB,                    // File A
        util::FileA | util::FileC,      // File B
        util::FileB | util::FileD,      // File C
        util::FileC | util::FileE,      // File D
        util::FileD | util::FileF,      // File E
        util::FileE | util::FileG,      // File F
        util::FileF | util::FileH,      // File G
        util::FileG                     // File H
    }},
};

} // namespace eval