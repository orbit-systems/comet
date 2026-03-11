#ifndef CORE_H
#define CORE_H

#include <pthread.h>

#include "common/type.h"
#include "common/vec.h"
#include "message.h"
#include "aphelion.h"
#include "mmu.h"

/// Lock definition from v6 spec 
typedef struct {
    bool locked;
    u8 width;
    u64 address;
} AtomicLock;

/// CPU Core 
typedef struct {
    /// CORE INTERNALS

    /// Register file 
    u64 regfile[32];
    
    /// Control registers 
    u64 control_reg[24];

    /// Memory management unit
    Mmu* mmu;

    /// Current lock
    AtomicLock current_lock;

    /// Core ID
    u64 core_id;

    /// Is the CPU core running?
    bool running;
    
    /// Is the core currently spinning for execution?
    bool is_waiting_load_ok;

    /// Will IP increment at the start of the core tick?
    bool will_inc;

    /// SYSTEM MANAGEMENT

    // TODO: change this to cache
    /// System response contents from LOAD_OK
    u64 loaded_value;

    /// Lock for the messages
    pthread_mutex_t message_lock;

    /// Current messages for the core
    Vec(SystemMessage) messages;

    uint64_t pc;
} CpuCore;

/// All types of errors that the core can return
typedef enum: u8 {
    ERROR_NONE,
    ERROR_CORE_INVALID,
} CpuError;

/// Create a new `cpu_core` structure, and initialise it
CpuCore* core_init(void);

/// Enqueue a message onto the core message queue 
CpuError core_enqueue_message(CpuCore* core, SystemMessage message);

/// Core thread entry point
void* core_thread_main(void* arguments);

#define WAIT_ON_MESSAGE_TYPE(type, core) \
        while (core_process_message((core)) != (type)) sched_yield();

#endif /* CORE_H */