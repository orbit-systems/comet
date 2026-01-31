#include "system.h"
#include "lock.h"

static System current_system;

int system_init(void) {
    current_system.messages = vec_new(SystemMessage, 1);
    pthread_mutex_init(&current_system.message_lock, NULL);
    return 0;
}

int system_enqueue_message(SystemMessage message) {
    comet_lock(&current_system.message_lock);
    {
        vec_append(&current_system.messages, message);
    }
    comet_unlock(&current_system.message_lock);

    return 0;
}

