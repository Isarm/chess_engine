//
// Created by isar on 12/08/2020.
//

#ifndef MOVEGENHELPFUNCTIONS_H
#define MOVEGENHELPFUNCTIONS_H



inline static bool notAFile(uint64_t bb) {
    return uint64_t(0x7F7F7F7F7F7F7F7F) & bb;
}
inline static bool notABFile(uint64_t bb){
    return uint64_t(0xFCFCFCFCFCFCFCFC) & bb;
}
inline static bool notHFile(uint64_t bb) {
    return uint64_t(0x7F7F7F7F7F7F7F7F) & bb;
}
inline static bool notGHFile(uint64_t bb) {
    return uint64_t(0x3F3F3F3F3F3F3F3F) & bb;
}
inline static bool is2ndRank(uint64_t bb){
    return uint64_t(0x00FF000000000000) & bb;
}
inline static bool is7thRank(uint64_t bb){
    return uint64_t(0x000000000000FF00) & bb;
}


uint64_t knightAttacks(uint64_t knight) {
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

unsigned debruijnSerialization(uint64_t &pieces) {

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
    }
    ;
    uint64_t LS1B = pieces & -pieces; // only keeps the 1st LSB bit so that the DeBruijn bitscan can be used
    pieces ^= LS1B; // remove this bit for next cycle using XOR
    return index64[(LS1B * debruijn64) >> 58]; //index64 defined in types.h
}


#endif //MOVEGENHELPFUNCTIONS_H
