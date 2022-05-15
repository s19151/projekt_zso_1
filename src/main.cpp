#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <queue>
#include "parameters.h"
#include "Waiter.h"
#include "Client.h"
#include "Group.h"

using namespace std;

bool programFinished;

Table** tables;

queue<Group*> restaurantQueue;
pthread_mutex_t queueMutex;
pthread_cond_t queueCond;

void takeCareOfGroupsInRestaurantQueue(int waiterNumber) {
    pthread_mutex_lock(&queueMutex);
    while (restaurantQueue.empty() && !programFinished) {
        #ifdef SLEEPS_AND_PRINTS
        printf("Waiter %d awaits clients\n", waiterNumber);
        #endif
        pthread_cond_wait(&queueCond, &queueMutex);
    }
    if(!programFinished) {
        Group* group = restaurantQueue.front();
        restaurantQueue.pop();
        pthread_mutex_unlock(&queueMutex);

        takeCareOfGroup(
            waiterNumber,
            group,
            tables,
            NUM_OF_TABLES
        );
    } else {
        pthread_mutex_unlock(&queueMutex);
    }
}

void* waiterThreadFn(void* arg) {
    int* waiterNumber = (int*)arg;

    while (!programFinished) {
        takeCareOfGroupsInRestaurantQueue(*waiterNumber);
        #ifdef SLEEPS_AND_PRINTS
        sleep(SLEEP_TIME);
        #endif
    }

    delete waiterNumber;
    return NULL;
}

void* clientThreadFn(void* arg) {
    #ifdef SLEEPS_AND_PRINTS
    sleep(EAT_TIME);
    #endif
    return NULL;
}

void* groupThreadFn(void* arg) {
    Group* group = (Group*)arg;

    enterRestaurantQueue(group, restaurantQueue, queueMutex, queueCond);
    waitForTable(group);
    createClientThreads(group, clientThreadFn);
    joinClientThreads(group);
    leaveRestaurant(group);

    return NULL;
}

void stopProgram() {
    programFinished = true;
    pthread_mutex_lock(&queueMutex);
    pthread_cond_broadcast(&queueCond);
    pthread_mutex_unlock(&queueMutex);
}

void initializePthreadVariables() {
    pthread_mutex_init(&queueMutex, NULL);
    pthread_cond_init(&queueCond, NULL);
}

void destroyPthreadVariables() {
    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&queueCond);
}

void projekt_zso(int numOfTables, int NumOfSeats, int numOfGroups, int maxGroupSize, int numOfWaiters) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Restaurant opens\n");
    #endif

    initializePthreadVariables();
    tables = createTables(numOfTables, NumOfSeats);
    pthread_t* waiters = new pthread_t[numOfWaiters];
    Group** groups = createGroups(numOfGroups, maxGroupSize);
    programFinished = false;

    createWaiterThreads(waiters, numOfWaiters, waiterThreadFn);
    createGroupThreads(groups, numOfGroups, groupThreadFn);
    joinGroupThreads(groups, numOfGroups);

    stopProgram();
    joinWaiterThreads(waiters, numOfWaiters);

    destroyTables(tables, numOfTables);
    destroyPthreadVariables();

    delete [] waiters;
    delete [] groups;

    #ifdef SLEEPS_AND_PRINTS
    printf("Restaurant closes\n");
    #endif
}

int main(int argc, char* argv[]) {
    for (int i = 0; i < NUM_OF_LOOPS; i++) {
        #ifdef SLEEPS_AND_PRINTS
        printf("--------- Program loop: %d ---------\n", i+1);
        #endif
        projekt_zso(
            NUM_OF_TABLES,
            MAX_SEATS_PER_TABLE,
            NUM_OF_GROUPS,
            MAX_GROUP_SIZE,
            NUM_OF_WAITERS
        );
    }

    return 0;
}

