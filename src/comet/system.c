#include "system.h"

static System current_system;

/* TODO: make this thread safe */
int system_enqueue_message(SystemMessage message) {
    vec_append(&current_system.messages, message);
}

