#ifndef SYSTEM_H
#define SYSTEM_H

#include <pthread.h>

#include "common/vec.h"
#include "physmem.h"
#include "message.h"
#include "core.h"


typedef struct {
    /// Queue lock on messages
    pthread_mutex_t message_lock;

    /// Current queue of messages
    Vec(SystemMessage) messages;

    /// Physical memory unit
    PhysMemUnit* phys_mem;

    /// CPU core
    CpuCore* core;
} System;

/// Initialise the system
int system_init(void);

/// Enqueue a message to the system
int system_enqueue_message(SystemMessage message);

/// Install a core into the system
int system_install_core(CpuCore* core);

/* TODO: remove this */
int system_process_message(void);

#endif // SYSTEM_H