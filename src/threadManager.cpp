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
    std::cout << "bestmove " << bestmove.c_str() << "\n";
    std::cout.flush();

    for (int i = 0; i < settings.threads; ++i) {
        threads[i].join();
    }
    std::cout << "info nodes " << searchInfo.stats.totalNodes << "\n";
}



