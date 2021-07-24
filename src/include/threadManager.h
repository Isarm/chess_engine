//
// Created by isar on 24/07/2021.
//

#ifndef ENGINE_THREADMANAGER_H
#define ENGINE_THREADMANAGER_H


#include <string>
#include <vector>
#include "definitions.h"
#include "position.h"
#include <pthread.h>
#include "atomic"

extern std::atomic_bool timerFlag;
extern std::atomic_bool exitFlag;

class ThreadManager {

public:
    ThreadManager();

    Results StartSearch(string basicString, vector<string> vector, Settings settings);

    void setSettings(Settings newSettings);

    string fen = "startpos";
    Settings settings;
    vector<string> moves = {};

private:
    pthread_t threads[12] = {};
    STATS threadStats[12] = {};


    void printinformation(int milliseconds, int score, LINE line, STATS stats, int depth);

    int lazySMP(int ply, int alpha, int beta, LINE *pline, STATS *stats, LINE iterativeDeepeningLine);
};


#endif //ENGINE_THREADMANAGER_H
