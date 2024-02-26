#include "term.h"
#include <stdlib.h>

#ifdef __unix__
#include <termios.h>
#include <unistd.h>

static struct termios old;
static int is_term_set_up = 0;

void term_setup(void) {
    if (is_term_set_up) return;

    tcgetattr(STDIN_FILENO, &old);
    is_term_set_up = 1;

    struct termios cool_state = old;
    cool_state.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &cool_state);
}

void term_reset(void) {
    if (is_term_set_up) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        is_term_set_up = 0;
    }
}
void Exit(void) {
    term_unfuck();
    exit(EXIT_FAILURE);
}
#elif defined(_WIN32)
#warning windows support for term missing, defaulting to nothing
void term_setup(void) {}
void term_unfuck(void) {}
void Exit(void) {
    exit(EXIT_FAILURE);
}
#endif