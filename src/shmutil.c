#include <sys/shm.h>
#include <errno.h>
#include <stdlib.h>
#include "../include/shmutil.h"


int detachandremove(int shmid, void* addr) {
    int error = 0;

    if (shmdt(addr) == -1) {
        error = errno;
    }

    if ((shmctl(shmid, IPC_RMID, NULL) == -1) && !error) {
        error = errno;
    }

    if (!error) {
        return 0;
    }

    errno = error;
    return -1;
}
