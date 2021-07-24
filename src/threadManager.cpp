//
// Created by isar on 24/07/2021.
//

#include <chrono>
#include <utility>
#include <search.h>
#include "threadManager.h"
#include "useful.h"
#include "definitions.h"
#include "position.h"


atomic_bool timerFlag(false);
atomic_bool exitFlag(false);



ThreadManager::ThreadManager() = default;

/**
 * start search
 */
Results ThreadManager::StartSearch(string positionFen, vector<string> newMoves, Settings newSettings) {
    Results results;

    this->fen = positionFen;
    this->moves = newMoves;
    setSettings(newSettings);


    LINE line{};
    LINE previousBestLine{};
    STATS stats{};


    auto t1 = std::chrono::high_resolution_clock::now();

    /** alpha and beta, with a little buffer to stop any overflowing */
    int alpha = std::numeric_limits<int>::min() + 10000;
    int beta = std::numeric_limits<int>::max() - 10000;

    int score = 0;
    for(int iterativeDepth = 1; iterativeDepth <= settings.depth; iterativeDepth++) {
        line = {};
        while(true) {


            score = ThreadManager::lazySMP(iterativeDepth, alpha, beta, &line,
                                           &stats, previousBestLine);
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

        /** if the timerflag is set, it exited the evaluation prematurely, so take the previous best line */
        if(timerFlag.load()){
            line = previousBestLine;
            break;
        }
        previousBestLine = line;

        auto t2 = std::chrono::high_resolution_clock::now();
        int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        printinformation(milliseconds, score, line, stats, iterativeDepth);

        if(abs(score) >= 1000000) {
            // this indicates that mate is found
            break;
        }
    }
    results.bestMove = moveToStrNotation(line.principalVariation[0]);
    return results;
}

void * startThread(void * args){
    searchParams params = *(searchParams *) args;

    std::cout << (&params.fen) << "\n";

    Search search = Search(params.fen, params.moves, params.settings);
    search.AlphaBeta(params.depth, params.alpha, params.beta, params.pline, params.pStats, params.IDline);

    return nullptr;
}

int ThreadManager::lazySMP(int ply, int alpha, int beta, LINE *pline, STATS *stats, LINE iterativeDeepeningLine) {

    /**
     * Helper threads search
     */
    for(long i = 0; i < settings.threads; i++){
        LINE line = {};

        searchParams params;
        params.depth = ply;
        params.alpha = alpha; params.beta = beta;
        params.pline = &line; params.pStats = &threadStats[i]; params.IDline = iterativeDeepeningLine;

        params.fen = this->fen;
        params.settings.threads = settings.threads;
        params.settings.depth = settings.depth;
        params.moves = moves;

        if(!(i % 2)){
            params.depth++;
        }

        pthread_create(&threads[i], nullptr, startThread, (void *) &params);
    }

    /**
     * Main thread
     */
    string fenTemp = fen;
    Search mainSearch = Search("startpos", moves, settings);
    mainSearch.AlphaBeta(ply, alpha, beta, pline, stats, iterativeDeepeningLine);

    /** Exit the other threads */
    exitFlag.store(true);

    for(int i = 0; i < settings.threads; i++){
        pthread_join(threads[i], nullptr);
        stats->totalNodes += threadStats[i].totalNodes;
        stats->quiescentNodes += threadStats[i].quiescentNodes;
    }
    exitFlag.store(false);

    return 0;
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

void ThreadManager::setSettings(Settings newSettings) {
    settings.threads = newSettings.threads;
    settings.depth = newSettings.depth;
}


