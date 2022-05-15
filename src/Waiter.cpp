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
    int takenSeats = group->size == PAIR_SIZE && group->pair ? MAX_SEATS_PER_TABLE : group->size;
    table->takenSeats += takenSeats;
    table->availableSeats -= takenSeats;

    pthread_mutex_lock(&group->mutex);
    group->satByTable = true;
    group->table = table;
    pthread_cond_broadcast(&group->cond);
    pthread_mutex_unlock(&group->mutex);
}

void lookForFreeTableAndSeatGroupByIt(
        int waiterNumber,
        Group* group,
        Table** tables,
        size_t size,
        pthread_mutex_t &tablesMutex,
        pthread_cond_t &tablesCond
    ) {
    pthread_mutex_lock(&tablesMutex);
    int neededNumOfSeats = group->size == PAIR_SIZE && group->pair ? MAX_SEATS_PER_TABLE : group->size;
    bool indexFound = false;
    while (!indexFound) {
        #ifdef SLEEPS_AND_PRINTS
        printf("Waiter %d searches for table for Group %d\n", waiterNumber, group->id);
        #endif
        for (int i = 0; i < size && !indexFound; i++) {
            pthread_mutex_lock(&tables[i]->mutex);
            if (
                tables[i]->availableSeats >= neededNumOfSeats &&
                (tables[i]->leaveVotes == 0 ||
                tables[i]->takenSeats != tables[i]->leaveVotes)
            ) {
                indexFound = true;
                #ifdef SLEEPS_AND_PRINTS
                printf("Waiter %d seats Group %d at table %d\n", waiterNumber, group->id, tables[i]->id);
                #endif
                seatGroupByTable(group, tables[i]);
            }
            pthread_mutex_unlock(&tables[i]->mutex);
        }
        if (!indexFound) {
            #ifdef SLEEPS_AND_PRINTS
            printf("Waiter %d awaits for table for Group %d\n", waiterNumber, group->id);
            #endif
            pthread_cond_wait(&tablesCond, &tablesMutex);
        }
    }
    pthread_mutex_unlock(&tablesMutex);
}
