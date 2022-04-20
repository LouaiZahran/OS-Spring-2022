#include <pthread.h>
#include <stdio.h>
#include "caltrain.h"

void
station_init(struct station *station)
{
    // FILL ME IN
    station->availableSeats = 0;
    station->waitingPassengers = 0;
    station->boardingPassengers = 0;
    pthread_mutex_init(&(station->mutex), NULL);
    pthread_cond_init(&(station->trainArrival), NULL);
    pthread_cond_init(&(station->boardingDone), NULL);
}

void
station_load_train(struct station *station, int count)
{
    // FILL ME IN
    pthread_mutex_lock(&(station->mutex));
    if(count == 0 || station->waitingPassengers == 0) {
        pthread_mutex_unlock(&(station->mutex));
        return;
    }
    station->availableSeats = count;
    pthread_cond_broadcast(&(station->trainArrival));
    pthread_cond_wait(&(station->boardingDone), &(station->mutex));
    station->availableSeats = 0;
    pthread_mutex_unlock(&(station->mutex));
}

void
station_wait_for_train(struct station *station)
{
    // FILL ME IN
    pthread_mutex_lock(&(station->mutex));
    station->waitingPassengers++;
    while(station->availableSeats == 0)
        pthread_cond_wait(&(station->trainArrival), &(station->mutex));
    station->availableSeats--;
    station->waitingPassengers--;
    station->boardingPassengers++;
    pthread_mutex_unlock(&(station->mutex));
}

void
station_on_board(struct station *station)
{
    // FILL ME IN
    pthread_mutex_lock(&(station->mutex));
    station->boardingPassengers--;
    if(station->boardingPassengers == 0 && (station->availableSeats == 0 || station->waitingPassengers == 0))
        pthread_cond_signal(&(station->boardingDone));
    pthread_mutex_unlock(&(station->mutex));
}