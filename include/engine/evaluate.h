#pragma once
#include "board.h"
#include "util.h"
#include "types.h"
#include <array>

namespace eval {
constexpr int KNIGHT_PHASE = 1;
constexpr int BISHOP_PHASE = 1;
constexpr int ROOK_PHASE   = 2;
constexpr int QUEEN_PHASE  = 4;
constexpr int TOTAL_PHASE  = (KNIGHT_PHASE * 4) + (BISHOP_PHASE * 4) + (ROOK_PHASE * 4) + (QUEEN_PHASE * 2);


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
    // Phase values for each piece type
    std::array<int, chess::PIECE_TYPE_NB> phase_values;

    // Material values for each piece type
    std::array<TaperedScore, chess::PIECE_TYPE_NB> material_values;

    // Piece-Square Tables (PSTs)
    // Indexed by [PieceType][Square]
    std::array<std::array<TaperedScore, 64>, chess::PIECE_TYPE_NB> psts;

    // Specific bonuses and penalties
    TaperedScore bishop_pair_bonus;
    TaperedScore connected_pawn_bonus;
    TaperedScore doubled_pawn_penalty;
    TaperedScore isolated_pawn_penalty;
    std::array<TaperedScore, 8> passed_pawn_bonus; // This might be an array per rank later

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

    std::array<uint64_t, chess::SQUARE_NB> passed_pawn_masks_white;
    std::array<uint64_t, chess::SQUARE_NB> passed_pawn_masks_black;

    std::array<uint64_t, 8> adjacent_files_masks;
};

