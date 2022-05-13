#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <queue>
#include "Group.h"

#define SLEEPS_AND_PRINTS

using namespace std;

const int MAX_GROUP_SIZE = 4;
const int PAIR_SIZE = 2;
const int MAX_SEATS_PER_TABLE = 4;
const int TABLE_STATES = MAX_SEATS_PER_TABLE + 1;

const int NUM_OF_WAITERS = 4;
const int NUM_OF_TABLES = 10;
const int NUM_OF_GROUPS = 20;
const int SLEEP_TIME = 1;
const int EAT_TIME = 5;

const int NUM_OF_LOOPS = 10;

bool programFinished;

Table* tables[NUM_OF_TABLES];
pthread_mutex_t tablesMutex;
pthread_cond_t tablesCond;

queue<Group*> restaurantQueue;
pthread_mutex_t queueMutex;
pthread_cond_t queueCond;

int findFreeTableIndex(int neededNumOfSeats) {
    int index;
    bool indexFound = false;
    for (index = 0; index < TABLE_STATES && !indexFound;) {
        index++;
        indexFound = tables[index]->availableSeats >= neededNumOfSeats;
    }
    return indexFound ? index : -1;
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

void lookForFreeTableAndSeatGroupByIt(int waiterNumber, Group* group) {
    pthread_mutex_lock(&tablesMutex);
    int neededNumOfSeats = group->size == PAIR_SIZE && group->pair ? MAX_SEATS_PER_TABLE : group->size;
    int tableNumber;
    while ((tableNumber = findFreeTableIndex(neededNumOfSeats)) == -1) {
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
}

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

        lookForFreeTableAndSeatGroupByIt(waiterNumber, group);

        pthread_mutex_unlock(&tablesMutex);
        pthread_cond_broadcast(&tablesCond);
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

void enterRestaurantQueue(Group* group) {
    pthread_mutex_lock(&queueMutex);
    #ifdef SLEEPS_AND_PRINTS
    printf("Group %d enters queue\n", group->id);
    #endif
    restaurantQueue.push(group);
    pthread_mutex_unlock(&queueMutex);
    pthread_cond_broadcast(&queueCond);
}

void waitForTable(Group* group) {
    pthread_mutex_lock(&group->mutex);
    while (!group->satByTable) {
        pthread_cond_wait(&group->cond, &group->mutex);
    }
    #ifdef SLEEPS_AND_PRINTS
    printf("Group %d sat by table\n", group->id);
    #endif
    pthread_mutex_unlock(&group->mutex);
}

void* clientThreadFn(void* arg) {
    #ifdef SLEEPS_AND_PRINTS
    sleep(EAT_TIME);
    #endif
    return NULL;
}

void createClientThreads(Group* group, void* callbackFn(void* arg)) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Group's %d clients start eating\n", group->id);
    #endif
    for (int i = 0; i < group->size; i++) {
        if (pthread_create(&group->clientThreads[i], NULL, callbackFn, NULL) != 0) {
            #ifdef SLEEPS_AND_PRINTS
            perror("Failed to create client thread");
            #endif
        }
    }
}

void joinClientThreads(Group* group) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Group %d waits for clients to finish eating\n", group->id);
    #endif
    for (int i = 0; i < group->size; i++) {
        if (pthread_join(group->clientThreads[i], NULL) != 0) {
            #ifdef SLEEPS_AND_PRINTS
            perror("Failed to join thread");
            #endif
        }
    }
}

void leaveRestaurant(Group* group) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Group's %d clients finished eating\n", group->id);
    #endif
    pthread_mutex_lock(&group->table->mutex);
    group->table->leaveVotes = group->table->leaveVotes + group->size;
    pthread_mutex_unlock(&group->table->mutex);
    pthread_cond_broadcast(&group->table->cond);

    #ifdef SLEEPS_AND_PRINTS
    printf("Group %d waits for other groups to stop eating\n", group->id);
    #endif
    pthread_mutex_lock(&group->table->mutex);
    while (group->table->leaveVotes != group->table->takenSeats) {
        pthread_cond_wait(&group->table->cond, &group->table->mutex);
    }

    #ifdef SLEEPS_AND_PRINTS
    printf("Group %d leaves restaurant\n", group->id);
    #endif
    group->table->availableSeats += group->size;
    group->table->takenSeats -= group->size;
    group->table->leaveVotes -= group->size;
    pthread_mutex_unlock(&group->table->mutex);
}

