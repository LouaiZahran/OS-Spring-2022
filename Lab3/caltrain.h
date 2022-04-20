#ifndef LAB3_CALTRAIN_H
#define LAB3_CALTRAIN_H

#endif //LAB3_CALTRAIN_H

#include <pthread.h>

struct station {
    // FILL ME IN
    int availableSeats;
    int waitingPassengers;
    int boardingPassengers;
    pthread_mutex_t mutex;
    pthread_cond_t trainArrival;
    pthread_cond_t boardingDone;
};

void station_init(struct station *station);

void station_load_train(struct station *station, int count);

void station_wait_for_train(struct station *station);

void station_on_board(struct station *station);