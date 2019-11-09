#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>

#include "../include/resource.h"
#include "../include/semutil.h"
#include "../include/shmutil.h"
#include "../include/util.h"

#define PERM (S_IRUSR | S_IWUSR)


static void initialize_resource_tables();


static int semid;
static int shid;

static struct sembuf semlock;
static struct sembuf semunlock;

static resource_descriptors_t* descriptors;


int init_resource_descriptors(int key) {
    setsembuf(&semlock, 0, -1, 0);
    setsembuf(&semunlock, 0, 1, 0);

    shid = shmget(key, sizeof(resource_descriptors_t), PERM | IPC_CREAT | IPC_EXCL);

    // Couldn't gedt shared memory, and there's a real error.
    if ((shid == -1) && errno != EEXIST) {
        return -1;
    }

    // Already created. Attach. 
    if (shid == -1) {
        // Access
        if ((shid = shmget(key, sizeof(resource_descriptors_t), PERM)) == -1) {
            return -1;
        }
        // Attach
        if ((descriptors = (resource_descriptors_t*) shmat(shid, NULL, 0)) == (void*)(-1)) {
            return -1;
        }
    } else {
        // Create in the first shmget call. 
        descriptors = (resource_descriptors_t*) shmat(shid, NULL, 0);
        if (descriptors == (void*)(-1)) {
            return -1;
        }
        initialize_resource_tables();
    }
    semid = initsemset(key, 1, &descriptors->ready);
    if (semid == -1) {
        return -1;
    }
    return 1;
}


int destruct_resource_descriptors() {
    if (removesem(semid) == -1) {
        return -1;
    }
    if (detachandremove(shid, descriptors) == -1) {
        return -1;
    }
    return 1;
}


void print_resource_descriptors(char* filepath) {
    
}


static void initialize_resource_tables() {
    int i, j;
    unsigned int total;

    for (i = 0; i < RESOURCE_COUNT; ++i) {
        descriptors->is_shared[i] = 0;
        total = rand_below(10) + 1;
        descriptors->total[i] = total;
        descriptors->available[i] = total;
        for (j = 0; j < MAX_PROCESS_COUNT; ++j) {
            descriptors->maximum_claim[j][i] = 0;
            descriptors->allocated[j][i] = 0;
            descriptors->needed_max_less_allocated[j][i] = 0;
        }
    }
}
