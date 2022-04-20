#ifndef LAB3_CALTRAIN_H
#define LAB3_CALTRAIN_H

#endif //LAB3_CALTRAIN_H

#include <pthread.h>

struct station {
    // FILL ME IN
    int availableSeats;     //Number of free seats on the train, defaults to zero of no train arrived
    int waitingPassengers;  //Number of passengers waiting for a train with free seats to arrive
    int boardingPassengers; //Number of passengers who found free seats and are currently boarding
    pthread_mutex_t mutex;  //For synchronization
    pthread_cond_t trainArrival;  //A flag for when a new train arrives so that waiting passenger should search for free seats
    pthread_cond_t boardingDone;  //A flag for when a train is fully boarded and ready to depart
};

//Initializer function
void station_init(struct station *station);

//Called when a new train arrives at the station with "count" be the number of free seats on the train
//Returns when all passengers board, or no more free seats are found, or no more passengers are waiting for the train
void station_load_train(struct station *station, int count);

//Called when a passenger is waiting for a train with available free seats
void station_wait_for_train(struct station *station);

//Called when a passenger found a free seat and is currently boarding
void station_on_board(struct station *station);