//
// Created by isar on 12/08/2020.
//

#ifndef MOVEGENHELPFUNCTIONS_H
#define MOVEGENHELPFUNCTIONS_H


#include <random>
#include "definitions.h"

using namespace definitions;

inline static bool notAFile(const uint64_t bb) {
    return uint64_t(0xFEFEFEFEFEFEFEFE) & bb;
}

inline static bool notABFile(const uint64_t bb) {
    return uint64_t(0xFCFCFCFCFCFCFCFC) & bb;
}

inline static bool notHFile(const uint64_t bb) {
    return uint64_t(0x7F7F7F7F7F7F7F7F) & bb;
}

inline static bool notGHFile(const uint64_t bb) {
    return uint64_t(0x3F3F3F3F3F3F3F3F) & bb;
}

inline static bool is1stRank(const uint64_t bb) {
    return uint64_t(0xFF00000000000000) & bb;
}

inline static bool is2ndRank(const uint64_t bb) {
    return uint64_t(0x00FF000000000000) & bb;
}

inline static bool is7thRank(const uint64_t bb) {
    return uint64_t(0x000000000000FF00) & bb;
}

inline static bool is8thRank(const uint64_t bb) {
    return uint64_t(0x00000000000000FF) & bb;
}

inline uint64_t knightAttacks(const uint64_t knight) {
    uint64_t currentKnightMoves = 0;
    if (notAFile(knight)) {
        currentKnightMoves |= ((knight >> NNW) | (knight << SSW));
    }
    if (notABFile(knight)) {
        currentKnightMoves |= ((knight >> NWW) | (knight << SWW));
    }
    if (notHFile(knight)) {
        currentKnightMoves |= ((knight >> NNE) | (knight << SSE));
    }
    if (notGHFile(knight)) {
        currentKnightMoves |= ((knight >> NEE) | (knight << SEE));
    }
    return currentKnightMoves;
}

inline uint64_t kingAttacks(const uint64_t king){
    uint64_t kingAttacks = 0;
    if(notAFile(king)){
        kingAttacks |= (king >> NW) | (king >> W) | (king << SW);
    }
    if(notHFile(king)) {
        kingAttacks |= (king >> NE) | (king << E) | (king << SE);
    }
    kingAttacks |= (king >> N) | (king << S);

    return kingAttacks;
}

// from the chess programming wiki
// for the debruijn bit serialization method

const auto debruijn64 = uint64_t(0x03f79d71b4cb0a89);
const int index64[64] = {
        0, 1, 48, 2, 57, 49, 28, 3,
        61, 58, 50, 42, 38, 29, 17, 4,
        62, 55, 59, 36, 53, 51, 43, 22,
        45, 39, 33, 30, 24, 18, 12, 5,
        63, 47, 56, 27, 60, 41, 37, 16,
        54, 35, 52, 21, 44, 32, 23, 11,
        46, 26, 40, 15, 34, 20, 31, 10,
        25, 14, 19, 9, 13, 8, 7, 6
};



inline unsigned debruijnSerialization(const uint64_t pieces) {
    uint64_t LS1B = pieces & -pieces; // only keeps the 1st LSB bit so that the DeBruijn bitscan can be used
    return index64[(LS1B * debruijn64) >> 58u];
}





inline string moveToStrNotation(const unsigned move){
    unsigned originInt = (move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT;
    unsigned destinationInt =  (move & DESTINATION_SQUARE_MASK) >> DESTINATION_SQUARE_SHIFT;


    char str[5] = "";
    str[0] = originInt%8 + 'a';
    str[1] = 8 - int(originInt/8) + '0';
    str[2] = destinationInt%8 + 'a';
    str[3] = 8 - int(destinationInt/8) + '0';

    switch ((move & SPECIAL_MOVE_FLAG_MASK) >> SPECIAL_MOVE_FLAG_SHIFT) {
        case CASTLING_FLAG:
           switch ((move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT) {
               case WHITE_KINGSIDE_CASTLING_RIGHTS:
                   return "e1g1";
               case WHITE_QUEENSIDE_CASTLING_RIGHTS:
                   return "e1c1";
               case BLACK_KINGSIDE_CASTLING_RIGHTS:
                   return "e8g8";
               case BLACK_QUEENSIDE_CASTLING_RIGHTS:
                   return "e8c8";
           }
           break;
        case PROMOTION_FLAG:
           char types[] = {'n', 'b', 'r', 'q'};
           unsigned promotionType = (move & PROMOTION_TYPE_MASK) >> PROMOTION_TYPE_SHIFT;
           str[4] = types[promotionType];

    }


    return string(str);

};

inline unsigned strToMoveNotation(const string str){
    unsigned originInt = 0, destinationInt = 0, move;

    originInt += str[0] - 'a';
    originInt += 8 * (8 - (str[1] - '0'));
    destinationInt += str[2] - 'a';
    destinationInt += 8 * (8 - (str[3] - '0'));

    move = originInt << ORIGIN_SQUARE_SHIFT;
    move |= destinationInt << DESTINATION_SQUARE_SHIFT;

    // check for promotion type
    switch (tolower(str[4])) {
        case 'n':
            move |= KNIGHTPROMOTION << PROMOTION_TYPE_SHIFT;
            move |= PROMOTION_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
            break;
        case 'b':
            move |= BISHOPPROMOTION << PROMOTION_TYPE_SHIFT;
            move |= PROMOTION_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
            break;
        case 'r':
            move |= ROOKPROMOTION << PROMOTION_TYPE_SHIFT;
            move |= PROMOTION_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
            break;
        case 'q':
            move |= QUEENPROMOTION << PROMOTION_TYPE_SHIFT;
            move |= PROMOTION_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
            break;
        default:
            break;
    }

    return move;
}



#endif //MOVEGENHELPFUNCTIONS_H
