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

void destroyTable(Table* table) {
    pthread_mutex_destroy(&table->mutex);
    pthread_cond_destroy(&table->cond);
    delete table;
}
