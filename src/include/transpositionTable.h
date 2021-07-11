//
// Created by isar on 03/02/2021.
//

#ifndef ENGINE_TRANSPOSITIONTABLE_H
#define ENGINE_TRANSPOSITIONTABLE_H


#include <cstdint>
#include <vector>


struct Entry{
    uint64_t key = 0;
    unsigned bestMove = 0;
    unsigned short depth = 0;
    int score = 0;
    short typeOfNode = 0;
    unsigned short age = 0;
};

enum typeOfNodes : short{
    EXACT_PV = 1, UPPER_BOUND_ALPHA = 2, LOWER_BOUND_BETA = 3
};

class TranspositionTable {

public:
    unsigned size = 0;

    Entry * table = nullptr;

    TranspositionTable();
    ~TranspositionTable();
    void setSize(unsigned int size);

    bool contains(uint64_t hash, Entry &entry);

    void addEntry(int score, unsigned int bestMove, unsigned short depth, typeOfNodes typeOfNode, uint64_t hash,
                  unsigned short halfmoveNumber);

    void reset();
};

extern TranspositionTable TT;


#endif //ENGINE_TRANSPOSITIONTABLE_H
