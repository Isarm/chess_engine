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

extern bool exitCondition();

class ThreadManager {

public:
    explicit ThreadManager(Settings settings);

    void StartSearch(string basicString, vector<string> vector);

private:

    Settings settings;

    std::thread threads[16];
};


#endif //ENGINE_THREADMANAGER_H
