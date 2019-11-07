#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "../include/parse.h"


static program_options_t global_program_options;


void print_help_and_terminate(char* program_name) {
    printf("Program to simulate resource allocation and deadlock avoidance.\n");
    printf("Usage: %s [-h] [-t -l -m]\n"
           "For: \n"
           "    -h: Display help and exit.\n"
           "    -t: Amount of time to allow the program to run for.\n"
           "    -l: Log file path.\n"
           "    -m: Maximum time between requests.\n", program_name);
    exit(EXIT_SUCCESS);
}


/* Parse the command line options and modify the passed in `program_opts`
 * with them. 
 * @Params:
 * * Note argc and argv come from the main method.
 * - argc: Count of arguments from the program.
 * - argv: Vector of arguments.
 * - program_opts: Pointer to a program_options_t. Is modified.
 * @Reference: 
 * - https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
 * - http://man7.org/linux/man-pages/man3/getopt.3.html
*/
void parse_options(int argc, char* argv[]) {
   // Parse default options.
   set_default_program_options();

    // Set this so that `getopt` doesn't write error messages.
    opterr = 0;
    int current_option;
    // As per the man page, calling `getopt` parses the arguments from argv,
    // as specified in the third param, `optstring`. When there are no option
    // characters left, `getopt` returns -1.
    while ((current_option = getopt(argc, argv, "hm:l:t:")) != -1) {
       switch (current_option) {
        case 'h':
            print_help_and_terminate(argv[0]);
            break;
        case 'm':
            global_program_options.max_time_between_requests = atoi(optarg);
            break;
        case 'l':
            strcpy(global_program_options.logfile_path, optarg);
            break;
        case 't':
            global_program_options.allowable_run_time = atol(optarg);
            break;
        // From the man page:
        //  By default, getopt() prints an error message on standard error,
        //  places the erroneous option character in optopt, and returns '?'
        //  as the function result.
        //  Note that `getopt` sets a global `optopt` with the option character.
        case '?':
            // If the error is for not having an arg with one of the params . . . 
            if (optopt == 'm' || optopt == 'l' || optopt == 't') {
                fprintf(stderr, "%s: Error: Argument `%c` requires a parameter\n", argv[0], optopt);
            // Unknown opt
            } else {
                fprintf(stderr, "%s: Error: Unkown option character -%c\n", argv[0], optopt);
            }
            // In either case, terminate.
            print_help_and_terminate(argv[0]);
            break;
       }
    }

    if (argv[optind] != NULL) {
        fprintf(stderr, "%s: Unexpected non-option argument: %s\n", argv[0], argv[optind]);
        print_help_and_terminate(argv[0]);
    }
}


void set_default_program_options() {
    global_program_options.allowable_run_time = 10L;
    global_program_options.max_time_between_requests = 500;
    strcpy(global_program_options.logfile_path, "log.txt");
}


unsigned int get_allowable_run_time() {
    return global_program_options.allowable_run_time;
}


unsigned int get_max_time_between_requests() {
    return global_program_options.max_time_between_requests;
}


const char* get_logfile_path() {
    return global_program_options.logfile_path;
}
