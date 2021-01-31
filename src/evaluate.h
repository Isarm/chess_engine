//
// Created by isar on 31/01/2021.
//

#ifndef ENGINE_EVALUATE_H
#define ENGINE_EVALUATE_H


class Evaluate {

public:
    Position position = Position("startpos");

    Evaluate(string fen, vector<string> moves, Settings settings);

    unsigned depth;

    Results Start();
};


#endif //ENGINE_EVALUATE_H
