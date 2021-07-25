
#include <iostream>
#include "uci.h"
#include "position.h"
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include "evaluate.h"
#include "transpositionTable.h"
#include "exitTimer.h"
#include "threadManager.h"


UCI::UCI() = default;

void UCI::start() {
    // output standard info
    std::cout << "id name Isar Engine\n";
    std::cout << "id author Isar Meijer\n";

    // hash table size can vary between 1MB and 8GB (although the upper bound is kind of arbitrary)
    std::cout << "option name Hash type spin default 512 min 1 max 8192\n";

    // uci is ready
    std::cout << "uciok\n";

    // wait for the isready command (ignore options for now)
    std::string input;


    while(true) {
        std::getline(std::cin, input);

        if(input == "isready"){
            break;
        }

        if (input.substr(0, input.find(' ')) == "setoption") {
            input.erase(0, input.find(' '));  // setoption
            input.erase(0, input.find(' '));  // name
            string optionType = input.substr(0, input.find(' '));
            input.erase(0, input.find(' '));

            if (optionType == "Hash") {
                input.erase(0, input.find(' ')); // Hash
                input.erase(0, input.find(' ')); // value
                unsigned value;
                try {
                    value = stoi(input);
                }
                catch (invalid_argument &exc) {
                    cout << "invalid hash table size: " << exc.what() << "\n";
                }
                // set size of transposition table
                TT.setSize(value);
            }

        }
    }

    Settings settings;
    settings.depth = MAX_DEPTH;
    settings.threads = 4;
    this->threadManager = ThreadManager(settings);

    // give the ready signal
    std::cout << "readyok\n";

    mainLoop();
}


void UCI::mainLoop(){
    std::string input;
    std::string fen;
    std::vector<string> moves;

    std::thread evaluation;
    std::thread exitTimer;

    bool threadStarted = false;

    while(true){

        std::getline(std::cin, input);

        if(input == "quit"){
            input.clear();
            break;
        }

        if(input == "ucinewgame"){
            input.clear();
            // nothing for now
        }

        if(input == "stop"){
            if(threadStarted){
                timeFlag.store(true);
                evaluation.join();
                exitTimer.join();
                threadStarted = false;
            }
            // stop the analysis thread
        }



        // set up the position
        if(input.substr(0, input.find(' ')) == "position"){
            moves = {}; // reset moves
            // remove position from the string
            input.erase(0, input.find(' ') + 1);

            // check for startposition
            if(input.substr(0, input.find(' ')) == "startpos"){
                fen = "startpos";
                input.erase(0, input.find(' ') + 1 );
            }
            // extract fen
            else if(input.substr(0, input.find(' ')) == "fen"){
                input.erase(0, input.find(' ') + 1);
                if(input.find("moves")){
                    fen = input.substr(0, input.find("moves"));
                    input.erase(0, input.find("moves"));
                }
                else{
                    fen = input;
                }
            }
            if(input.find("moves") != std::string::npos){
                // remove moves keyword
                input.erase(0, input.find(' ') + 1);
                int pos = 0;
                while((pos = input.find(' ')) != std::string::npos){
                    moves.push_back(input.substr(0, pos));
                    input.erase(0, pos + 1);
                }
                moves.push_back(input);
            }
            input.clear();
            std::cout << fen << "\n";
        }


        if(input.substr(0, input.find(' ')) == "go"){
            input.erase(0, input.find(' ') + 1);

            int time = 10000;

            if(input.substr(0, input.find(' ')) == "movetime"){
                input.erase(0, input.find(' ') + 1);
                time = stoi(input.substr(0, input.find(' ')));
            }

            input.clear();

            if(threadStarted){
                evaluation.join();
                exitTimer.join();
            }

            // only now check if TT is initialized
            if(!TT.size){
                TT.setSize(4096); // set default size
            }

            threadStarted = true;

            exitTimer = std::thread(&UCI::timer, this, time);
            evaluation = std::thread{&UCI::go, this, fen, moves};
        }
    }
    // wait for the evaluation thread to finish so the program can exit safely
    evaluation.join();
    exitTimer.join();
}

void UCI::timer(int ms){
    timerLoop(ms);
}

void UCI::go(std::string fen, std::vector<std::string> moves) {
    threadManager.StartSearch(fen, moves);
}