void* groupThreadFn(void* arg) {
    Group* group = (Group*)arg;

    enterRestaurantQueue(group);
    waitForTable(group);
    createClientThreads(group, clientThreadFn);
    joinClientThreads(group);
    leaveRestaurant(group);

    return NULL;
}

void createTables(int numOfTables, int numOfSeats) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Creating tables\n");
    #endif
    for (int i = 0; i < numOfTables; i++) {
        tables[i] = createTable(i+1, numOfSeats);
    }
}

void createGroups(vector<Group*> &groups, int numOfGroups) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Creating client groups\n");
    #endif
    bool pair = false;
    for (int i = 0; i < numOfGroups; i++) {
        int size = (i % MAX_GROUP_SIZE) + 1;
        groups.push_back(createGroup(i+1, size, size == PAIR_SIZE && pair));
        pair = !pair;
    }
}

void createWaiterThreads(vector<pthread_t> &waiters, int numOfWaiters, void* callbackFn(void* arg)) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Creating waiter threads\n");
    #endif
    for (int i = 0; i < numOfWaiters; i++) {
        pthread_t thread;
        waiters.push_back(thread);
        int* waiterNumber = new int(i+1);
        if (pthread_create(&waiters[i], NULL, callbackFn, waiterNumber) != 0) {
            #ifdef SLEEPS_AND_PRINTS
            perror("Failed to create waiter thread");
            #endif
        }
    }
}

void createGroupThreads(vector<Group*> &groups, void* callbackFn(void* arg)) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Creating group threads\n");
    #endif
    for (int i = 0; i < groups.size(); i++) {
        if (pthread_create(&groups[i]->thread, NULL, callbackFn, groups[i]) != 0) {
            #ifdef SLEEPS_AND_PRINTS
            perror("Failed to create Group thread");
            #endif
        }
    }
}

void joinWaiterThreads(vector<pthread_t> &waiters) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Joining waiter threads\n");
    #endif
    for (pthread_t waiter : waiters) {
        if (pthread_join(waiter, NULL) != 0) {
            #ifdef SLEEPS_AND_PRINTS
            perror("Failed to join thread");
            #endif
        }
    }
    #ifdef SLEEPS_AND_PRINTS
    printf("Waiter threads finished\n");
    #endif
}

void joinGroupThreads(vector<Group*> &groups) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Joining group threads\n");
    #endif
    for (int i = 0; i < groups.size(); i++) {
        if (pthread_join(groups[i]->thread, NULL) != 0) {
            #ifdef SLEEPS_AND_PRINTS
            perror("Failed to join thread");
            #endif
        }
        destroyGroup(groups[i]);
    }
    #ifdef SLEEPS_AND_PRINTS
    printf("Group threads finished\n");
    #endif
}

void stopProgram() {
    programFinished = true;
    pthread_cond_broadcast(&queueCond);
}

void initializePthreadVariables() {
    pthread_mutex_init(&tablesMutex, NULL);
    pthread_mutex_init(&queueMutex, NULL);
    pthread_cond_init(&queueCond, NULL);
    pthread_cond_init(&tablesCond, NULL);
}

void destroyPthreadVariables() {
    pthread_mutex_destroy(&tablesMutex);
    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&queueCond);
    pthread_cond_destroy(&tablesCond);
}

void destroyTables(int numOfTables) {
    for (int i = 0; i < numOfTables; i++) {
        destroyTable(tables[i]);
    }
}

void projekt_zso(int numOfTables, int NumOfSeats, int numOfGroups, int numOfWaiters) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Restaurant opens\n");
    #endif

    initializePthreadVariables();
    vector<pthread_t> waiters;
    vector<Group*> groups;
    programFinished = false;
    createTables(numOfTables, NumOfSeats);
    createGroups(groups, numOfGroups);

    createWaiterThreads(waiters, numOfWaiters, waiterThreadFn);
    createGroupThreads(groups, groupThreadFn);
    joinGroupThreads(groups);

    stopProgram();
    joinWaiterThreads(waiters);

    destroyTables(numOfTables);
    destroyPthreadVariables();

    #ifdef SLEEPS_AND_PRINTS
    printf("Restaurant closes\n");
    #endif
}

int main(int argc, char* argv[]) {
    for (int i = 0; i < NUM_OF_LOOPS; i++) {
        #ifdef SLEEPS_AND_PRINTS
        printf("Program loop: %d\n", i+1);
        #endif
        projekt_zso(NUM_OF_TABLES, MAX_SEATS_PER_TABLE, NUM_OF_GROUPS, NUM_OF_WAITERS);
    }

    return 0;
}