// Best Practice 3: Create a single, compile-time constant instance of the configuration.
// This ensures all values are in one place and initialized safely.
constexpr EvalData eval_data = {
    .phase_values = {{
        0, // PAWN (has no phase value)
        1, // KNIGHT
        1, // BISHOP
        2, // ROOK
        4, // QUEEN
        0  // KING (has no phase value)
    }},

    // 1. Material Values
    .material_values = {{
        {0, 0},   // NO_PIECE_TYPE
        {100, 100}, // PAWN
        {320, 320}, // KNIGHT
        {330, 330}, // BISHOP
        {500, 500}, // ROOK
        {900, 900}, // QUEEN
        {0, 0}      // KING (material value is infinite/not counted)
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
    .bishop_pair_bonus     = {30, 50},
    .connected_pawn_bonus    = {20, 30},
    .doubled_pawn_penalty  = {-25, -35},
    .isolated_pawn_penalty = {-15, -25},
    .passed_pawn_bonus     = {{
    {  0,   0 },  // Rank 1 (impossible)
    {  5,  15 },  // Rank 2 (starting rank)
    { 10,  25 },  // Rank 3
    { 20,  45 },  // Rank 4
    { 35,  75 },  // Rank 5 (major threat)
    { 60, 125 },  // Rank 6 (very dangerous)
    { 100, 220},  // Rank 7 (unstoppable)
    {   0,   0 }   // Rank 8 (promotion)},
    }},

    // 4. King Safety Values
    .pawn_shield_penalty = {{
        {0, 0},      // No penalty if pawn is on its starting rank
        {-10, -15},  // Small penalty if pawn advanced one square
        {-25, -30}   // Larger penalty if pawn advanced two+ squares
    }},

    .open_file_penalty = {-30, -40}, // Penalty if a shielding pawn is missing entirely

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
        // Index is the raw 'attack score'
        // Values are {middlegame_penalty, endgame_penalty}
        // A higher attack score leads to a much larger penalty.

        /* 0 - 9   */
        {0,0},    {-5,-5},  {-10,-10}, {-20,-20}, {-30,-30}, {-45,-45}, {-60,-60}, {-75,-75}, {-90,-90}, {-105,-105},
        /* 10 - 19  */
        {-120,-120},{-135,-135},{-150,-150},{-165,-165},{-180,-180},{-195,-195},{-210,-210},{-225,-225},{-240,-240},{-255,-255},
        /* 20 - 29  */
        {-270,-270},{-285,-285},{-300,-300},{-315,-315},{-330,-330},{-345,-345},{-360,-360},{-375,-375},{-390,-390},{-405,-405},
        /* 30 - 39  */
        {-420,-420},{-435,-435},{-450,-450},{-465,-465},{-480,-480},{-495,-495},{-510,-510},{-525,-525},{-540,-540},{-555,-555},
        /* 40 - 49  */
        {-570,-570},{-585,-585},{-600,-600},{-615,-615},{-630,-630},{-645,-645},{-660,-660},{-675,-675},{-690,-690},{-705,-705},
        /* 50 - 59  */
        {-720,-720},{-735,-735},{-750,-750},{-765,-765},{-780,-780},{-795,-795},{-810,-810},{-825,-825},{-840,-840},{-855,-855},
        /* 60 - 69  */
        {-870,-870},{-885,-885},{-900,-900},{-910,-910},{-920,-920},{-930,-930},{-940,-940},{-950,-950},{-960,-960},{-970,-970},
        /* 70 - 79  */
        {-980,-980},{-990,-990},{-1000,-1000},{-1010,-1010},{-1020,-1020},{-1030,-1030},{-1040,-1040},{-1050,-1050},{-1060,-1060},{-1070,-1070},
        /* 80 - 89  */
        {-1080,-1080},{-1090,-1090},{-1100,-1100},{-1110,-1110},{-1120,-1120},{-1130,-1130},{-1140,-1140},{-1150,-1150},{-1160,-1160},{-1170,-1170},
        /* 90 - 99  */
        {-1180,-1180},{-1190,-1190},{-1200,-1200},{-1210,-1210},{-1220,-1220},{-1230,-1230},{-1240,-1240},{-1250,-1250},{-1260,-1260},{-1270,-1270}
    }},

    .passed_pawn_masks_white = {
        217020518514230016ULL, 506381209866536704ULL, 1012762419733073408ULL, 2025524839466146816ULL, 4051049678932293632ULL, 8102099357864587264ULL, 16204198715729174528ULL, 13889313184910721024ULL, 217020518514229248ULL, 506381209866534912ULL, 1012762419733069824ULL, 2025524839466139648ULL, 4051049678932279296ULL, 8102099357864558592ULL, 16204198715729117184ULL, 13889313184910671872ULL, 217020518514032640ULL, 506381209866076160ULL, 1012762419732152320ULL, 2025524839464304640ULL, 4051049678928609280ULL, 8102099357857218560ULL, 16204198715714437120ULL, 13889313184898088960ULL, 217020518463700992ULL, 506381209748635648ULL, 1012762419497271296ULL, 2025524838994542592ULL, 4051049677989085184ULL, 8102099355978170368ULL, 16204198711956340736ULL, 13889313181676863488ULL, 217020505578799104ULL, 506381179683864576ULL, 1012762359367729152ULL, 2025524718735458304ULL, 4051049437470916608ULL, 8102098874941833216ULL, 16204197749883666432ULL, 13889312357043142656ULL, 217017207043915776ULL, 506373483102470144ULL, 1012746966204940288ULL, 2025493932409880576ULL, 4050987864819761152ULL, 8101975729639522304ULL, 16203951459279044608ULL, 13889101250810609664ULL, 216172782113783808ULL, 504403158265495552ULL, 1008806316530991104ULL, 2017612633061982208ULL, 4035225266123964416ULL, 8070450532247928832ULL, 16140901064495857664ULL, 13835058055282163712ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL
    },

    .passed_pawn_masks_black = {
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 3ULL, 7ULL, 14ULL, 28ULL, 56ULL, 112ULL, 224ULL, 192ULL, 771ULL, 1799ULL, 3598ULL, 7196ULL, 14392ULL, 28784ULL, 57568ULL, 49344ULL, 197379ULL, 460551ULL, 921102ULL, 1842204ULL, 3684408ULL, 7368816ULL, 14737632ULL, 12632256ULL, 50529027ULL, 117901063ULL, 235802126ULL, 471604252ULL, 943208504ULL, 1886417008ULL, 3772834016ULL, 3233857728ULL, 12935430915ULL, 30182672135ULL, 60365344270ULL, 120730688540ULL, 241461377080ULL, 482922754160ULL, 965845508320ULL, 827867578560ULL, 3311470314243ULL, 7726764066567ULL, 15453528133134ULL, 30907056266268ULL, 61814112532536ULL, 123628225065072ULL, 247256450130144ULL, 211934100111552ULL, 847736400446211ULL, 1978051601041159ULL, 3956103202082318ULL, 7912206404164636ULL, 15824412808329272ULL, 31648825616658544ULL, 63297651233317088ULL, 54255129628557504ULL
    },

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