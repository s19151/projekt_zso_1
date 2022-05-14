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

Table** createTables(int numOfTables, int numOfSeats);

void destroyTable(Table* table);

void destroyTables(Table** tables, int numOfTables);

int findFreeTableIndex(Table** tables, size_t size, int neededNumOfSeats);

#endif
