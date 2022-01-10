// Grace Biemans & Nick Heleta
// geb965 & nwh397

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "petgroomsynch.h"

int stations;
int availStations;
int numDogsGrooming;
int numCatsGrooming;
int dogsWaiting;
int catsWaiting;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/**
 * Initializes variables for the solution
 * @param numstations : the number of grooming stations in the facility
 * @return: 0 on success, -1 on failure
 */
int petgroom_init(int numstations) {
    if (numstations == 0) {
        return -1;
    }
    stations = numstations;
    availStations = numstations;
    numCatsGrooming = 0;
    numDogsGrooming = 0;
    catsWaiting = 0;
    dogsWaiting = 0;
    return 0;
}

/**
 * Deals with the arrival of a new pet to the facility
 * @param pet: the new pet
 * @return: 0 on success, -1 on failure
 */
int newpet(pet_t pet) {
    if (pet < 1 || pet > 3) {
        return -1;
    }

    pthread_mutex_lock(&lock);
    int waited = 0;
    printf("1. Adding new pet of type %d ...\n", pet);

    // The calling thread should block until the new pet can be allocated a grooming station
    if (pet == 1) {

        // if the pet is a cat, wait until there are no dogs in the stations
        printf("  Checking if need to wait:\n     Dogs grooming: %d - stations: %d - dogs waiting: %d - cats waiting: %d\n", numDogsGrooming, availStations, dogsWaiting, catsWaiting);
        while (availStations == 0 || numDogsGrooming != 0 || (dogsWaiting > catsWaiting && numCatsGrooming == 0)) {
            printf("Waiting...\n\n");
            if (waited == 0) { // makes sure each pet only adds one, not one each time checked
                catsWaiting += 1;
            }
            waited = 1;
            pthread_cond_wait(&cond, &lock);
        }

        if (waited == 1) {
            printf("Removing a cat from waiting list.....\n");
            catsWaiting -= 1;
        }
        numCatsGrooming += 1;
    }

    else if (pet == 2) {

        // if the pet is a dog, wait until there are no cats in the station
        printf("  Checking if need to wait:\n     Cats grooming: %d - stations: %d - dogs waiting: %d - cats waiting: %d\n", numCatsGrooming, availStations, dogsWaiting, catsWaiting);
        while (availStations == 0 || numCatsGrooming != 0 || (catsWaiting > dogsWaiting && numDogsGrooming == 0)) {
            printf("Waiting...\n\n");
            if (waited == 0) {  // makes sure each pet only adds one, not one each time checked
                dogsWaiting += 1;
            }
            waited = 1;
            pthread_cond_wait(&cond, &lock);
        }

        if (waited == 1) {
            printf("Removing a dog from waiting list.....\n");
            dogsWaiting -= 1;
        }
        numDogsGrooming += 1;
    }

    else {  // pet is of type other
        printf("  Checking if other needs to wait\n");
        while (availStations == 0) {
            printf("Waiting...\n\n");
            pthread_cond_wait(&cond, &lock);
        }
    }

    // the pet takes a grooming station
    availStations -= 1;
    printf("  Pet gets grooming station:\n     Dogs grooming: %d - Cats grooming: %d - stations: %d - dogs waiting: %d - cats waiting: %d\n", numDogsGrooming, numCatsGrooming, availStations, dogsWaiting, catsWaiting);

    printf("2. Pet of type %d added\n", pet);
    pthread_mutex_unlock(&lock);
    return 0;
}

/**
 * Called when pet is done with the grooming station, the station is now free
 * @param pet: the pet that is done being groomed
 * @return: 0 on success, -1 on failure
 */
int petdone(pet_t pet) {
    if (pet < 1 || pet > 3) {
        return -1;
    }

    pthread_mutex_lock(&lock);

    if (pet == 1) { // if the pet is cat
        numCatsGrooming -= 1;
    }
    else if (pet == 2) { // if the pet is a dog
        numDogsGrooming -= 1;
    }

    // pet is done with the grooming station
    availStations += 1;

    pthread_cond_signal(&cond);

    printf("Removed pet of type %d\n\n", pet);
    pthread_mutex_unlock(&lock);
    return 0;
}

/**
 * Performs required data cleanup actions. Makes it possible to reinitialize and groom another pet
 * @return: 0 on success, -1 on failure
 */
int petgroom_done() {
    if (availStations != stations) {
        return -1;
    }
    printf("Petgroom done\n");
    return 0;
}

/**
 * @param arg: first index is the pet type and second index is the groom time
 */
void* mythread(int arg[]) {
    int pet = arg[0];

    newpet(pet);

    printf("Groom for %d seconds\n\n", arg[1]);
    sleep(arg[1]);

    petdone(pet);

    return NULL;
}

