//
// Created by isar on 24/07/2021.
//

#include "thread.h"
#include "definitions.h"
#include "threadManager.h"
#include "mutex"
#include "useful.h"

std::mutex searchInfoMutex;
SearchInfo searchInfo;

Thread::Thread(int id, Settings settings) {
    this->id = id;
    this->settings = settings;
}

string Thread::search() {
    auto t1 = std::chrono::high_resolution_clock::now();

    evaluate = Evaluate(settings.fen, settings.moves);

    LINE PVline{};
    LINE previousBestLine{};
    STATS stats{};

    /** alpha and beta, with a little buffer to stop any overflowing */
    int alpha = std::numeric_limits<int>::min() + 10000;
    int beta = std::numeric_limits<int>::max() - 10000;

    int score;
    for(int iterativeDepth = 1; iterativeDepth <= settings.depth; iterativeDepth++) {
        PVline = {};
        while(true) {
            score = evaluate.AlphaBeta(iterativeDepth, alpha, beta, &PVline, &stats, previousBestLine);
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

        searchInfoMutex.lock();

        searchInfo.stats.totalNodes += stats.totalNodes;
        stats.totalNodes = 0;

        /** A different thread has found a line at this depth already */
        if(searchInfo.depth >= iterativeDepth){
            iterativeDepth = searchInfo.depth;
            /** Check if a majority is already searching the next depth */
            if(searchInfo.searchingAt[searchInfo.depth + 1] > (settings.threads + 1) / 2){
                iterativeDepth++; /** In that case jump ahead*/
            }
            searchInfo.searchingAt[iterativeDepth + 1]++;
            searchInfoMutex.unlock();
            continue;
        }

        /** This thread is the first to complete the search at this depth, so print and store the information */
        printinformation(milliseconds, score, PVline, searchInfo.stats, iterativeDepth);

        searchInfo.depth = iterativeDepth;
        searchInfo.searchingAt[iterativeDepth + 1]++;
        copyline(&PVline, &searchInfo.PVline);

        searchInfoMutex.unlock();

        if(abs(score) >= 1000000) {
            // this indicates that mate is found
            exitFlag.store(true);
            break;
        }
    }
    string bestmove;
    searchInfoMutex.lock();
    bestmove = moveToStrNotation(searchInfo.PVline.principalVariation[0]);
    searchInfoMutex.unlock();
    return bestmove;
}


void Thread::printinformation(int milliseconds, int score, LINE line, STATS stats, int depth) {
    string pv[100];

    std::cout << "info depth " << depth;
    std::cout << " time " << milliseconds;
    std::cout << " nodes " << stats.totalNodes;

    if (milliseconds != 0) {
        std::cout << " nps " << int(1000 * float(stats.totalNodes) / float(milliseconds));
    }
    std::cout << " score ";
    if (score >= 1000000) {
        // this indicates that mate is found
        int mateIn = int((depth - score + 1000001) / 2);
        std::cout << "mate " << mateIn << " pv ";
    } else if (score <= -1000000) {
        // this indicates that the engine is getting mated
        int mateIn = -int((depth + score + 1000001) / 2);
        std::cout << "mate " << mateIn << " pv ";
    } else {
        std::cout << "cp " << score << " pv ";
    }

    for (int i = 0; i < line.nmoves; i++) {
        pv[i] = moveToStrNotation(line.principalVariation[i]);
        std::cout << pv[i] << " ";
    }
//    std::cout << "\n";
    std::cout.flush();

//    std::cout << "quiescent nodes " << stats.quiescentNodes << "/" << stats.totalNodes << " total nodes\n";
//    std::cout << float(stats.quiescentNodes) / stats.totalNodes << "\n";

//    std::cout << stats.totalNodes - stats.quiescentNodes << " normal nodes\n";
//
//    std::cout << "table hits: " << stats.transpositionHits << "\n";

    std::cout << '\n';
    std::cout.flush();
}