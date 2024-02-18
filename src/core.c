// ╭───────╮
// │ comet │ the Aphelion ISA reference emulator ☄️
// ╰───────╯
// by kaylatheegg, spsandwichman

// (unfinished) using aphelion v0.4

#include "comet.h"
#include "mem.h"

void print_help() {
    printf("\nusage: comet (path) [flags]\n");
    printf("\n-debug               launch window with debug interface");
    printf("\n-max-cycles:[int]    halt after cycle count has been reached (will run forever if unset)");
    printf("\n-memory:[int]        use a custom address space size; the maximum addressable byte will be [int]-1");
    printf("\n                     if not provided, defaults to 2^26 (64 MiB)");
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
            comet.flag_debug = true;
        } else if (!strcmp(a.key, "-bench")) {
            comet.flag_benchmark = true;
        } else if (!strcmp(a.key, "-max-cycles")) {
            comet.flag_cycle_limit = strtoll(a.val, NULL, 0);
            if (comet.flag_cycle_limit == 0) {
                printf("error: expected positive int, got \"%s\"\n", a.val);
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(a.key, "-memory")) {
            comet.mmu.mem_max = strtoll(a.val, NULL, 0);
            if (comet.mmu.mem_max == 0) {
                printf("error: expected positive int, got \"%s\"\n", a.val);
                exit(EXIT_FAILURE);
            }
        } else {
            if (i == 1 && a.key[0] != '-') {
                comet.flag_bin_path = a.key;
            } else {
                printf("error: unrecognized option \"%s\"\n", a.key);
                exit(EXIT_FAILURE);
            }
        }
    }
}

emulator comet = (emulator){};

int main(int argc, char *argv[]) {

    load_arguments(argc, argv);
    bool mem_init_success = init_memory();
    if (!mem_init_success) {
        printf("crash: virtual memory space could not initialize (ask sandwichman about this)\n");
        exit(EXIT_FAILURE);
    }

    FILE* bin_file = fopen(comet.flag_bin_path, "rb");
    if (bin_file == NULL) {
        printf("error: could not open file \"%s\"\n", comet.flag_bin_path);
        exit(EXIT_FAILURE);
    }

    bool load_img_success = load_image(bin_file);
    if (!load_img_success) {
        printf("crash: accessed but could not load file \"%s\" into memory (ask sandwichman about this)\n", comet.flag_bin_path);
        fclose(bin_file);
        exit(EXIT_FAILURE);
    }

    fclose(bin_file);

    comet.cpu.registers[r_ip] = 0x0;
    comet.cpu.running = true;

    TODO("cpu core");

    free_memory();
    return EXIT_SUCCESS;
}