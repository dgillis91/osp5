/* All code taken from, or inspired by,
** UNIX SYSTEM PROGRAMMING.
*/
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include "../include/semutil.h"

#define TEN_MILLION 10000000L
#define PERMS (S_IRUSR | S_IWUSR)


/* Set the semaphore given by `semid` to
 * `semvalue`.
*/
int initelement(int semid, int semnum, int semvalue) {
    union semun {
        int val;
        struct semid_ds* buf;
        unsigned short* array;
    } arg;
    arg.val = semvalue;
    return semctl(semid, semnum, SETVAL, arg);
}


/* Remove the semaphore given by
 * `semid`.
*/
int removesem(int semid) {
   return semctl(semid, 0, IPC_RMID);
}


/* Initialize the semaphore `readyp`.
 * Params:
 *  key - for the semaphore
 *  value - to initialize the semaphore with
 *  readyp - lock
*/
int initsemset(key_t key, int value, sig_atomic_t *readyp) {
    int semid;
    struct timespec sleeptime;

    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = TEN_MILLION;
    semid = semget(key, 2, PERMS | IPC_CREAT | IPC_EXCL);

    // semget resulted in error. Error wasn't for the sem existing.
    if ((semid == -1) && (errno != EEXIST)) {
        return -1;
    }

    // Semaphore was created.
    if (semid >= 0) {
        if (initelement(semid, 0, value) == -1) {
            return -1;
        }
        *readyp = 1;
        return semid;
    }

    // Otherwise, it existed before calling init. 
    if ((semid = semget(key, 2, PERMS)) == -1) {
        return -1;
    }

    // Wait for the semaphore to be initialized.
    while (*readyp == 0) {
        nanosleep(&sleeptime, NULL);
    }

    return semid;
}


/* Initialize the values in the sembuf `s`.
*/
void setsembuf(struct sembuf* s, int num, int op, int flg) {
    s->sem_num = (short) num;
    s->sem_op = (short) op;
    s->sem_flg = (short) flg;
    return;
}
