#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

#include "../include/util.h"
#include "../include/pclock.h"
#include "../include/sharedvals.h"
#include "../include/procutil.h"
#include "../include/parse.h"


#define MAX_PROCESS_COUNT 3

#define MAX_TIME_BETWEEN_PROCS_NANO 20
#define CLOCK_TICK_NANO NANO_SEC_IN_SEC / 1000

#define MAX_RUN_TIME_SECONDS 2
#define MAX_RUN_TIME_NANO MAX_RUN_TIME_SECONDS * NANO_SEC_IN_SEC
#define CONSOLE_OUT 0


void terminate_program();
unsigned long compute_random_next_init_time(unsigned long current_time_nano);

/* Functions to deal with a process handle, 
 * mapping a integer (index) to a pid.
*/
static pid_t process_handle[MAX_PROCESS_COUNT];
void initialize_process_handle();
int get_first_unset_pid();
int set_first_unset_pid(pid_t pid);
int unset_pid(pid_t pid);
void print_proc_handle();


void sig_handler(int signum);

static int proc_shid;
static int out_fd;

int main(int argc, char* argv[]) {
    parse_options(argc, argv);

    if (!CONSOLE_OUT) {
        out_fd = open(get_logfile_path(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    } else {
        out_fd = STDERR_FILENO;
    }

    int current_process_count = 0;

    init_clock(CLOCK_KEY);
    proc_shid = init_proc_handle(PROC_KEY);
    initialize_process_handle();

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("OSS: Fail to set SIGINT");
    }
    if (signal(SIGALRM, sig_handler) == SIG_ERR) {
        perror("OSS: Fail to set SIGALRM");
    }
    alarm(get_allowable_run_time());

    unsigned long current_time_nano;
    unsigned long next_process_init_time = MAX_TIME_BETWEEN_PROCS_NANO;
    // Main clock loop
    while ((current_time_nano = get_total_tick()) <= MAX_RUN_TIME_NANO) {
        // Spawn processes off at randomish times
        int is_time_to_init_new_proc = (current_time_nano >= next_process_init_time);
        int not_at_process_limit = (current_process_count < MAX_PROCESS_COUNT);
        if (is_time_to_init_new_proc && not_at_process_limit) {
            ++current_process_count;
            pid_t current_fork_value = fork();
            
            if (current_fork_value) {
                set_first_unset_pid(current_fork_value);
                dprintf(out_fd, "OSS: Forking child [%ld] at: [%u:%uT%lu]\n",
                        (long) current_fork_value, get_seconds(), 
                        get_nano(), current_time_nano);
                print_proc_handle();
            }
            
            if (!current_fork_value) {
                execl("user", "user", NULL);
                return 0;
            }
            next_process_init_time = compute_random_next_init_time(current_time_nano);
        }
        if (get_count_procs_ready_terminate() > 0) {
            int stat;
            mark_terminate();
            pid_t wait_pid = wait(&stat);
            unset_pid(wait_pid);
            dprintf(out_fd, "OSS: [%lu] is terminating at %u.%u\n",
                    (long) wait_pid, get_seconds(), get_nano());
            --current_process_count;
            print_proc_handle();
        }

        //fprintf(stderr, "OSS: Time [%u:%uT%lu]\n",
        //        get_seconds(), get_nano(), current_time_nano);
        tick_clock(CLOCK_TICK_NANO);
    }

    dprintf(out_fd, "OSS: Sleep\n");
    sleep(10);
    destruct_clock();
    destruct_proc_handle(proc_shid);
    close(out_fd);
    return 0;
}


void sig_handler(int signum) {
    if (signum == SIGINT) {
        dprintf(out_fd, "[!] OSS: Killing all from SIGINT\n");
    } else if (signum == SIGALRM) {
        dprintf(out_fd, "[!] OSS: Killing all from SIGALRM\n");
    } else {
        dprintf(out_fd, "[!] OSS: Killing all from unkown signal\n");
    }
    terminate_program();
}


void terminate_program() {
    destruct_proc_handle(proc_shid);
    destruct_clock();
    close(out_fd);
    kill(0, SIGKILL);
    exit(1);
}


unsigned long compute_random_next_init_time(unsigned long current_time_nano) {
    int time_until_next_proc = rand_below(MAX_TIME_BETWEEN_PROCS_NANO);
    return current_time_nano + time_until_next_proc;
}


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


void print_proc_handle() {
    int i;
    dprintf(out_fd, "OSS: process_handle = ");
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        dprintf(out_fd, "|%ld", (long) process_handle[i]);
    }
    dprintf(out_fd, "|\n");
}