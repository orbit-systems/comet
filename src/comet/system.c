#include <assert.h>
#include <inttypes.h>
#include <sys/time.h>

#include "comet.h"
#include "system.h"
#include "core.h"
#include "lock.h"
#include "message.h"
#include "physmem.h"

static System current_system;

char* message_str[] = {
    #define MSG(msg) #msg,
        MESSAGES
    #undef MSG
};

/// Dequeue a message from the system message queue
static SystemMessage system_dequeue_message(void) {
    SystemMessage new_msg;
    new_msg.type = MSG_NONE;
    new_msg.data = NULL;
    comet_lock(&current_system.message_lock);
    {
        if (vec_len(current_system.messages) != 0) {
            new_msg = current_system.messages[0];
            vec_remove_ordered(&current_system.messages, 0);
        }
    }
    comet_unlock(&current_system.message_lock);

    if (new_msg.type != MSG_NONE)
        MDPRINTF("Dequeued message: %s\n", message_str[new_msg.type]);

    return new_msg;
}

int system_init(void) {
    current_system.messages = vec_new(SystemMessage, 1);
    current_system.running = true;
    pthread_mutex_init(&current_system.message_lock, NULL);

    /* Create physical memory unit */
    current_system.phys_mem = physmem_init();

    /* Create 1MB at 0x0 */
    physmem_create_block(current_system.phys_mem, 1 << 20, 0);

    return 0;
}

bool system_is_running(void) {
    return current_system.running;
}

PhysMemUnit* system_get_pmu(void) {
    return current_system.phys_mem;
}

int system_install_core(CpuCore* core) {
    current_system.core = core;
    return 0;
}

int system_enqueue_message(SystemMessage message) {
    comet_lock(&current_system.message_lock);
    {
        MDPRINTF("Enqueued message: %s\n", message_str[message.type]);
        vec_append(&current_system.messages, message);
    }
    comet_unlock(&current_system.message_lock);

    return 0;
}

void* system_thread_main(void* data) {
    (void)data;

    struct timeval prev;
    struct timeval after;

    gettimeofday(&prev, NULL);

    while (current_system.running) {
        gettimeofday(&after, NULL);
        if ((double)(after.tv_sec - prev.tv_sec) >= 1.0)
            break;

        system_process_message();
        sched_yield();
    }


    double elapsed = (double)(after.tv_sec - prev.tv_sec) + (double)(after.tv_usec - prev.tv_usec) * 1e-6;

    WPRINTF("cyc/s: %f\n", current_system.core->pc / elapsed);

    return NULL;
}

int system_process_message(void) {
    SystemMessage new_msg = system_dequeue_message();

    switch (new_msg.type) {
    case MSG_CORE_LOAD:
        /* Get information from the message */
        SystemMessageCoreLoad load = *(SystemMessageCoreLoad*)new_msg.data;
        SDPRINTF("Processing core load, addr: %"PRIx64", size: %"PRIx64"\n", load.addr, load.size);
        SystemMessage load_resp = physmem_read(current_system.phys_mem, load.addr, load.size);
        SDPRINTF("Got core load, resp: %s\n", message_str[load_resp.type]);
        
        core_enqueue_message(current_system.core, load_resp);

        break;

    case MSG_CORE_STOP:
        current_system.running = false;
        break;

    case MSG_NONE:
        break;

    default:
        assert(0 && "Got unhandled message!");
    }

    if (new_msg.data != NULL)
        rca_free(new_msg.data);
    
    return 0;
}