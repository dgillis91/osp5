#ifndef RESOURCE_H_
#define RESOURCE_H_

#include <signal.h>

#include "../include/sharedvals.h"

#define RESOURCE_COUNT 20

typedef struct resource_descriptors {
    int is_shared[RESOURCE_COUNT];
    unsigned int total[RESOURCE_COUNT];
    unsigned int available[RESOURCE_COUNT];
    unsigned int maximum_claim[MAX_PROCESS_COUNT][RESOURCE_COUNT];
    unsigned int allocated[MAX_PROCESS_COUNT][RESOURCE_COUNT];
    unsigned int needed_max_less_allocated[MAX_PROCESS_COUNT][RESOURCE_COUNT];
    unsigned int requested[MAX_PROCESS_COUNT][RESOURCE_COUNT];
    sig_atomic_t ready;
} resource_descriptors_t;

int init_resource_descriptors(int key);
int destruct_resource_descriptors();
void print_resource_descriptors(int fd);
void clear_process_from_resource_descriptors(int pid);
void make_request(int pid, int* local_req_buffer);
void make_release(int pid);
void get_max_claims(int* buffer_array, int pid);

#endif