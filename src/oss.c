#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>


#define MAX_PROCESS_COUNT 18


void terminate_program();

static pid_t children[MAX_PROCESS_COUNT];


int main(int argc, char* argv[]) {
    int i;

    pid_t current_fork_value;
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        children[i] = (current_fork_value = fork());
        if (!current_fork_value) {
            break;
        }
    }

    if (!current_fork_value) {
        fprintf(stderr, "OSS: Child of %ld, %ld\n",
                (long) getppid(), (long) getpid());
    }

    sleep(5);
    return 0;
}


void terminate_program() {
    int i;
    for (i = 0; i < MAX_PROCESS_COUNT; ++i) {
        kill(children[i], SIGKILL);
    }
}