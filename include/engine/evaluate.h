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
    TaperedScore bad_bishop_penalty;
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
        {0, 0},   // NO_PIECE_TYPE
        {80, 100},   // PAWN
        {320, 320},   // KNIGHT
        {330, 360},   // BISHOP
        {500, 600},   // ROOK
        {900, 1000},  // QUEEN
        {0, 0}      // KING (material value is infinite/not counted)
    }},

    // .material_values = {{
    // {0, 0},   // NO_PIECE_TYPE
    // {80, 100},   // PAWN
    // {400, 420},   // KNIGHT
    // {450, 480},   // BISHOP
    // {600, 650},   // ROOK
    // {1100, 1200},  // QUEEN
    // {0, 0}      // KING (material value is infinite/not counted)
    // }},

    // .material_values = {{
    //     {0, 0},   // NO_PIECE_TYPE
    //     {80, 100},   // PAWN
    //     {400, 420},   // KNIGHT
    //     {500, 530},   // BISHOP
    //     {1200, 1300},   // ROOK
    //     {2500, 2600},  // QUEEN
    //     {0, 0}      // KING (material value is infinite/not counted)
    // }},

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
            { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, // Rank 2
            { 10,  50}, { 10,  50}, { 20,  50}, { 30,  50}, { 30,  50}, { 20,  50}, { 10,  50}, { 10,  50}, // Rank 3
            {  5,  30}, {  5,  30}, { 10,  30}, { 25,  30}, { 25,  30}, { 10,  30}, {  5,  30}, {  5,  30}, // Rank 4
            {  0,  20}, {  0,  20}, {  0,  20}, { 20,  20}, { 20,  20}, {  0,  20}, {  0,  20}, {  0,  20}, // Rank 5
            {  5,  10}, { -5,  10}, {-10,  10}, {  0,  10}, {  0,  10}, {-10,  10}, { -5,  10}, {  5,  10}, // Rank 6
            {  5,  10}, { 10,  10}, { 10,  10}, {-20,  10}, {-20,  10}, { 10,  10}, { 10,  10}, {  5,  10}, // Rank 7
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
            {-10, -10}, { 12,  12}, {  0,   5}, {  5,   5}, {  5,   5}, {  0,   5}, { 12,  12}, {-10, -10}, // Rank 2
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
            {  5,  10}, { 10,  15}, { 10,  15}, { 10,  15}, { 10,  15}, { 10,  15}, { 10,  15}, {  5,  10}, // Rank 2
            { -5,  10}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, { -5,  10}, // Rank 3
            { -5,  10}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, { -5,  10}, // Rank 4
            { -5,  10}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, { -5,  10}, // Rank 5
            { -5,  10}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, { -5,  10}, // Rank 6
            { -5,  10}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, {  0,  15}, { -5,  10}, // Rank 7
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
            {-30, -50}, {-40, -30}, {-40, -20}, {-50, -10}, {-50, -10}, {-40, -20}, {-40, -30}, {-30, -50}, // Rank 1
            {-30, -30}, {-40, -10}, {-40,   0}, {-50,  10}, {-50,  10}, {-40,   0}, {-40, -10}, {-30, -30}, // Rank 2
            {-30, -20}, {-40,   0}, {-40,  15}, {-50,  20}, {-50,  20}, {-40,  15}, {-40,   0}, {-30, -20}, // Rank 3
            {-30, -10}, {-40,  10}, {-40,  20}, {-50,  25}, {-50,  25}, {-40,  20}, {-40,  10}, {-30, -10}, // Rank 4
            {-20, -10}, {-30,  10}, {-30,  20}, {-40,  25}, {-40,  25}, {-30,  20}, {-30,  10}, {-20, -10}, // Rank 5
            {-10, -20}, {-20,   0}, {-20,  15}, {-20,  20}, {-20,  20}, {-20,  15}, {-20,   0}, {-10, -20}, // Rank 6
            { 20, -30}, { 20, -10}, {  0,   0}, {  0,  10}, {  0,  10}, {  0,   0}, { 20, -10}, { 20, -30}, // Rank 7
            { 20, -50}, { 30, -30}, { 10, -20}, {  0, -10}, {  0, -10}, { 10, -20}, { 30, -30}, { 20, -50}  // Rank 8
        }}
    }},

    // 3. Bonuses and Penalties
    // --- STRUCTURAL & PIECE SYNERGY BONUSES ---
    .bishop_pair_bonus     = {35, 60},     // Midgame: strong on open diagonals; Endgame: more mobility, pair dominates
    .rook_on_open_file_bonus = {70, 40},   // Rooks thrive on open/semi-open files
    .rook_on_semi_open_file_bonus = {55, 25},   // Rooks thrive on open/semi-open files
    .rook_on_7th_bonus       = {40, 60},   // Rook on 7th rank dominates enemy pawns
    .knight_outpost_bonus    = {30, 45},   // Knight anchored on a safe outpost
    .bishop_center_control   = {15, 25},   // Centralized bishop, good diagonals
    .bad_bishop_penalty      = {10, 2},    // Penalty for each friendly pawn on the same color square
    .controlled_square_bonus = {5, 2}, 

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
            {-62, -81}, {-53, -53}, {-12, -29}, {-4,  -2}, { 3,  12}, { 12,  23}, { 21,  34}, { 28,  43},{ 32,  55}
        }},
        // Bishop (Max 13 moves)
        {{ // MG, EG
            {-48, -59}, {-20, -22}, { 16, -11}, { 26,   4}, { 38,  14}, { 51,  27}, { 55,  34}, { 63,  45},
            { 68,  54}, { 81,  68}, { 88,  78}, { 91,  89}, { 98,  97}, {108, 100}
        }},
        // Rook (Max 14 moves)
        {{ // MG, EG
            {-53, -72}, {-28, -26}, {-14,  13}, { -5,  32}, { -2,  54}, {  6,  73}, { 13,  84}, { 19,  98},
            { 29, 106}, { 37, 116}, { 40, 123}, { 43, 127}, { 45, 128}, { 51, 130}, { 51, 130}
        }},
        // Queen (Max 27 moves)
        {{ // MG, EG
            {-39, -42}, {-21, -22}, { -9,  -9}, {  3,   7}, {  8,  18}, { 17,  28}, { 21,  38}, { 28,  49},
            { 33,  56}, { 38,  67}, { 45,  73}, { 49,  78}, { 54,  84}, { 58,  89}, { 63,  96}, { 63, 100},
            { 68, 106}, { 72, 112}, { 75, 116}, { 78, 121}, { 83, 126}, { 87, 133}, { 89, 136}, { 93, 138},
            { 96, 143}, { 99, 148}, {102, 150}, {102, 150}
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
    .doubled_pawn_penalty    = {15, 30},   // Structural liability, especially later
    .isolated_pawn_penalty   = {15, 30},   // Easier to attack in endgame
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
        {  15,  40 },  // Rank 2
        {  30,  60 },  // Rank 3
        {  50,  90 },  // Rank 4
        {  90, 150 },  // Rank 5
        { 160, 260 },  // Rank 6
        { 250, 400 },  // Rank 7
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
        {0,0},    {-5,-1},   {-10,-2},  {-20,-5},  {-30,-7},  {-45,-11}, {-60,-15}, {-75,-18}, {-90,-22}, {-105,-26},
        /* 10 - 19  */
        {-120,-30}, {-135,-33}, {-150,-37}, {-165,-41}, {-180,-45}, {-195,-48}, {-210,-52}, {-225,-56}, {-240,-60}, {-255,-63},
        /* 20 - 29  */
        {-270,-67}, {-285,-71}, {-300,-75}, {-315,-78}, {-330,-82}, {-345,-86}, {-360,-90}, {-375,-93}, {-390,-97}, {-405,-101},
        /* 30 - 39  */
        {-420,-105},{-435,-108},{-450,-112},{-465,-116},{-480,-120},{-495,-123},{-510,-127},{-525,-131},{-540,-135},{-555,-138},
        /* 40 - 49  */
        {-570,-142},{-585,-146},{-600,-150},{-615,-153},{-630,-157},{-645,-161},{-660,-165},{-675,-168},{-690,-172},{-705,-176},
        /* 50 - 59  */
        {-720,-180},{-735,-183},{-750,-187},{-765,-191},{-780,-195},{-795,-198},{-810,-202},{-825,-206},{-840,-210},{-855,-213},
        /* 60 - 69  */
        {-870,-217},{-885,-221},{-900,-225},{-910,-227},{-920,-230},{-930,-232},{-940,-235},{-950,-237},{-960,-240},{-970,-242},
        /* 70 - 79  */
        {-980,-245},{-990,-247},{-1000,-250},{-1010,-252},{-1020,-255},{-1030,-257},{-1040,-260},{-1050,-262},{-1060,-265},{-1070,-267},
        /* 80 - 89  */
        {-1080,-270},{-1090,-272},{-1100,-275},{-1110,-277},{-1120,-280},{-1130,-282},{-1140,-285},{-1150,-287},{-1160,-290},{-1170,-292},
        /* 90 - 99  */
        {-1180,-295},{-1190,-297},{-1200,-300},{-1210,-302},{-1220,-305},{-1230,-307},{-1240,-310},{-1250,-312},{-1260,-315},{-1270,-317}
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