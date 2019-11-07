#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "../include/util.h"
#include "../include/pclock.h"
#include "../include/sharedvals.h"
#include "../include/procutil.h"


#define MAX_PROCESS_COUNT 3

#define MAX_TIME_BETWEEN_PROCS_NANO 20
#define CLOCK_TICK_NANO NANO_SEC_IN_SEC / 1000

#define MAX_RUN_TIME_SECONDS 2
#define MAX_RUN_TIME_NANO MAX_RUN_TIME_SECONDS * NANO_SEC_IN_SEC

#define MAX_ACTUAL_RUN_TIME_SECONDS 5

void terminate_program();
unsigned long compute_random_next_init_time(unsigned long current_time_nano);
void sig_handler(int signum);

static int proc_shid;

int main(int argc, char* argv[]) {
    int current_process_count = 0;

    init_clock(CLOCK_KEY);
    proc_shid = init_proc_handle(PROC_KEY);

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("OSS: Fail to set SIGINT");
    }

    if (signal(SIGALRM, sig_handler) == SIG_ERR) {
        perror("OSS: Fail to set SIGALRM");
    }
    alarm(MAX_ACTUAL_RUN_TIME_SECONDS);

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
                fprintf(stderr, "OSS: Forking child [%ld] at: [%u:%uT%lu]\n",
                        (long) current_fork_value, get_seconds(), 
                        get_nano(), current_time_nano);
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
            fprintf(stderr, "OSS: [%lu] is terminating at %u.%u\n",
                    (long) wait_pid, get_seconds(), get_nano());
            --current_process_count;
        }

        //fprintf(stderr, "OSS: Time [%u:%uT%lu]\n",
        //        get_seconds(), get_nano(), current_time_nano);
        tick_clock(CLOCK_TICK_NANO);
    }

    fprintf(stderr, "OSS: Sleep\n");
    sleep(10);
    destruct_clock();
    destruct_proc_handle(proc_shid);
    return 0;
}


void sig_handler(int signum) {
    if (signum == SIGINT) {
        fprintf(stderr, "[!] OSS: Killing all from SIGINT\n");
    } else if (signum == SIGALRM) {
        fprintf(stderr, "[!] OSS: Killing all from SIGALRM\n");
    } else {
        fprintf(stderr, "[!] OSS: Killing all from unkown signal\n");
    }
    terminate_program();
}


void terminate_program() {
    destruct_proc_handle(proc_shid);
    destruct_clock();
    kill(0, SIGKILL);
    exit(1);
}


unsigned long compute_random_next_init_time(unsigned long current_time_nano) {
    int time_until_next_proc = rand_below(MAX_TIME_BETWEEN_PROCS_NANO);
    return current_time_nano + time_until_next_proc;
}
