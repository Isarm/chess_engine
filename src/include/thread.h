//
// Created by isar on 24/07/2021.
//

#ifndef ENGINE_THREAD_H
#define ENGINE_THREAD_H

#include <mutex>
#include "evaluate.h"
#include "definitions.h"

class Thread {
public:
    explicit Thread(int id, Settings settings);

    void idleLoop();

    string search();

private:


    Evaluate evaluate;
    int id;

    Settings settings;

    void printinformation(int milliseconds, int score, LINE line, STATS stats, int depth);
};


#endif //ENGINE_THREAD_H
