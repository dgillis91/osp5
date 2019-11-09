#ifndef _PARSE_H
#define _PARSE_H


typedef struct program_options {
    char logfile_path[200];
    unsigned int max_time_between_requests;
    unsigned int allowable_run_time;
} program_options_t;


int init_prog_opts(int key);
int destruct_prog_opts();
void parse_options(int, char**);
void print_help_and_terminate(char*);
void set_default_program_options();
unsigned int get_allowable_run_time();
unsigned int get_max_time_between_requests();
const char* get_logfile_path();

#endif
