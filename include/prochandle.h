#ifndef PROC_HANDLE_H_
#define PROC_HANDLE_H_

#include <unistd.h>


void initialize_process_handle();
int get_first_unset_pid();
int set_first_unset_pid(pid_t pid);
int unset_pid(pid_t pid);
void print_proc_handle(int out_fd);

#endif
