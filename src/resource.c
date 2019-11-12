#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
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


static void internal_print(int fd) {
    /* TOTAL */
    int i, j;
    dprintf(fd, "OSS: TOTAL:\n");
    for (i = 0; i < RESOURCE_COUNT; ++i) {
        dprintf(fd, "|%d", descriptors->total[i]);
    }
    dprintf(fd, "|\n");
    /* AVAILABLE */
    dprintf(fd, "OSS: AVAILABLE:\n");
    for (i = 0; i < RESOURCE_COUNT; ++i) {
        dprintf(fd, "|%d", descriptors->available[i]);
    }
    dprintf(fd, "|\n");
    /* MAXIMUM CLAIM */
    dprintf(fd, "OSS: MAXIMUM CLAIMS:\n");
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        for (j = 0; j < RESOURCE_COUNT; ++j) {
            dprintf(fd, "|%d",
                    descriptors->maximum_claim[i][j]);
        }
        dprintf(fd, "|\n");
    }
    /* ALLOCATED */
    dprintf(fd, "OSS: ALLOCATED:\n");
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        for (j = 0; j < RESOURCE_COUNT; ++j) {
            dprintf(fd, "|%d",
                    descriptors->allocated[i][j]);
        }
        dprintf(fd, "|\n");
    }
    /* NEEDED */
    dprintf(fd, "OSS: NEEDED:\n");
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        for (j = 0; j < RESOURCE_COUNT; ++j) {
            dprintf(fd, "|%d",
                    descriptors->needed_max_less_allocated[i][j]);
        }
        dprintf(fd, "|\n");
    }
    /* REQUESTED */
    dprintf(fd, "OSS: REQUESTED:\n");
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        for (j = 0; j < RESOURCE_COUNT; ++j) {
            dprintf(fd, "|%d",
                    descriptors->requested[i][j]);
        }
        dprintf(fd, "|\n");
    }
}


void print_resource_descriptors(int fd) {
    if (semop(semid, &semlock, 1) == -1) {
        perror("resource: fail to get semlock");
        return;
    }
    internal_print(fd);
    if (semop(semid, &semunlock, 1) == -1) {
        perror("resource: fail to get semunlock");
        return;
    }
}


static void initialize_resource_tables() {
    srand(time(NULL) ^ (getpid() << 16));
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
            descriptors->requested[j][i] = 0;
            descriptors->needed_max_less_allocated[j][i] = 0;
        }
    }
}


void clear_process_from_resource_descriptors(int pid) {
    if (semop(semid, &semlock, 1) == -1) {
        perror("resource: fail to get semlock");
        return;
    }
    const int ROW = pid;
    int col, alloc;

    for (col = 0; col < RESOURCE_COUNT; ++col) {
        descriptors->maximum_claim[ROW][col] = 0;
        // Give the resources back
        alloc = descriptors->allocated[ROW][col];
        descriptors->available[col] += alloc;
        descriptors->allocated[ROW][col] = 0;
        // Clear out the requests
        descriptors->requested[ROW][col] = 0;
        // Clear out the max requests
        descriptors->maximum_claim[ROW][col] = 0;
    }
    descriptors->has_request[pid] = 0;
    if (semop(semid, &semunlock, 1) == -1) {
        perror("resource: fail to get semunlock");
        return;
    }
}


void make_request(int pid, int* local_req_buffer) {
    if (semop(semid, &semlock, 1) == -1) {
        perror("resource: fail to get semlock");
        return;
    }
    int random_resource = rand_below(RESOURCE_COUNT);
    int res_max_claim = descriptors->maximum_claim[pid][random_resource];
    int allocated = descriptors->allocated[pid][random_resource];
    int already_requested = descriptors->requested[pid][random_resource];
    int resource_still_needed = res_max_claim - allocated;
    int rand_request = rand_below(resource_still_needed + 1);

    if (already_requested >= res_max_claim) {
        rand_request = 0;
    }

    local_req_buffer[random_resource] += rand_request;
    descriptors->requested[pid][random_resource] += rand_request;

    descriptors->has_request[pid] = 1;
    if (semop(semid, &semunlock, 1) == -1) {
        perror("resource: fail to get semunlock");
        return;
    }
}


