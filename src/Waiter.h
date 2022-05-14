#ifndef ZSO_PR_1_WAITER_H
#define ZSO_PR_1_WAITER_H

#include "Group.h"

void createWaiterThreads(pthread_t* &waiters, int numOfWaiters, void* callbackFn(void* arg));

void joinWaiterThreads(pthread_t* &waiters, size_t size);

void seatGroupByTable(Group* group, Table* table);

void lookForFreeTableAndSeatGroupByIt(Table** tables, pthread_mutex_t &tablesMutex, pthread_cond_t &tablesCond, int waiterNumber, Group* group);

#endif
