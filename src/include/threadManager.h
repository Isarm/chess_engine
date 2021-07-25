//
// Created by isar on 24/07/2021.
//

#ifndef ENGINE_THREADMANAGER_H
#define ENGINE_THREADMANAGER_H

#include <thread>
#include "definitions.h"
#include "useful.h"
#include "evaluate.h"
#include "mutex"

extern std::atomic_bool timeFlag;
extern std::atomic_bool exitFlag;

extern SearchParams params;
extern std::mutex paramsMutex;

extern std::mutex startSearch;
extern bool startID[16];

extern std::atomic_bool killThreads;

extern string mutexFen;
extern vector<string> mutexMoves;
extern std::mutex positionMutex;

extern atomic_int doneCount;

extern bool exitCondition();

class ThreadManager {

public:
    explicit ThreadManager(Settings settings);

    Results StartSearch(string basicString, vector<string> vector);

    void printinformation(int milliseconds, int score, LINE line, STATS stats, int depth);


private:
    LINE PVline{};
    LINE previousBestLine{};
    STATS stats{};

    Settings settings;

    Evaluate mainSearch;
    Evaluate threadEvals[16];

    std::thread threads[16];

    int lazySMP(int depth, int &alpha, int &beta);

    void Initialize(string fen, vector<string> moves);

    void startAll();
};


#endif //ENGINE_THREADMANAGER_H
