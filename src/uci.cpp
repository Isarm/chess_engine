
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


void UCI::start() {

    // output standard info
    std::cout << "id name Isar Engine\n";
    std::cout << "id author Isar Meijer\n";

    // hash table size can vary between 1MB and 2GB
    std::cout << "option name Hash type spin default 512 min 1 max 2048\n";

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

    // give the ready signal
    std::cout << "readyok\n";

    mainLoop();
}


void UCI::mainLoop(){
    std::string input;
    std::string fen;
    std::vector<string> moves;

    std::thread evaluation;

    bool threadStarted = false;

    Results results;
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
                evaluation.join();
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

            input.clear();
            Settings settings;
            settings.depth = 12;
            if(threadStarted){
                evaluation.join();
            }

            // only now check if TT is initialized
            if(!TT.size){
                TT.setSize(2048); // set default size
            }

            threadStarted = true;
            evaluation = std::thread{UCI::go, fen, moves, settings, std::ref(results)};
        }
    }
    evaluation.join();
}


void UCI::go(std::string fen, std::vector<std::string> moves, Settings settings, Results &results) {
    Evaluate evaluate = Evaluate(fen, moves, settings);
    results = evaluate.StartSearch();
    std::cout << "bestmove " << results.bestMove << "\n";
    std::cout.flush();
}

