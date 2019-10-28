#include <stdio.h>
#include <unistd.h>

#include "../include/pclock.h"
#include "../include/sharedvals.h"


int main(int argc, char* argv[]) {
    init_clock(CLOCK_KEY);

    int i;
    for (i = 0; i < 10; ++i) {
        fprintf(stderr, "USER: Clock Time in Child [%ld] [%u:%uT%lu] iter: %d\n",
                (long) getpid(), get_seconds(), get_nano(), get_total_tick(), i);
    }

    return 0;
}