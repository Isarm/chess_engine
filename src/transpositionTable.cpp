//
// Created by isar on 03/02/2021.
//

#include <new>
#include <iostream>
#include <evaluate.h>
#include "transpositionTable.h"
#include "threadManager.h"

TranspositionTable TT;

TranspositionTable::TranspositionTable() = default;

void TranspositionTable::setSize(unsigned sizeMB){
    size = 1024ull * 1024ull * sizeMB / sizeof(Entry);

    table = new Entry[size];
}

void TranspositionTable::reset(){
    free(table);
    table = new Entry[size];
}

inline unsigned short calculateChecksum(Entry entry){
    unsigned long long checksum = 0;
    checksum = entry.depth ^ entry.bestMove ^ entry.age ^ entry.key ^ entry.score ^ entry.typeOfNode;
    return checksum;
}

bool TranspositionTable::contains(uint64_t hash, Entry &entry){
    uint64_t entryHash = table[hash % size].key;
    if(hash == entryHash){
        // point to entry
        entry = table[hash % size];
        /** Verify checksum*/
        if(calculateChecksum(entry) != entry.checksum){
            table[hash % size] = Entry();
            return false;
        }
        return true;
    }
    return false;
}

void TranspositionTable::addEntry(int score, unsigned int bestMove, unsigned short depth, typeOfNodes typeOfNode, uint64_t hash,
                                  unsigned short halfmoveNumber) {
    if(exitCondition()){
        return;
    }
    Entry newEntry{};
    newEntry = {hash, bestMove, depth, score, typeOfNode, halfmoveNumber};
    newEntry.checksum = calculateChecksum(newEntry);

    if(table[hash % size].typeOfNode){ // check if there is already a node (as this is 0 for no node)
        // replacement scheme: check for depth (and if equal replace) and age
        if(table[hash % size].depth <= newEntry.depth){
            table[hash % size] = newEntry;
        }
        else if(halfmoveNumber - table[hash % size].age > 5){
            table[hash % size] = newEntry;
        }
        // otherwise do not replace
    }
    else {
        // if not taken replace
        table[hash % size] = newEntry;
    }
}

TranspositionTable::~TranspositionTable() {
    free(table);
}


