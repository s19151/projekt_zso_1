#include <queue>
#include <pthread.h>
#include <cstdio>
#include "Waiter.h"
#include "parameters.h"

void createWaiterThreads(pthread_t* &waiters, int numOfWaiters, void* callbackFn(void* arg)) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Creating waiter threads\n");
    #endif
    for (int i = 0; i < numOfWaiters; i++) {
        pthread_t thread;
        waiters[i] = thread;
        int* waiterNumber = new int(i+1);
        if (pthread_create(&waiters[i], NULL, callbackFn, waiterNumber) != 0) {
            #ifdef SLEEPS_AND_PRINTS
            perror("Failed to create waiter thread");
            #endif
        }
    }
}

void joinWaiterThreads(pthread_t* &waiters, size_t size) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Joining waiter threads\n");
    #endif
    for (int i = 0; i < size; i++) {
        if (pthread_join(waiters[i], NULL) != 0) {
            #ifdef SLEEPS_AND_PRINTS
            perror("Failed to join thread");
            #endif
        }
    }
    #ifdef SLEEPS_AND_PRINTS
    printf("Waiter threads finished\n");
    #endif
}

void seatGroupByTable(Group* group, Table* table) {
    pthread_mutex_lock(&table->mutex);
    table->takenSeats = table->takenSeats + group->size;
    pthread_mutex_unlock(&table->mutex);

    pthread_mutex_lock(&group->mutex);
    group->satByTable = true;
    group->table = table;
    pthread_mutex_unlock(&group->mutex);
    pthread_cond_signal(&group->cond);
}

void lookForFreeTableAndSeatGroupByIt(Table** tables, pthread_mutex_t &tablesMutex, pthread_cond_t &tablesCond, int waiterNumber, Group* group) {
    pthread_mutex_lock(&tablesMutex);
    int neededNumOfSeats = group->size == PAIR_SIZE && group->pair ? MAX_SEATS_PER_TABLE : group->size;
    int tableNumber;
    while ((tableNumber = findFreeTableIndex(tables, TABLE_STATES, neededNumOfSeats)) == -1) {
        #ifdef SLEEPS_AND_PRINTS
        printf("Waiter %d awaits free table\n", waiterNumber);
        #endif
        pthread_cond_wait(&tablesCond, &tablesMutex);
    }

    Table* table = tables[tableNumber];
    #ifdef SLEEPS_AND_PRINTS
    printf("Waiter %d seats Group %d at table %d\n", waiterNumber, group->id, table->id);
    #endif
    seatGroupByTable(group, table);

    pthread_mutex_unlock(&tablesMutex);
    pthread_cond_broadcast(&tablesCond);
}
