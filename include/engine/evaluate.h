#pragma once
#include "chess/board.h"
#include "chess/util.h"
#include "chess/types.h"
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
    TaperedScore rook_on_open_file_bonus;
    TaperedScore rook_on_semi_open_file_bonus;
    TaperedScore rook_on_7th_bonus;
    TaperedScore knight_outpost_bonus;
    TaperedScore bishop_center_control;
    TaperedScore connected_pawn_bonus;
    TaperedScore doubled_pawn_penalty;
    TaperedScore isolated_pawn_penalty;
    TaperedScore backward_pawn_penalty;
    std::array<TaperedScore, 8> passed_pawn_bonus; // This might be an array per rank later
    TaperedScore passed_pawn_supported_bonus;
    TaperedScore passed_pawn_blocked_penalty;
    TaperedScore king_distance_from_center_penalty;
    TaperedScore opponent_king_distance_from_center_bonus;
    TaperedScore king_near_to_pawns_penalty;
    TaperedScore opponent_king_distance_opponent_king_penalty;
    TaperedScore space_bonus;          // More central squares controlled = higher eval
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
        {80, 120},   // PAWN
        {320, 320},   // KNIGHT
        {330, 360},   // BISHOP
        {500, 650},   // ROOK
        {900, 1000},  // QUEEN
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
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0},  // Rank 1
            {  5,  10}, { 10,  10}, { 10,  10}, {-20,  10}, {-20,  10}, { 10,  10}, { 10,  10}, {  5,  10}, // Rank 2
            {  5,  10}, { -5,  10}, {-10,  10}, {  0,  10}, {  0,  10}, {-10,  10}, { -5,  10}, {  5,  10}, // Rank 3
            {  0,  20}, {  0,  20}, {  0,  20}, { 20,  20}, { 20,  20}, {  0,  20}, {  0,  20}, {  0,  20}, // Rank 4
            {  5,  30}, {  5,  30}, { 10,  30}, { 25,  30}, { 25,  30}, { 10,  30}, {  5,  30}, {  5,  30}, // Rank 5
            { 10,  50}, { 10,  50}, { 20,  50}, { 30,  50}, { 30,  50}, { 20,  50}, { 10,  50}, { 10,  50}, // Rank 6
            { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, { 50,  80}, // Rank 7
            {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0} // Rank 8
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
            {-10, -10}, { 30,  20}, {  0,   5}, {  5,   5}, {  5,   5}, {  0,   5}, { 30,  20}, {-10, -10}, // Rank 2
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
    .bishop_pair_bonus     = {35, 60},     // Midgame: strong on open diagonals; Endgame: more mobility, pair dominates
    .rook_on_open_file_bonus = {70, 40},   // Rooks thrive on open/semi-open files
    .rook_on_semi_open_file_bonus = {55, 25},   // Rooks thrive on open/semi-open files
    .rook_on_7th_bonus       = {40, 60},   // Rook on 7th rank dominates enemy pawns
    .knight_outpost_bonus    = {30, 45},   // Knight anchored on a safe outpost
    .bishop_center_control   = {15, 25},   // Centralized bishop, good diagonals

    // --- PAWN STRUCTURE ---
    .connected_pawn_bonus    = {25, 35},   // Connected pawns reinforce each other
    .doubled_pawn_penalty    = {-30, -45}, // Structural liability, especially later
    .isolated_pawn_penalty   = {-20, -35}, // Easier to attack in endgame
    .backward_pawn_penalty   = {-15, -25}, // Lags behind, tough to defend

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
    .passed_pawn_blocked_penalty = {-40, -25}, // Penalize blocked passed pawns

    // --- KING ACTIVITY ---
    .king_distance_from_center_penalty = {0, 3},
    .opponent_king_distance_from_center_bonus = {0, 6},
    .king_near_to_pawns_penalty = {0, 3},
    .opponent_king_distance_opponent_king_penalty = {0, 15},

    // --- MISC STRATEGIC FACTORS ---

    .space_bonus         = {10, 20},     // More central squares controlled = higher eval
    .pawn_majority_bonus = {15, 30},     // On one wing, potential for passed pawns
    .rook_connected_bonus = {40, 30},    // Rooks on same rank/file

    // 4. King Safety Values
    .pawn_shield_penalty = {{
        {0, 0},      // No penalty if pawn is on its starting rank
        {-10, -15},  // Small penalty if pawn advanced one square
        {-25, -30}   // Larger penalty if pawn advanced two+ squares
    }},

    .open_file_penalty = {-100, -20}, // Penalty if a shielding pawn is missing entirely

    .king_attack_weights = {{
        0, // NO_PIECE
        2, // KNIGHT
        2, // BISHOP
        3, // ROOK
        5, // QUEEN
        0  // KING
    }},

    .king_safety_table = {{
        {0,0}, {-5,-1}, {-10,-2}, {-20,-5}, {-30,-7}, {-45,-11}, {-60,-15}, {-75,-18}, {-90,-22}, {-105,-26},
        /* 10 - 19 */
        {-120,-30}, {-135,-33}, {-150,-37}, {-165,-41}, {-180,-45}, {-195,-48}, {-210,-52}, {-225,-56}, {-240,-60}, {-255,-63},
        /* 20 - 29 */
        {-270,-67}, {-285,-71}, {-300,-75}, {-315,-78}, {-330,-82}, {-345,-86}, {-360,-90}, {-375,-93}, {-390,-97}, {-405,-101},
        /* 30 - 39 */
        {-420,-105},{-435,-108},{-450,-112},{-465,-116},{-480,-120},{-495,-123},{-510,-127},{-525,-131},{-540,-135},{-555,-138},
        /* 40 - 49 */
        {-570,-142},{-585,-146},{-600,-150},{-615,-153},{-630,-157},{-645,-161},{-660,-165},{-675,-168},{-690,-172},{-705,-176},
        /* 50 - 59 */
        {-720,-180},{-735,-183},{-750,-187},{-765,-191},{-780,-195},{-795,-198},{-810,-202},{-825,-206},{-840,-210},{-855,-213},
        /* 60 - 69 */
        {-870,-217},{-885,-221},{-900,-225},{-910,-227},{-920,-230},{-930,-232},{-940,-235},{-950,-237},{-960,-240},{-970,-242},
        /* 70 - 79 */
        {-980,-245},{-990,-247},{-1000,-250},{-1010,-252},{-1020,-255},{-1030,-257},{-1040,-260},{-1050,-262},{-1060,-265},{-1070,-267},
        /* 80 - 89 */
        {-1080,-270},{-1090,-272},{-1100,-275},{-1110,-277},{-1120,-280},{-1130,-282},{-1140,-285},{-1150,-287},{-1160,-290},{-1170,-292},
        /* 90 - 99 */
        {-1180,-295},{-1190,-297},{-1200,-300},{-1210,-302},{-1220,-305},{-1230,-307},{-1240,-310},{-1250,-312},{-1260,-315},{-1270,-317}
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