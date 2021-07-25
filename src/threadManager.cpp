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


/** Mutex for search parameters */
SearchParams params;

/** Mutex for position settings */
string mutexFen;
vector<string> mutexMoves;
std::mutex positionMutex;

bool startID[16];

atomic_bool killThreads(false);

void startThread(int id){
    Thread thread = Thread(id);
    thread.idleLoop();
}

ThreadManager::ThreadManager(Settings newsettings){
    settings = newsettings;
}

void ThreadManager::Initialize(string fen, vector<string> moves){
    /** Main search object */
    mainSearch = Evaluate(fen, moves);

    /** Set variables for threads protected with mutex */
    positionMutex.lock();
    mutexFen = fen;
    for(string &move : moves){
        mutexMoves.push_back(move);
    }
    positionMutex.unlock();

    /** Start the helper threads */
    for (int i = 0; i < settings.threads; ++i) {
        threads[i] = std::thread(startThread, i);
    }

}

// start evaluation
Results ThreadManager::StartSearch(string fen, vector<string> moves) {
    Initialize(fen, moves);

    Results results;

    auto t1 = std::chrono::high_resolution_clock::now();

    /** alpha and beta, with a little buffer to stop any overflowing */
    int alpha = std::numeric_limits<int>::min() + 10000;
    int beta = std::numeric_limits<int>::max() - 10000;

    int score;
    for(int iterativeDepth = 1; iterativeDepth <= settings.depth; iterativeDepth++) {
        while(true) {
            score = lazySMP(iterativeDepth, alpha, beta);
            if((score > alpha && score < beta) || abs(score) >= 1000000){
                break;
            }
            else{
                /** widen aspiration window */
                if(score <= alpha){
                    alpha -= 100;
                }
                if(score >= beta){
                    beta += 100;
                }
            }
        }

        /** aspiration window */
        alpha = score - 50;
        beta = score + 50;

        /** if the timeFlag is set, it exited the evaluation prematurely, so take the previous best line */
        if(timeFlag.load()){
            PVline = previousBestLine;
            break;
        }
        previousBestLine = PVline;

        auto t2 = std::chrono::high_resolution_clock::now();
        int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        printinformation(milliseconds, score, PVline, stats, iterativeDepth);

        if(abs(score) >= 1000000) {
            // this indicates that mate is found
            break;
        }
    }

    /** Kill the threads */
    killThreads.store(true);
    for(int i = 0; i < settings.threads; i++){
        threads[i].join();
    }
    killThreads.store(false);

    results.bestMove = moveToStrNotation(PVline.principalVariation[0]);
    return results;
}

int ThreadManager::lazySMP(int depth, int &alpha, int &beta) {
    /** Start helper threads */
    copyline(&previousBestLine, &params.iterativeDeepeningLine);
    params.alpha = alpha; params.beta = beta;
    params.ply = depth;

    exitFlag.store(false);
    this->startAll();

    /** Main search */
    LINE iterativeLine {};
    copyline(&previousBestLine, &iterativeLine);
    int score = mainSearch.AlphaBeta(depth, alpha, beta, &PVline, &stats, iterativeLine);

    exitFlag.store(true);

    return score;
}

void ThreadManager::printinformation(int milliseconds, int score, LINE line, STATS stats, int depth) {
    string pv[100];

    std::cout << "info depth " << depth;
    std::cout << " time " << milliseconds;
    std::cout << " nodes " << stats.totalNodes;

    if(milliseconds !=0) {
        std::cout << " nps " << int(1000 * float(stats.totalNodes) / float(milliseconds));
    }
    std::cout << " score ";
    if(score >= 1000000){
        // this indicates that mate is found
        int mateIn = int((depth - score + 1000001)/2);
        std::cout << "mate " << mateIn << " pv ";
    }
    else if(score <= -1000000){
        // this indicates that the engine is getting mated
        int mateIn = -int((depth + score + 1000001)/2);
        std::cout << "mate " << mateIn << " pv ";
    }
    else{
        std::cout << "cp " << score << " pv ";
    }

    for(int i = 0; i < line.nmoves; i++){
        pv[i] = moveToStrNotation(line.principalVariation[i]);
        std::cout << pv[i] << " ";
    }
    std::cout << "\n";
    std::cout.flush();

    std::cout << "quiescent nodes " << stats.quiescentNodes << "/" << stats.totalNodes << " total nodes\n";
    std::cout << float(stats.quiescentNodes)/stats.totalNodes << "\n";

//    std::cout << stats.totalNodes - stats.quiescentNodes << " normal nodes\n";
//
//    std::cout << "table hits: " << stats.transpositionHits << "\n";

    std::cout <<'\n';
    std::cout.flush();

}

void ThreadManager::startAll() {
    for (int i = 0; i < settings.threads; ++i) {
        startID[i] = true;
    }

}

bool exitCondition() {
    if(timeFlag.load() or exitFlag.load()){
        return true;
    }
    return false;
}
