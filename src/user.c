#include <stdio.h>
#include <unistd.h>

#include "../include/pclock.h"
#include "../include/sharedvals.h"
#include "../include/util.h"
#include "../include/procutil.h"

#define PROBABILITY_TERMINATE 20


/* Return true if the process should terminate.
 * Non-deterministic. We expect this to return
 * true *roughly* with p=`PROBABILITY_TERMINATE`.
 */
int is_should_terminate() {
    int random = rand_below(100);
    return (random < PROBABILITY_TERMINATE);
}


/* Return a random time to terminate.
 * If the `current_tick_ns` is less than
 * one second, the time will be after one
 * second. Else, it will be the random
 * time plus the current tick.
*/
unsigned long long next_termination_check_time(unsigned long long current_tick) {
    int term_check_add = rand_below(250);
    // We only check if we are past one second
    if (current_tick <= NANO_SEC_IN_SEC) {
        return term_check_add + NANO_SEC_IN_SEC;
    }
    return current_tick + term_check_add;
}


int main(int argc, char* argv[]) {
    init_clock(CLOCK_KEY);
    int proc_shid = init_proc_handle(PROC_KEY);
    unsigned long long current_tick = get_total_tick();
    unsigned long long next_term_check_time = next_termination_check_time(current_tick);

    while (1) {
        current_tick = get_total_tick();

        if (next_term_check_time >= current_tick) {
            next_term_check_time = next_termination_check_time(current_tick);
            if (is_should_terminate()) {
                fprintf(stderr, "[+] USER: Terminating PID %ld at %lld\n",
                        (long) getpid(), current_tick);
                mark_ready_to_terminate();
                return 0;
            } else {
                fprintf(stderr, "[-] USER: PID %ld at %lld\n", (long) getpid(), current_tick);
            }
        }
    }

    return 0;
}