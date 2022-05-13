#ifndef ZSO_PR_1_TABLE_H
#define ZSO_PR_1_TABLE_H

#include <pthread.h>

struct Table {
    int id;
    int availableSeats;
    int takenSeats;
    int leaveVotes;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

Table* createTable(int id, int numOfSeats);

void destroyTable(Table* table);

#endif
