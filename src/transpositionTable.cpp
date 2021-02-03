//
// Created by isar on 03/02/2021.
//

#include <new>
#include <iostream>
#include "transpositionTable.h"

TranspositionTable TT;

TranspositionTable::TranspositionTable() = default;

void TranspositionTable::setSize(unsigned sizeMB){
    size = 1024 * 1024 * sizeMB / sizeof(Entry);

    try {
        table = new Entry[size];
    }
    catch (std::bad_alloc &ba) {
        std::cerr << "Unable to allocate memory for hash table: " << ba.what() << '\n';
    }
}

bool TranspositionTable::contains(uint64_t hash, Entry &entry){
    uint64_t entryHash = table[hash % size].key;
    if(hash == entryHash){
        // point to entry
        entry = table[hash % size];
        return true;
    }
}




