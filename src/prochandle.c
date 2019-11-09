#include <unistd.h>

#include "../include/prochandle.h"
#include "../include/sharedvals.h"


static pid_t process_handle[MAX_PROCESS_COUNT];

/* Initialize all pids to 0.
 */
void initialize_process_handle() {
    int i; 
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        process_handle[i] = (pid_t) 0;
    }
}

/* Return the first index of an unset pid in
 * the `process_handle`. Returns -1 if full.
 */
int get_first_unset_pid() {
    int i;
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        if (process_handle[i] == (pid_t) 0) {
            return i;
        }
    }
    return -1;
}


/* Set the first empty spot in the `process_handle`.
 * Return 1 on success, -1 on full.
 */
int set_first_unset_pid(pid_t pid) {
    int i;
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        if (process_handle[i] == (pid_t) 0) {
            process_handle[i] = pid;
            return 1;
        }
    }
    return -1;
}


/* Set `pid` to 0 in `process_table`. Returns -1
 * if `pid` not found.
 */
int unset_pid(pid_t pid) {
    int i;
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        if (process_handle[i] == pid) {
            process_handle[i] = (pid_t) 0;
            return 1;
        }
    }
    return -1;
}


void print_proc_handle(int out_fd) {
    int i;
    dprintf(out_fd, "OSS: process_handle = ");
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        dprintf(out_fd, "|%ld", (long) process_handle[i]);
    }
    dprintf(out_fd, "|\n");
}