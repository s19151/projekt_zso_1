#include <cstdio>
#include "Table.h"

Table* createTable(int id, int numOfSeats) {
    Table* table = new Table();

    table->id = id;
    table->availableSeats = numOfSeats;
    table->takenSeats = 0;
    table->leaveVotes = 0;
    pthread_mutex_init(&table->mutex, NULL);
    pthread_cond_init(&table->cond, NULL);

    return table;
}

Table** createTables(int numOfTables, int numOfSeats) {
    Table** tables = new Table*[numOfTables];

    #ifdef SLEEPS_AND_PRINTS
    printf("Creating tables\n");
    #endif
    for (int i = 0; i < numOfTables; i++) {
        tables[i] = createTable(i+1, numOfSeats);
    }

    return tables;
}

void destroyTable(Table* table) {
    pthread_mutex_destroy(&table->mutex);
    pthread_cond_destroy(&table->cond);
    delete table;
}

void destroyTables(Table** tables, int numOfTables) {
    for (int i = 0; i < numOfTables; i++) {
        destroyTable(tables[i]);
    }
    delete [] tables;
}

int findFreeTableIndex(Table** tables, size_t size, int neededNumOfSeats) {
    int index;
    bool indexFound = false;
    for (index = 0; index < size && !indexFound;) {
        index++;
        indexFound = tables[index]->availableSeats >= neededNumOfSeats;
    }
    return indexFound ? index : -1;
}
