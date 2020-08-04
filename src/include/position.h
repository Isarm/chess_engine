#include <cstdint>
#include <iostream>

using namespace std;

#ifndef POSITION_H
#define POSITION_H

class Position{
public:

    uint64_t bitboards[12];
    Position(string FEN);


};

#endif