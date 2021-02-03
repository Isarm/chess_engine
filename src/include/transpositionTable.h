//
// Created by isar on 03/02/2021.
//

#ifndef ENGINE_TRANSPOSITIONTABLE_H
#define ENGINE_TRANSPOSITIONTABLE_H


#include <cstdint>
#include <vector>


struct Entry{
    uint64_t key;
    unsigned bestMove;
    unsigned short depth;
    int score;
    short typeOfNode;
    unsigned short age;
};

enum typeOfNodes : short{
    EXACT_PV, UPPER_BOUND_ALPHA, LOWER_BOUND_BETA
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
};

extern TranspositionTable TT;


#endif //ENGINE_TRANSPOSITIONTABLE_H
