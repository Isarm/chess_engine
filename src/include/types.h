#include <map>

using namespace std;

#ifndef DICTIONARIES_H
#define DICTIONARIES_H

enum bitboards{
    BLACK_PAWNS, BLACK_KNIGHTS, BLACK_BISHOPS, BLACK_ROOKS, BLACK_QUEENS, BLACK_KING,
    WHITE_PAWNS, WHITE_KNIGHTS, WHITE_BISHOPS, WHITE_ROOKS, WHITE_QUEENS, WHITE_KING,
    BLACK_PIECES, WHITE_PIECES
};

static const map<char, int> FENpieces = {
        {'p', BLACK_PAWNS},
        {'n', BLACK_KNIGHTS},
        {'b', BLACK_BISHOPS},
        {'r', BLACK_ROOKS},
        {'q', BLACK_QUEENS},
        {'k', BLACK_KING},
        {'P', WHITE_PAWNS},
        {'N', WHITE_KNIGHTS},
        {'B', WHITE_BISHOPS},
        {'R', WHITE_ROOKS},
        {'Q', WHITE_QUEENS},
        {'K', WHITE_KING}
};

enum turn{
    WHITE_TURN, BLACK_TURN
};



enum knightMoves{
    NNW = -17,
    NNE = -15,
    NWW = -10,
    NEE = -6,
    ZWW = 6,
    ZEE = 10,
    ZZW = 15,
    ZZE = 17
};


// for the debruijn bit serialization method
const int index64[64] = {
        0,  1, 48,  2, 57, 49, 28,  3,
        61, 58, 50, 42, 38, 29, 17,  4,
        62, 55, 59, 36, 53, 51, 43, 22,
        45, 39, 33, 30, 24, 18, 12,  5,
        63, 47, 56, 27, 60, 41, 37, 16,
        54, 35, 52, 21, 44, 32, 23, 11,
        46, 26, 40, 15, 34, 20, 31, 10,
        25, 14, 19,  9, 13,  8,  7,  6
};

#endif