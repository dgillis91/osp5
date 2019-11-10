#include "../include/procutil.h"
#include "../include/shmutil.h"
#include "../include/semutil.h"
#include "../include/sharedvals.h"

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define PROC_HANDLE_PERM (S_IRUSR | S_IWUSR)

#define NULL_PID (pid_t)(0)

static int semid;
static struct sembuf semlock;
static struct sembuf semunlock;
static proc_handle_t* proc_handle;

static void initialize_process_handle();


int init_proc_handle(int key) {
    int shid;

    setsembuf(&semlock, 0, -1, 0);
    setsembuf(&semunlock, 0, 1, 0);

    shid = shmget(key, sizeof(proc_handle_t), PROC_HANDLE_PERM | IPC_CREAT | IPC_EXCL);

    if ((shid == -1) && errno != EEXIST) {
        return -1;
    }

    if (shid == -1) {
        if ((shid = shmget(key, sizeof(proc_handle_t), PROC_HANDLE_PERM)) == -1) {
            return -1;
        }
        if ((proc_handle = (proc_handle_t*) shmat(shid, NULL, 0)) == (void*)(-1)) {
            return -1;
        }
    } else {
        proc_handle = (proc_handle_t*) shmat(shid, NULL, 0);
        if (proc_handle == (void*)(-1)) {
            return -1;
        }
        proc_handle->count_procs_ready_terminate = 0;
        proc_handle->is_abrupt_terminate = 0;
        initialize_process_handle();
    }
    semid = initsemset(key, 1, &proc_handle->ready);
    if (semid == -1) {
        return -1;
    }
    return shid;
}

/* Clear the shared memory and semaphores from
 * the system. Must only be called once. Must
 * only be called in the master process. 
*/
int destruct_proc_handle(int shid) {
    // XXX: `shid` should be a static to this "module".
    if (removesem(semid) == -1) {
        return -1;
    }
    if (detachandremove(shid, proc_handle) == -1) {
        return -1;
    }
    return 1;
}


/* Called in a child process to specify that the
 * process is ready to terminate. Will increate
 * the count of children ready to terminate.
*/
int mark_ready_to_terminate() {
    if (semop(semid, &semlock, 1) == -1) 
        return -1;
    ++proc_handle->count_procs_ready_terminate;
    if (semop(semid, &semunlock, 1) == -1)
        return -1;
    return 1;
}


unsigned int get_count_procs_ready_terminate() {
    unsigned int count;
    if (semop(semid, &semlock, 1) == -1) 
        return 0;
    count = proc_handle->count_procs_ready_terminate;
    if (semop(semid, &semunlock, 1) == -1)
        return 0;
    return count;
}


/* Signifies that the master process accepted
 * the termination of a process. Has no error
 * handling. For now, it's on master to handle
 * that. Will refactor if possible.
 */
int mark_terminate() {
    if (semop(semid, &semlock, 1) == -1) 
        return -1;
    --proc_handle->count_procs_ready_terminate;
    if (semop(semid, &semunlock, 1) == -1)
        return -1;
    return 1;
}



/* Initialize all pids to 0.
 */
static void initialize_process_handle() {
    int i; 
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        proc_handle->pid_map[i] = (pid_t) 0;
    }
}

/* Return the first index of an unset pid in
 * the `process_handle`. Returns -1 if full.
 */
int get_first_unset_pid() {
    int i;
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        if (proc_handle->pid_map[i] == (pid_t) 0) {
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
        if (proc_handle->pid_map[i] == (pid_t) 0) {
            proc_handle->pid_map[i] = pid;
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
        if (proc_handle->pid_map[i] == pid) {
            proc_handle->pid_map[i] = (pid_t) 0;
            return 1;
        }
    }
    return -1;
}


/* Return the index of `pid` if it exists
 * in the `process_handle`, else -1.
 */
int index_of_pid(pid_t pid) {
    int i;
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        if (proc_handle->pid_map[i] == pid) {
            return i;
        }
    }
    return -1;
}


void print_proc_handle(int out_fd) {
    int i;
    dprintf(out_fd, "OSS: process_handle = ");
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        dprintf(out_fd, "|%ld", 
                (long) proc_handle->pid_map[i]);
    }
    dprintf(out_fd, "|\n");
}