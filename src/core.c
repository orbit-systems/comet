// ╭───────╮
// │ comet │ the Aphelion ISA reference emulator - rewritten in C!
// ╰───────╯
// by kaylatheegg, spsandwichman

// using aphelion v0.3

#include "comet.h"

void print_help() {
    printf("\nusage: comet (path) [flags]\n");
    printf("\n-debug               launch window with debug interface");
    printf("\n-max-cycles:[int]    halt after cycle count has been reached (will run forever if unset)");
    printf("\n-halt-on-inv         friendship ended with -halt-on-inv. now -max-cycles is my best friend");
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

void load_arguments(int argc, char* argv[], emulator_state* comet) {
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
            comet->flag_debug = true;
        } else if (!strcmp(a.key, "-no-color")) {
            comet->flag_no_color = true;
        } else if (!strcmp(a.key, "-bench")) {
            comet->flag_benchmark = true;
        } else if (!strcmp(a.key, "-max-cycles")) {
            comet->flag_cycle_limit = strtoll(a.val, NULL, 0);
            if (comet->flag_cycle_limit == 0) {
                printf("error: expected positive int, got \"%s\"\n", a.val);
                exit(EXIT_FAILURE);
            }
        } else {
            if (i == 1 && a.key[0] != '-') {
                comet->flag_bin_path = a.key;
            } else {
                printf("error: unrecognized option \"%s\"\n", a.key);
                exit(EXIT_FAILURE);
            }
        }
    }
};

int main(int argc, char *argv[]) {

    emulator_state comet = {
        .cpu = (cpu_state){}, 
        .ic = (ic_state){}, 
    };

    load_arguments(argc, argv, &comet);
    bool mem_init_success = init_memory();
    if (!mem_init_success) {
        printf("error: virtual memory space could not initialize (fuck! ask sandwichman about this)\n");
        exit(EXIT_FAILURE);
    }

    FILE* bin_file = fopen(comet.flag_bin_path, "r");
    if (bin_file == NULL) {
        printf("error: could not open file \"%s\"\n", comet.flag_bin_path);
        exit(EXIT_FAILURE);
    }

    load_image(bin_file);
    fclose(bin_file);

    // timing
    struct timeval begin, end;
    gettimeofday(&begin, 0);

    comet.cpu.registers[r_pc] = 0;
    comet.cpu.running = true;

    while (comet.cpu.running && comet.flag_cycle_limit > comet.cpu.cycle) {
        comet.cpu.cycle++;

        // attempt to read instruction

        bool success = phys_read_u32(comet.cpu.registers[r_pc], &comet.cpu.raw_ins);
        if (!success) { // if it did not work (unaligned access)
            // retrieve interrupt handler address
            phys_read_u64(comet.ic.ivt_base_address + 8*int_unaligned_access, &comet.cpu.registers[r_pc]);

            // read new instruction
            phys_read_u32(comet.cpu.registers[r_pc], &comet.cpu.raw_ins);
        }

        raw_decode(comet.cpu.raw_ins, &comet.cpu.ins_info);
        comet.cpu.registers[r_st] &= 0x00000000FFFFFFFFull;
        comet.cpu.registers[r_st] |= (u64) comet.cpu.raw_ins << 32;

        exec_instruction(&comet, &comet.cpu.ins_info);

        comet.cpu.registers[r_pc] += 4 * (u64) comet.cpu.increment_next;
    }

    gettimeofday(&end, 0);
    if (comet.flag_benchmark) {
        long seconds = end.tv_sec - begin.tv_sec;
        long microseconds = end.tv_usec - begin.tv_usec;
        double elapsed = seconds + microseconds*1e-6;
        double cycles_per_sec = comet.cpu.cycle / elapsed;
        printf("overall time : %fs\n", elapsed);
        printf("total cycles : %lu\n", comet.cpu.cycle);
        printf("cycles/sec   : %f\n", cycles_per_sec);
    }
    free_memory();

    return EXIT_SUCCESS;
}