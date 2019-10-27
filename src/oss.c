#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "../include/util.h"
#include "../include/pclock.h"
#include "../include/sharedvals.h"


#define MAX_PROCESS_COUNT 18
#define MAX_TIME_BETWEEN_PROCS_NANO 20
#define MAX_RUN_TIME_SECONDS 10
#define MAX_RUN_TIME_NANO MAX_RUN_TIME_SECONDS * NANO_SEC_IN_SEC
#define CLOCK_TICK_NANO NANO_SEC_IN_SEC / 10

void terminate_program();

int main(int argc, char* argv[]) {
    init_clock(CLOCK_KEY);
    while (get_total_tick() <= MAX_RUN_TIME_NANO) {
        fprintf(stderr, "OSS: Time [%u:%uT%lu]\n",
                get_seconds(), get_nano(), get_total_tick());
        tick_clock(CLOCK_TICK_NANO);
    }

    destruct_clock();
    return 0;
}


void terminate_program() {
    destruct_clock();
}