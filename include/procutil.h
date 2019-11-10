#ifndef PROC_UTIL_H
#define PROC_UTIL_H

#include <unistd.h>
#include <signal.h>

#include "../include/sharedvals.h"


typedef struct proc_handle {
    unsigned int count_procs_ready_terminate;
    int is_abrupt_terminate;
    pid_t pid_map[MAX_PROCESS_COUNT];
    sig_atomic_t ready;
} proc_handle_t;


int init_proc_handle(int key);
int destruct_proc_handle(int shid);
unsigned int get_procs_ready_to_terminate();
int mark_ready_to_terminate();
unsigned int get_count_procs_ready_terminate();
int mark_terminate();

/* PROCESS TABLE METHODS */
int get_first_unset_pid();
int set_first_unset_pid(pid_t pid);
int unset_pid(pid_t pid);
int index_of_pid(pid_t pid);
void print_proc_handle(int out_fd);


#endif
