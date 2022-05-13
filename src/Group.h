#ifndef ZSO_PR_1_GROUP_H
#define ZSO_PR_1_GROUP_H

#include <pthread.h>
#include <queue>
#include "Table.h"

using namespace std;

struct Group {
    int id;
    pthread_t thread;
    pthread_t* clientThreads;
    size_t size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool pair;
    bool satByTable;
    Table* table;
};

Group* createGroup(int id, size_t size, bool pair);

void destroyGroup(Group* group);

#endif
