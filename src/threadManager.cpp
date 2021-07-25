//
// Created by isar on 24/07/2021.
//

#include <chrono>
#include <utility>
#include <mutex>
#include "threadManager.h"
#include "evaluate.h"
#include "thread.h"


std::atomic_bool timeFlag(false);
std::atomic_bool exitFlag(false);

atomic_bool killThreads(false);


string startThread(int id, Settings settings){
    Settings newSettings;
    newSettings.threads = settings.threads;
    newSettings.depth = settings.depth;
    for(string &move : settings.moves){
        newSettings.moves.push_back(move);
    }
    newSettings.fen = settings.fen;

    Thread thread = Thread(id, newSettings);
    return thread.search();
}

ThreadManager::ThreadManager(Settings newsettings){
    settings = std::move(newsettings);
}

// start evaluation
void ThreadManager::StartSearch(string fen, vector<string> moves) {
    settings.fen = std::move(fen);
    settings.moves = std::move(moves);

    searchInfo = SearchInfo();

    /** Start the helper threads */
    for (int i = 0; i < settings.threads; ++i) {
        threads[i] = std::thread(startThread, i, settings);
    }

    /** Main thread with id -1 */
    string bestmove = startThread(-1, settings);
    printf("bestmove %s\n", bestmove.c_str());

    for (int i = 0; i < settings.threads; ++i) {
        threads[i].join();
    }
}


bool exitCondition() {
    if(timeFlag.load() or exitFlag.load()){
        return true;
    }
    return false;
}
