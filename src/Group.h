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

int getNeededNumOfSeats(Group* group);

Group* createGroup(int id, size_t size, bool pair);

Group** createGroups(int numOfGroups, int maxGroupSize);

void destroyGroup(Group* group);

void createGroupThreads(Group** groups, size_t size, void* callbackFn(void* arg));

void joinGroupThreads(Group** groups, size_t size);

void enterRestaurantQueue(Group* group, queue<Group*> &queue, pthread_mutex_t &queueMutex, pthread_cond_t &queueCond);

void waitForTable(Group* group);

void leaveRestaurant(Group* group);

#endif
