#include <cstdio>
#include <unistd.h>
#include "Group.h"

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

void destroyGroup(Group* group) {
    group->table = NULL;
    pthread_mutex_destroy(&group->mutex);
    pthread_cond_destroy(&group->cond);
    delete [] group->clientThreads;
    delete group;
}
