//
// Created by isar on 07/02/2021.
//

#ifndef ENGINE_ZOBRISTTABLES_H
// zobrist piece table for [colour][piece][index]
uint64_t zobristPieceTable[2][6][64];
uint64_t zobristCastlingRightsTable[16];
uint64_t zobristBlackToMove;
uint64_t zobristEnPassantFile[8];

inline void zobristPieceTableInitialize(){
    /* Random number generator */
    std::default_random_engine generator{static_cast<long long unsigned>(0xc9558c91601b5d95)};
    // use constant seed, such that hashes don't change between different runs, makes for easy debugging

    /* Distribution on which to apply the generator */
    std::uniform_int_distribution<unsigned long long> distribution(0, 0xFFFFFFFFFFFFFFFF);
    for(auto & i : zobristPieceTable){
        for(auto & j : i){
            for(unsigned long & k : j) {
                k = distribution(generator);
            }
        }
    }
    for(unsigned long & i : zobristCastlingRightsTable){
        i = distribution(generator);
    }
    zobristBlackToMove = distribution(generator);
    for(unsigned long & i : zobristEnPassantFile){
        i = distribution(generator);
    }

}
#define ENGINE_ZOBRISTTABLES_H

#endif //ENGINE_ZOBRISTTABLES_H
