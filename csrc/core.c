#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "comet.h"

// ╭───────╮
// │ comet │ the Aphelion ISA reference emulator - rewritten in C!
// ╰───────╯
// by kaylatheegg, spsandwichman

// using aphelion v0.2.2

bool flag_debug = false;
u64  flag_cycle_limit = 0;
bool flag_no_color = false;
bool flag_halt_inv_op = false;
bool flag_benchmark = false;
char* inpath = "";

bool flag_internal_restart = 0;

void printhelp() {
    printf("\nusage: comet (path) [flags]\n");
    printf("\n-debug               launch window with debug information");
    printf("\n-max-cycles:[int]    halt after cycle count has been reached");
    printf("\n-halt-on-inv         halt on invalid opcode");
    printf("\n-no-color            disable ANSI formatting");
    printf("\n-bench               output benchmarking information after execution is halted");
    printf("\n-help                display this text\n\n");
}

void load_arguments(int argc, char* argv[]) {
    if (argc < 2) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    for (size_t i = i; i < argc; i++) {
        
    }
    TODO("fucking argument parsing lmao");

};

int main(int argc, char *argv[]) {

    //load_arguments(argc, argv);

    return 0;
}