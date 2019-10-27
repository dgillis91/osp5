/* Semaphore utilities taken from UNIX SYSTEM PROGRAMMING
** Utilities for working with semaphores. 
*/
#ifndef SEM_UTIL_H
#define SEM_UTIL_H


#include <signal.h>
#include <sys/shm.h>
#include <sys/sem.h>


int initelement(int semid, int semnum, int semvalue);
int removesem(int semid);
int initsemset(key_t key, int value, sig_atomic_t *readyp);
void setsembuf(struct sembuf* s, int num, int op, int flg);

#endif
