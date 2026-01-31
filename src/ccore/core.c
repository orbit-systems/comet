#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <inttypes.h>

#include "core.h"
#include "comet.h"
#include "aphelion.h"
#include "lock.h"
#include "common/vec.h"
#include "message.h"

CpuCore* core_init(void) {
    CpuCore* new_core = malloc(sizeof(*new_core));
    memset(new_core, 0, sizeof(*new_core));
    pthread_mutex_init(&new_core->message_lock, NULL);

    new_core->messages = vec_new(SystemMessage, 1);

    new_core->running = true;

    return new_core;
}

CpuError core_enqueue_message(CpuCore* core, SystemMessage message) {
    comet_lock(&core->message_lock);
    {
        DPRINTF("Enqueued message: %s\n", message_str[message.type]);
        vec_append(&core->messages, message);
    }
    comet_unlock(&core->message_lock);
    return ERROR_NONE;
}

SystemMessage core_dequeue_message(CpuCore* core) {
    SystemMessage new_msg;
    new_msg.type = MSG_REMOVED;
    comet_lock(&core->message_lock);
    // TODO: Fix this terribleness
    {
        for (size_t i = 0; i < vec_len(core->messages); i++) {
            if (core->messages[i].type != MSG_REMOVED) {
                new_msg = core->messages[i];
                core->messages[i].type = MSG_REMOVED;
            }
        }
    }
    comet_unlock(&core->message_lock);

    DPRINTF("Dequeued message: %s\n", message_str[new_msg.type]);
    return new_msg;
}

CpuError core_write_register(CpuCore* core, AphelGpr reg_idx, u64 value) {
    assert(reg_idx < GPR_COUNT && "Register index larger than Aphelion register file!");

    switch(reg_idx) {
    case GPR_ZR:
    case GPR_IP:
        break;
    default:
        core->regfile[reg_idx] = value;
    break;
    }

    return ERROR_NONE;
}

void* core_thread_main(void* arguments) {
    DPRINTF("Running core\n");
    CpuCore* core = (CpuCore*)arguments;
    while (core->running) {
        SystemMessage curr_msg = core_dequeue_message(core);
        switch (curr_msg.type) {
        case MSG_SYS_STOP:
            DPRINTF("Stopping core\n");
            core->running = false;
            break;
        case MSG_SYS_LOAD_OK:
            /* Get information about message and free */
            u64 data = *(u64*)curr_msg.data;
            printf("Got data: %"PRIx64"\n", data);
            free(curr_msg.data);
            /* Spoof stop message */
            core_enqueue_message(core, (SystemMessage){.type = MSG_SYS_STOP});
            break;

        case MSG_REMOVED:
            break;
        default:
            assert(0 && "Got unhandled message!");
        }
        sched_yield();
    }

    return NULL;
}


CpuError core_execute_instruction(CpuCore* core, u64 instruction) {

    return ERROR_NONE;
}