
#include <position.h>
#include <string>
#include <cstdint>

#include "position.h"
#include "types.h"
#include "bitboard.h"

using namespace std;

Position::Position(string FEN){

    string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if(FEN == "startpos") FEN = startFEN;

    // initialize bitboards to 0.
    for(uint64_t& i : bitboards){
        i = 0;
    }

    // loop through the FEN string until first space
    unsigned BBindex = 0; // this gets incremented when the FEN contains numbers as well
    char pieceChar = FEN.at(0);
    for(int i = 0; pieceChar != ' '; i++, BBindex++, pieceChar = FEN.at(i)){
        if(pieceChar == '/'){
            BBindex--; // account for (incorrect) increment, as it is simply a new row
            continue;
        }
        if(pieceChar <= '9' && pieceChar > '0') {
            BBindex += pieceChar - '1';
            continue;
        }
        bitboards[FENpieces.at(pieceChar)] += 1uLL << BBindex; // shift the bits into place
    }

    for(int i = 0; i < 12; i++){
        cout << Bitboard::print(bitboards[i]);
        cout << "\n\n";
    }
}


string Position::GeneratePseudoLegalMoves(){
    
}

