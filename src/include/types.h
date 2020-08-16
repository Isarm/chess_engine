#include <map>
#include <vector>

using namespace std;

#ifndef DICTIONARIES_H
#define DICTIONARIES_H

namespace types {

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


    enum knightMoves : unsigned {
        NNW = 17, // right shift
        NNE = 15, // right shift
        NWW = 10, // right shift
        NEE = 6,  // right shift
        SWW = 6,  // left shift
        SEE = 10, // left shift
        SSW = 15, // left shift
        SSE = 17  // left shift
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
        ORIGIN_SQAURE_MASK = 0x3F,
        DESTINATION_SQARE_MASK = 0xFC0,
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

}

#endif