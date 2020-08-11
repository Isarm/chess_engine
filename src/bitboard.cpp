

using namespace std;

#include <string>
#include <bitset>
#include <bits/stdc++.h>
#include "bitboard.h"

string Bitboard::print(uint64_t bitboard){
    //convert to binary string
    string binary = bitset<64>(bitboard).to_string();
    //reverse
    reverse(binary.begin(), binary.end());

    //insert new line
    for(int i = 56; i > 0; i -= 8){
        binary.insert(i, "\n");
    }

    //insert spaces
    for(int i = binary.length() - 1; i >= 0; i-- ){
        binary.insert(i, " ");
    }


    return binary;
}