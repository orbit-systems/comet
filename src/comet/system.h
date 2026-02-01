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

    /// Is the system running?
    bool running;
} System;

/// Initialise the system
int system_init(void);

/// Enqueue a message to the system
int system_enqueue_message(SystemMessage message);

/// Install a core into the system
int system_install_core(CpuCore* core);

/* TODO: remove this */
int system_process_message(void);

/// Is the system currently running?
bool system_is_running(void);

/// Main entry for the system thread
void* system_thread_main(void* data); 

/// Get the physical memory unit for the system
PhysMemUnit* system_get_pmu(void);

#endif // SYSTEM_H