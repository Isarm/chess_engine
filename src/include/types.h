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
        OCCUPIED_SQUARES
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





    struct moveList{
        unsigned move[256] = {};
        unsigned moveLength = 0;
        unsigned captureMove[128] = {};
        unsigned captureMoveLength = 0;
    };

}

#endif