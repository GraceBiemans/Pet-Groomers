// Grace Biemans & Nick Heleta
// geb965 & nwh397

#ifndef A3_PETGROOMSYNCH_H
#define A3_PETGROOMSYNCH_H

#endif //A3_PETGROOMSYNCH_H

typedef enum {
    cat = 1,
    dog = 2,
    other = 3,
} pet_t;

/**
* Initializes variables for the solution
* @param numstations : the number of grooming stations in the facility
* @return: 0 on success, -1 on failure
*/
int petgroom_init(int numstations);

/**
 * Deals with the arrival of a new pet to the facility
 * @param pet: the new pet
 * @return: 0 on success, -1 on failure
 */
int newpet(pet_t pet);

/**
 * Called when pet is done with the grooming station, the station is now free
 * @param pet: the pet that is done being groomed
 * @return: 0 on success, -1 on failure
 */
int petdone(pet_t pet);

/**
 * Performs required data cleanup actions. Makes it possible to reinitialize and groom another pet
 * @return: 0 on success, -1 on failure
 */
int petgroom_done();