void make_release(int pid) {
    if (semop(semid, &semlock, 1) == -1) {
        perror("resource: fail to get semlock");
        return;
    }
    int random_resource = rand_below(RESOURCE_COUNT);

    int allocated = descriptors->allocated[pid][random_resource];
    
    int rand_release;
    if (allocated > 0) {
        rand_release = rand_below(allocated + 1);
    } else {
        rand_release = 0;
    }

    descriptors->allocated[pid][random_resource] -= rand_release;
    descriptors->available[random_resource] += rand_release;

    if (semop(semid, &semunlock, 1) == -1) {
        perror("resource: fail to get semunlock");
        return;
    }
}


void get_max_claims(int* buffer_array, int pid) {
    if (semop(semid, &semlock, 1) == -1) {
        perror("resource: fail to get semlock");
        return;
    }
    int i, current_claim;
    for (i = 0; i < RESOURCE_COUNT; ++i) {
        current_claim = rand_below(descriptors->total[i]);
        descriptors->maximum_claim[pid][i] = current_claim;
        buffer_array[i] = current_claim;
    }
    if (semop(semid, &semunlock, 1) == -1) {
        perror("resource: fail to get semunlock");
        return;
    }
}


void run_check(int fd) {
    if (semop(semid, &semlock, 1) == -1) {
        perror("resource: fail to get semlock");
        return;
    }
    int i, j;

    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        for (j = 0; j < RESOURCE_COUNT; ++j) {
            int request = descriptors->requested[i][j];
            if (request > 0) {
                // Copy the allocations and make the need matrix.
                int allocated[MAX_PROCESS_COUNT][RESOURCE_COUNT];
                int m, n;
                for (m = 0; m < MAX_PROCESS_COUNT; ++m) {
                    for (n = 0; n < RESOURCE_COUNT; ++n) {
                        allocated[m][n] = descriptors->allocated[m][n];
                        descriptors->needed_max_less_allocated[m][n] = 
                            descriptors->maximum_claim[m][n] - descriptors->allocated[m][n];
                    }
                }
                allocated[i][j] += request;
                // Run the banker's algorithm

                // Initialize the finish and the work array
                int finish[MAX_PROCESS_COUNT];
                int work[RESOURCE_COUNT];
                for (m = 0; m < MAX_PROCESS_COUNT; ++m) {
                    finish[m] = 0;
                }
                for (m = 0; m < RESOURCE_COUNT; ++m) {
                    work[m] = descriptors->available[m];
                }

                int count = 0;
                while (count < MAX_PROCESS_COUNT) {
                    int flag = 0;
                    for (m = 0; m < MAX_PROCESS_COUNT; ++m) {
                        if (!finish[m]) {
                            for (n = 0; n < RESOURCE_COUNT; ++n) {
                                if (descriptors->needed_max_less_allocated[m][n] > work[n]) {
                                    break;
                                }
                            }
                            if (n == RESOURCE_COUNT) {
                                ++count;
                                finish[m] = 1;
                                flag = 1;

                                for (n = 0; n < RESOURCE_COUNT; ++n) {
                                    work[n] = work[n] + allocated[m][n];
                                }
                            }
                        }
                    }
                    if (!flag) {
                        break;
                    }
                }
                if (count < MAX_PROCESS_COUNT) {
                    dprintf(fd, "OSS: Unable to grant resource (%d) for id (%d)\n",
                            j, i);
                    internal_print(fd);
                } else {
                    dprintf(fd, "OSS: Able to grant resource (%d) for id (%d)\n",
                            j, i);
                    descriptors->allocated[i][j] += request;
                    descriptors->available[j] -= request;
                    internal_print(fd);
                }
            }
        }
    }
    if (semop(semid, &semunlock, 1) == -1) {
        perror("resource: fail to get semunlock");
        return;
    }
}
