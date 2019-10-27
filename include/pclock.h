/* pclock.h
** Abstract representation of a shared clock.
** We need to create atomic access to the clock (sounds funny). 
** We will have n-many processes. We need to ensure that only
** the parent can modify the clock. All children can read it. 
** Note: shm and sem code inspired by UNIX SYSTEM PROGRAMMING.
*/
#ifndef P_CLOCK_H_
#define P_CLOCK_H_
#include <signal.h>

#define NANO_SEC_IN_SEC 1000000000UL

/* Abstract representation of a system clock.
*/
typedef struct {
    unsigned long total_tick;
    sig_atomic_t ready;
} pclock_t;


int init_clock(int);
int destruct_clock();
int get_copy_of_system_clock(pclock_t*);
int tick_clock(unsigned long);
pclock_t clock_add(pclock_t, unsigned long);
void clock_add_in_place(pclock_t*, unsigned long);
int is_equal_to_sys_clock(pclock_t);
unsigned int get_seconds();
unsigned int get_nano();
unsigned long get_total_tick();


#endif
