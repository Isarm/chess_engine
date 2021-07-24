//
// Created by isar on 24/07/2021.
//

#ifndef ENGINE_THREAD_H
#define ENGINE_THREAD_H

#include "evaluate.h"
#include "definitions.h"

class Thread {
public:
    explicit Thread(int id);
    void go(Evaluate eval, SearchParams params) const;

private:
    Evaluate evaluate;
    int id;

};


#endif //ENGINE_THREAD_H