int main() {

    // TEST ONE: RULE 2: Only one pet can occupy a grooming station at a time
    // One station, multiple pets that will have to wait
    printf("TEST ONE\n");
    int numstations = 1;
    int initStations = petgroom_init(numstations);
    if (initStations == -1) {
        perror("Error, stations initialization failed (petgroom_init())");
    }

    pthread_t p1, p2, p3;
    pthread_t threads[3] = {p1 = NULL, p2 = NULL, p3 = NULL};
    int types[3] = {3, 2, 1};
    int sleepTime[3] = {3, 1, 2};

    int result;
    int args[2];
    for (int i = 0; i < 3; i++) {
        args[0] = types[i];
        args[1] = sleepTime[i];
        result = pthread_create(&threads[i], NULL, (void *(*)(void *)) mythread, args);
        sleep(1);
        if (result != 0) {
            perror("Failed thread creation");
        }
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    petgroom_done();
    printf("\n\n\n");


    // TEST TWO: RULE 1: Cats and dogs can't be groomed at the same time
    // 2 stations and 2 pets. There will be enough stations for them both to get stations right away, but they don't because one is a cat and one is a dog
    printf("TEST TWO\n");
    numstations = 2;
    initStations = petgroom_init(numstations);
    if (initStations == -1) {
        perror("Error, stations initialization failed (petgroom_init())");
    }

    pthread_t p4, p5;
    pthread_t threads2[2] = {p4 = NULL, p5 = NULL};
    int types2[2] = {1, 2};
    int sleepTime2[2] = {3, 3};

    int arg2[2];
    for (int i = 0; i < 2; i++) {
        arg2[0] = types2[i];
        arg2[1] = sleepTime2[i];
        result = pthread_create(&threads2[i], NULL, (void *(*)(void *)) mythread, arg2);
        sleep(1);
        if (result != 0) {
            perror("Failed thread creation");
        }
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(threads2[i], NULL);
    }

    petgroom_done();
    printf("\n\n\n");


    // TEST THREE: RULE 1: Cats and dogs can't be groomed at the same time
    // 2 stations and 2 pets. There will be enough stations for them both to get stations right away, but they don't because one is a cat and one is a dog
    printf("TEST THREE\n");
    numstations = 2;
    initStations = petgroom_init(numstations);
    if (initStations == -1) {
        perror("Error, stations initialization failed (petgroom_init())");
    }

    pthread_t p6, p7;
    pthread_t threads3[2] = {p6 = NULL, p7 = NULL};
    int types3[2] = {2, 1};
    int sleepTime3[2] = {3, 3};

    int arg3[2];
    for (int i = 0; i < 2; i++) {
        arg3[0] = types3[i];
        arg3[1] = sleepTime3[i];
        result = pthread_create(&threads3[i], NULL, (void *(*)(void *)) mythread, arg3);
        sleep(1);
        if (result != 0) {
            perror("Failed thread creation");
        }
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(threads3[i], NULL);
    }

    petgroom_done();
    printf("\n\n\n");


    // TEST FOUR: RULE 2: Only one pet can occupy a grooming station at a time
    // two stations, three pets that could all be groomed at the same time if there were enough stations, so one will have to wait for a station to open
    printf("TEST FOUR\n");
    numstations = 2;
    initStations = petgroom_init(numstations);
    if (initStations == -1) {
        perror("Error, stations initialization failed (petgroom_init())");
    }

    pthread_t p8, p9, p10;
    pthread_t threads4[3] = {p8 = NULL, p9 = NULL, p10 = NULL};
    int types4[3] = {2, 3, 3};
    int sleepTime4[3] = {3, 2, 1};

    int arg4[2];
    for (int i = 0; i < 3; i++) {
        arg4[0] = types4[i];
        arg4[1] = sleepTime4[i];
        result = pthread_create(&threads4[i], NULL, (void *(*)(void *)) mythread, arg4);
        sleep(1);
        if (result != 0) {
            perror("Failed thread creation");
        }
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads4[i], NULL);
    }

    petgroom_done();
    printf("\n\n\n");


    // TEST FIVE: RULE 3: Neither types of pet can wait indefinitely and RULE 4: efficient solution
    printf("TEST FIVE\n");
    numstations = 2;
    initStations = petgroom_init(numstations);
    if (initStations == -1) {
        perror("Error, stations initialization failed (petgroom_init())");
    }

    pthread_t p11, p12, p13, p14, p15, p16, p17;
    pthread_t threads5[7] = {p11 = NULL, p12 = NULL, p13 = NULL, p14 = NULL, p15 = NULL, p16 = NULL, p17 = NULL};
    int types5[7] = {2, 1, 2, 1, 2, 1, 2};
    int sleepTime5[7] = {3, 2, 2, 2, 2, 2, 2};

    int arg5[2];
    for (int i = 0; i < 7; i++) {
        arg5[0] = types5[i];
        arg5[1] = sleepTime5[i];
        result = pthread_create(&threads5[i], NULL, (void *(*)(void *)) mythread, arg5);
        sleep(1);
        if (result != 0) {
            perror("Failed thread creation");
        }
    }

    for (int i = 0; i < 7; i++) {
        pthread_join(threads5[i], NULL);
    }

    petgroom_done();
    printf("\n\n\n");
}
