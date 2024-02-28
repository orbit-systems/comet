#include "orbit.h"
#include "term.h"
#include <stdlib.h>



static bool is_term_set_up = false;

#if defined(__unix__)
#include <termios.h>
#include <unistd.h>

static struct termios old;

void term_setup(void) {
    if (is_term_set_up) return;

    tcgetattr(STDIN_FILENO, &old);
    is_term_set_up = true;

    struct termios cool_state = old;
    cool_state.c_lflag &= (~ICANON & ~ECHO & ~IGNBRK); // ~IGNBRK ignores ctrl-c and stuff
    tcsetattr(STDIN_FILENO, TCSANOW, &cool_state);
}

void term_reset(void) {
    if (is_term_set_up) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        is_term_set_up = false;
    }
}

void Exit(void) {
    term_reset();
    exit(EXIT_FAILURE);
}
#elif defined(_WIN32)

#include <windows.h>

HANDLE handle_stdin;
HANDLE handle_stdout;

// #warning windows support for term missing, defaulting to nothing
void term_setup(void) {
    if (is_term_set_up) return;

    handle_stdin = GetStdHandle(STD_INPUT_HANDLE);
    handle_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

    if (handle_stdin == INVALID_HANDLE_VALUE || handle_stdout == INVALID_HANDLE_VALUE) {
        CRASH("(windows-specific) could not get stdin/stdout handles");
    }

    SetConsoleMode(handle_stdin,  ENABLE_INSERT_MODE | ENABLE_PROCESSED_INPUT);
    SetConsoleMode(handle_stdout, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

    is_term_set_up = true;

}

void term_reset(void) {
    return;
}

void Exit(void) {
    exit(EXIT_FAILURE);
}
#endif