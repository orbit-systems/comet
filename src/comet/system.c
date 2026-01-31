#include <assert.h>

#include "comet.h"
#include "system.h"
#include "core.h"
#include "lock.h"
#include "message.h"

static System current_system;

char* message_str[] = {
    #define MSG(msg) #msg,
        MESSAGES
    #undef MSG
};

/// Dequeue a message from the system message queue
static SystemMessage system_dequeue_message(void) {
    SystemMessage new_msg;
    new_msg.type = MSG_REMOVED;
    comet_lock(&current_system.message_lock);
    // TODO: Fix this terribleness
    {
        for (size_t i = 0; i < vec_len(current_system.messages); i++) {
            if (current_system.messages[i].type != MSG_REMOVED) {
                new_msg = current_system.messages[i];
                current_system.messages[i].type = MSG_REMOVED;
            }
        }
    }
    comet_unlock(&current_system.message_lock);

    DPRINTF("Dequeued message: %s\n", message_str[new_msg.type]);

    return new_msg;
}

int system_init(void) {
    current_system.messages = vec_new(SystemMessage, 1);
    pthread_mutex_init(&current_system.message_lock, NULL);
    return 0;
}

int system_install_core(CpuCore* core) {
    current_system.core = core;
    return 0;
}

int system_enqueue_message(SystemMessage message) {
    comet_lock(&current_system.message_lock);
    {
        DPRINTF("Enqueued message: %s\n", message_str[message.type]);
        vec_append(&current_system.messages, message);
    }
    comet_unlock(&current_system.message_lock);

    return 0;
}


int system_process_message(void) {
    SystemMessage new_msg = system_dequeue_message();
    switch (new_msg.type) {
    case MSG_CORE_LOAD:
        /* Enqueue core load message */
        /* TODO: make this real */
        u64* data = malloc(sizeof(*data));
        *data = 0xDEADBEEF;
        break;
    case MSG_REMOVED:
        break;
    default:
        assert(0 && "Got unhandled message!");
    }
    
    return 0;
}