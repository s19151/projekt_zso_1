#include <cstdio>
#include "Group.h"
#include "parameters.h"

int getNeededNumOfSeats(Group* group) {
    return group->size == PAIR_SIZE && group->pair ? MAX_GROUP_SIZE : group->size;
}

Group* createGroup(int id, size_t size, bool pair) {
    Group* group = new Group();

    pthread_t thread;
    group->thread = thread;
    pthread_mutex_init(&group->mutex, NULL);
    pthread_cond_init(&group->cond, NULL);
    group->id = id;
    group->size = size;
    group->clientThreads = new pthread_t[group->size];
    for (int i = 0; i < group->size; i++) {
        pthread_t client;
        group->clientThreads[i] = client;
    }
    group->pair = pair;
    group->satByTable = false;

    return group;
}

Group** createGroups(int numOfGroups, int maxGroupSize) {
    Group** groups = new Group*[numOfGroups];

    #ifdef SLEEPS_AND_PRINTS
    printf("Creating client groups\n");
    #endif
    bool pair = false;
    for (int i = 0; i < numOfGroups; i++) {
        int mod = i % maxGroupSize;
        int size = mod == 0 ? maxGroupSize : mod;
        groups[i] = createGroup(i+1, size, size == PAIR_SIZE && pair);
        pair = !pair;
    }

    return groups;
}

void destroyGroup(Group* group) {
    group->table = NULL;
    pthread_mutex_destroy(&group->mutex);
    pthread_cond_destroy(&group->cond);
    delete [] group->clientThreads;
    delete group;
}

void createGroupThreads(Group** groups, size_t size, void* callbackFn(void* arg)) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Creating group threads\n");
    #endif
    for (int i = 0; i < size; i++) {
        if (pthread_create(&groups[i]->thread, NULL, callbackFn, groups[i]) != 0) {
            #ifdef SLEEPS_AND_PRINTS
            perror("Failed to create Group thread");
            #endif
        }
    }
}

void joinGroupThreads(Group** groups, size_t size) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Joining group threads\n");
    #endif
    for (int i = 0; i < size; i++) {
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

void enterRestaurantQueue(Group* group, queue<Group*> &queue, pthread_mutex_t &queueMutex, pthread_cond_t &queueCond) {
    pthread_mutex_lock(&queueMutex);
    #ifdef SLEEPS_AND_PRINTS
    printf("Group %d enters queue\n", group->id);
    #endif
    queue.push(group);
    pthread_cond_broadcast(&queueCond);
    pthread_mutex_unlock(&queueMutex);
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

void leaveRestaurant(Group* group) {
    #ifdef SLEEPS_AND_PRINTS
    printf("Group's %d clients finished eating\n", group->id);
    #endif
    pthread_mutex_lock(&group->table->mutex);
    int takenSeats = getNeededNumOfSeats(group);
    group->table->leaveVotes += takenSeats;
    pthread_mutex_unlock(&group->table->mutex);

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
    group->table->availableSeats += takenSeats;
    group->table->takenSeats -= takenSeats;
    group->table->leaveVotes -= takenSeats;
    pthread_cond_broadcast(&group->table->cond);
    pthread_mutex_unlock(&group->table->mutex);
}
