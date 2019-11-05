#ifndef PROC_UTIL_H
#define PROC_UTIL_H

#include <unistd.h>
#include <signal.h>


typedef struct proc_handle {
    unsigned int count_procs_ready_terminate;
    int is_abrupt_terminate;
    sig_atomic_t ready;
} proc_handle_t;


int init_proc_handle(int key);
int destruct_proc_handle(int shid);
unsigned int get_procs_ready_to_terminate();
int mark_ready_to_terminate();
unsigned int get_count_procs_ready_terminate();
int mark_terminate();
int get_is_abrupt_terminate();
int set_is_abrupt_terminate();
int unset_is_abrupt_terminate();


#endif
