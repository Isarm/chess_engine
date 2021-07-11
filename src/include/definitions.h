#include <map>
#include <vector>

using namespace std;

#ifndef DICTIONARIES_H
#define DICTIONARIES_H

namespace definitions {

    enum bitboards2d {
        PAWNS, KNIGHTS, BISHOPS, ROOKS, QUEENS, KING, PIECES, EN_PASSANT_SQUARES
    };

    typedef enum {
        pawn, knight, bishop, rook, queen, king
    }Pieces;

    enum turn {
        BLACK, WHITE
    };

    enum helpBitboards {
        OCCUPIED_SQUARES, PINNED_PIECES
    };

    enum bitboards_full {
        BLACK_PAWNS, BLACK_KNIGHTS, BLACK_BISHOPS, BLACK_ROOKS, BLACK_QUEENS, BLACK_KING,
        WHITE_PAWNS, WHITE_KNIGHTS, WHITE_BISHOPS, WHITE_ROOKS, WHITE_QUEENS, WHITE_KING,
        BLACK_PIECES, WHITE_PIECES
    }; // use bitboards_full

    static const map<char, int> FENpieces = {
            {'p', PAWNS},
            {'n', KNIGHTS},
            {'b', BISHOPS},
            {'r', ROOKS},
            {'q', QUEENS},
            {'k', KING},
    };

    static const map<int, char> FENpiecesReverse = {
            {PAWNS,   'p'},
            {KNIGHTS, 'n'},
            {BISHOPS, 'b'},
            {ROOKS,   'r'},
            {QUEENS,  'q'},
            {KING,    'k'}
    };


    enum directionsShifting : unsigned {
        NNW = 17, // right shift
        NN = 16, // right shift
        NNE = 15, // right shift
        NWW = 10, // right shift
        NW = 9,  // right shift
        N = 8,  // right shift
        NE = 7,  // right shift
        NEE = 6,  // right shift
        W = 1,  // right shift

        E = 1,  // left shift
        SWW = 6,  // left shift
        SW = 7,  // left shift
        S = 8,  // left shift
        SE = 9,  // left shift
        SEE = 10, // left shift
        SSW = 15, // left shift
        SS = 16, // left shift
        SSE = 17  // left shift
    };

    enum rayDirections : int {
        VERTICAL_RAY = 1, HORIZONTAL_RAY, NORTHWEST_RAY, SOUTHWEST_RAY
    };


    enum moveListShift : unsigned {
        ORIGIN_SQUARE_SHIFT = 0,
        DESTINATION_SQUARE_SHIFT = 6,
        PROMOTION_TYPE_SHIFT = 12,
        SPECIAL_MOVE_FLAG_SHIFT = 14,
        MOVE_SCORE_SHIFT = 16,
        CAPTURE_MOVE_FLAG_SHIFT = 20,
        CAPTURED_PIECE_TYPE_SHIFT = 21,
        CAPTURED_PIECE_INDEX_SHIFT = 24,
        EN_PASSANT_DESTINATION_SQUARE_SHIFT = 30,
        CASTLING_RIGHTS_BEFORE_MOVE_SHIFT = 36,
        HALFMOVENUMBER_BEFORE_MOVE_SHIFT = 40,
        HALFMOVENUMBER_SINCE_IRREVERSIBLE_MOVE_SHIFT = 47
    };

    enum moveListMask : uint64_t {
        ORIGIN_SQUARE_MASK = 0x3F,
        DESTINATION_SQUARE_MASK = 0xFC0,
        PROMOTION_TYPE_MASK = 0x3000,
        SPECIAL_MOVE_FLAG_MASK = 0xC000,
        MOVE_SCORE_MASK = 0xF0000,
        CAPTURE_MOVE_FLAG_MASK = 0x100000,
        CAPTURED_PIECE_TYPE_MASK = 0xE00000,
        CAPTURED_PIECE_INDEX_MASK = 0x3F000000,
        EN_PASSANT_DESTINATION_SQUARE_MASK = 0xFC0000000,
        CASTLING_RIGHTS_BEFORE_MOVE_MASK =  0xF000000000,
        HALFMOVENUMBER_BEFORE_MOVE_MASK = 0x7F0000000000,
        HALFMOVENUMBER_SINCE_IRREVERSIBLE_MOVE_MASK = 0x3F800000000000
    };

    enum specialMoves : unsigned {
        PROMOTION_FLAG = 1,
        EN_PASSANT_FLAG = 2,
        CASTLING_FLAG = 3,
    };

    enum promotionPieces : unsigned {
        KNIGHTPROMOTION = 0,
        BISHOPPROMOTION = 1,
        ROOKPROMOTION = 2,
        QUEENPROMOTION = 3
    };

    enum castlingRights : unsigned {
        NO_CASTLING_RIGHTS = 0x0,
        WHITE_KINGSIDE_CASTLING_RIGHTS = 0x1,
        WHITE_QUEENSIDE_CASTLING_RIGHTS = 0x2,
        BLACK_KINGSIDE_CASTLING_RIGHTS = 0x4,
        BLACK_QUEENSIDE_CASTLING_RIGHTS = 0x8,

        WHITE_CASTLING_RIGHTS = 0x3,
        BLACK_CASTLING_RIGHTS = 0xC,

        KINGSIDE_CASTLING = 0x5,
        QUEENSIDE_CASTLING = 0xA,

        ALL_CASTLING_RIGHTS = 0xF,
    };


    const uint64_t WHITE_KINGSIDE_CASTLING_EMPTY_AND_NONATTACKED_SQUARES = 0x6000000000000000;
    const uint64_t BLACK_KINGSIDE_CASTLING_EMPTY_AND_NONATTACKED_SQUARES = 0x60;
    const uint64_t WHITE_QUEENSIDE_CASTLING_EMPTY = 0xE00000000000000;
    const uint64_t WHITE_QUEENSIDE_CASTLING_NONATTACKED = 0xC00000000000000;
    const uint64_t BLACK_QUEENSIDE_CASTLING_EMPTY = 0xE;
    const uint64_t BLACK_QUEENSIDE_CASTLING_NONATTACKED = 0xC;


    struct moveList {
        pair<unsigned, int> moves[512] = {};
        unsigned moveLength = 0;
    };


    enum Square : unsigned {
        SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
        SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
        SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
        SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
        SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
        SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
        SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
        SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    };

    struct perftCounts {
        uint64_t total = 0;
        uint64_t normal = 0;
    };

    struct Settings {
        int depth = 1;
    };

    struct Results {
        string bestMove;
    };

    struct LINE {
        int nmoves;
        unsigned principalVariation[100];
    };

    struct STATS {
        int totalNodes = 0;
        int betaCutoffs = 0;
        int quiescentNodes = 0;
        int transpositionHits = 0;
    };


    constexpr unsigned MAX_DEPTH = 30;
    constexpr unsigned KILLER_MOVE_SLOTS = 3;

    constexpr double BISHOP_MOBILITY_SCALING = 1.5;
    constexpr double KNIGHT_MOBILITY_SCALING = 1;
    constexpr double ROOK_MOBILITY_SCALING = 2;
    constexpr double QUEEN_MOBILITY_SCALING = 6;


    constexpr int CAPTURE_SCORE = 1000000;
    constexpr int KILLER_BONUS = 900000;

}

#endif