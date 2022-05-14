#include <cstdio>
#include "Client.h"
#include "parameters.h"

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
