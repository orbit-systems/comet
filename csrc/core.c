// ╭───────╮
// │ comet │ the Aphelion ISA reference emulator - rewritten in C!
// ╰───────╯
// by kaylatheegg, spsandwichman

// using aphelion v0.2.2

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#include "comet.h"
#include "cpu.c"
#include "mem.c"
#include "decode.c"

void print_help() {
    printf("\nusage: comet (path) [flags]\n");
    printf("\n-debug               launch window with debug interface");
    printf("\n-max-cycles:[int]    halt after cycle count has been reached (will run forever if unset)");
    printf("\n-halt-on-inv         halt on invalid opcode");
    printf("\n-no-color            disable ANSI formatting");
    printf("\n-bench               output benchmark info after execution is halted");
    printf("\n-help                display this text\n\n");
}

typedef struct cmd_arg {
    char* key;
    char* val;
} cmd_arg;

// hacky shit but whatever
cmd_arg make_argument(char* s) {
    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == ':') {
            s[i] = '\0';
            return (cmd_arg){s, s+i+1};
        }
    }
    return (cmd_arg){s, ""};
}

void load_arguments(int argc, char* argv[]) {
    if (argc < 2) {
        print_help();
        exit(EXIT_SUCCESS);
    }
    for (size_t i = 1; i < argc; i++) {
        cmd_arg a = make_argument(argv[i]);
        if (!strcmp(a.key, "-help")) {
            print_help();
            exit(EXIT_SUCCESS);
        } else if (!strcmp(a.key, "-debug")) {
            flag_debug = true;
        } else if (!strcmp(a.key, "-no-color")) {
            flag_no_color = true;
        } else if (!strcmp(a.key, "-halt-on-inv")) {
            flag_halt_inv_op = true;
        } else if (!strcmp(a.key, "-bench")) {
            flag_benchmark = true;
        } else if (!strcmp(a.key, "-max-cycles")) {
            flag_cycle_limit = strtoll(a.val, NULL, 0);
            if (flag_cycle_limit == 0) {
                printf("error: expected positive int, got \"%s\"\n", a.val);
                exit(EXIT_FAILURE);
            }
        } else {
            if (i == 1 && a.key[0] != '-') {
                flag_bin_path = a.key;
            } else {
                printf("error: unrecognized option \"%s\"\n", a.key);
                exit(EXIT_FAILURE);
            }
        }
    }
};

int main(int argc, char *argv[]) {

    load_arguments(argc, argv);
    emulator_state comet = {};
    init_page_map(0);

    FILE* bin_file = fopen(flag_bin_path, "r");
    if (bin_file == NULL) {
        printf("error: could not open file \"%s\"\n", flag_bin_path);
        exit(EXIT_FAILURE);
    }

    load_image(bin_file);
    fclose(bin_file);

    comet.cpu.registers[r_pc] = 0xA00; // starting point
    comet.cpu.running = true;

    // timing
    struct timeval begin, end;
    gettimeofday(&begin, 0);

    while (comet.cpu.running && flag_cycle_limit > comet.cpu.cycle) {
        do_cpu_cycle(&comet.cpu);
    }

    gettimeofday(&end, 0);
    if (flag_benchmark) {
        long seconds = end.tv_sec - begin.tv_sec;
        long microseconds = end.tv_usec - begin.tv_usec;
        double elapsed = seconds + microseconds*1e-6;
        double cycles_per_sec = comet.cpu.cycle / elapsed;
        printf("overall time : %fs (%fms)\n", elapsed, elapsed*1000.0);
        printf("total cycles : %d\n", comet.cpu.cycle);
        printf("cycles/sec   : %f\n", cycles_per_sec);
    }
    free_page_map();

    return EXIT_SUCCESS;
}