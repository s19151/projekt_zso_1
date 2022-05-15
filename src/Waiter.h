#ifndef ZSO_PR_1_WAITER_H
#define ZSO_PR_1_WAITER_H

#include "Group.h"

void createWaiterThreads(pthread_t* &waiters, int numOfWaiters, void* callbackFn(void* arg));

void joinWaiterThreads(pthread_t* &waiters, size_t size);

void seatGroupByTable(int waiterNumber, Group* group, Table* table);

bool findTableAndSeatGroupByIt(
    int waiterNumber,
    Group* group,
    int neededNumOfSeats,
    Table** tables,
    size_t size
);

void takeCareOfGroup(
    int waiterNumber,
    Group* group,
    Table** tables,
    size_t size
);

#endif
