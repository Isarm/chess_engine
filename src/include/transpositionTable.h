//
// Created by isar on 03/02/2021.
//

#ifndef ENGINE_TRANSPOSITIONTABLE_H
#define ENGINE_TRANSPOSITIONTABLE_H


#include <cstdint>



struct Entry{
    uint64_t key;
    unsigned bestMove;
    unsigned depth;
    int score;
    int typeOfNode;
    int age;
};

enum typeOfNodes : int{
    EXACT, UPPER_BOUND, LOWER_BOUND
};

class TranspositionTable {

public:
    // hash table and size
    Entry *table{};
    unsigned size = 0;

    TranspositionTable();

    void setSize(unsigned int size);

    bool contains(uint64_t hash, Entry &entry);
};

extern TranspositionTable TT;


#endif //ENGINE_TRANSPOSITIONTABLE_H
