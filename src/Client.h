#ifndef ZSO_PR_1_CLIENT_H
#define ZSO_PR_1_CLIENT_H

#include "Group.h"

void createClientThreads(Group* group, void* callbackFn(void* arg));

void joinClientThreads(Group* group);

#endif
