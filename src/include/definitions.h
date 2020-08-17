#include <map>
#include <vector>

using namespace std;

#ifndef DICTIONARIES_H
#define DICTIONARIES_H

namespace definitions {

    enum bitboards2d{
        PAWNS, KNIGHTS, BISHOPS, ROOKS, QUEENS, KING, PIECES
    };

    enum turn{
        BLACK, WHITE
    };

    enum helpBitboards{
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
            {PAWNS, 'p'},
            {KNIGHTS, 'n'},
            {BISHOPS, 'b'},
            {ROOKS, 'r'},
            {QUEENS, 'q'},
            {KING, 'k'}
    };


    enum directionsShifting : unsigned {
        NNW  = 17, // right shift
        NN   = 16, // right shift
        NNE  = 15, // right shift
        NWW  = 10, // right shift
        NW   = 9,  // right shift
        N    = 8,  // right shift
        NE   = 7,  // right shift
        NEE  = 6,  // right shift
        W    = 1,  // right shift

        E    = 1,  // left shift
        SWW  = 6,  // left shift
        SW   = 7,  // left shift
        S    = 8,  // left shift
        SE   = 9,  // left shift
        SEE  = 10, // left shift
        SSW  = 15, // left shift
        SS   = 16, // left shift
        SSE  = 17  // left shift
    };

    enum rayDirections : int{
        VERTICAL_RAY = 1, HORIZONTAL_RAY, NORTHWEST_RAY, SOUTHWEST_RAY
    };


    enum moveListShift : unsigned{
        ORIGIN_SQUARE_SHIFT = 0,
        DESTINATION_SQUARE_SHIFT = 6,
        PROMOTION_TYPE_SHIFT = 12,
        SPECIAL_MOVE_FLAG_SHIFT = 14,
        CAPTURE_MOVE_FLAG_SHIFT = 16,
        CAPTURED_PIECE_TYPE_SHIFT = 17,
        CAPTURED_PIECE_INDEX_SHIFT = 20,


    };

    enum moveListMask : unsigned{
        ORIGIN_SQUARE_MASK = 0x3F,
        DESTINATION_SQUARE_MASK = 0xFC0,
        PROMOTION_TYPE_MASK = 0x3000,
        SPECIAL_MOVE_FLAG_MASK = 0xC000,
        CAPTURE_MOVE_FLAG_MASK = 0x10000,
        CAPTURED_PIECE_TYPE_MASK = 0xE0000,
        CAPTURED_PIECE_INDEX_MASK = 0x3F00000,
    };


    struct moveList{
        unsigned move[256] = {};
        unsigned moveLength = 0;
        unsigned captureMove[128] = {};
        unsigned captureMoveLength = 0;
    };


    enum Square : int {
        SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
        SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
        SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
        SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
        SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
        SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
        SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
        SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    };

    struct perftCounts{
        uint64_t total = 0;
        uint64_t normal = 0;
        uint64_t captures = 0;
    };


}

#endif