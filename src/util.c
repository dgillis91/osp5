#include <stdlib.h>

#include "../include/util.h"


/* Return a random number between `lower` and `upper`.
** Note that we don't seed. Additionally, we are working with 
** the naturals mod upper.
*/
unsigned int rand_between(unsigned int lower, unsigned int upper) {
    return (rand() % (upper - lower + 1)) + lower;
}


/* Return a random number below `max` but 
** greater than or equal to 0.
*/
unsigned int rand_below(unsigned int max) {
    return (rand() % max);
}
