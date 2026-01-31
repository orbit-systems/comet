#ifndef CORE_H
#define CORE_H

#include <pthread.h>

#include "common/type.h"
#include "common/vec.h"
#include "system.h"
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
    /// Register file 
    u64 regfile[32];
    
    /// Control registers 
    u64 control_reg[24];

    /// Memory management unit
    Mmu* mmu;

    /// Current lock
    AtomicLock current_lock;

    /// Lock for the messages
    pthread_mutex_t message_lock;

    /// Current messages for the core
    Vec(SystemMessage) messages;
} CpuCore;

/// All types of errors that the core can return
typedef enum: u8 {
    ERROR_NONE,
    ERROR_CORE_INVALID,
} CpuError;

/// Create a new `cpu_core` structure, and initialise it
CpuCore* core_init(void);

CpuError core_enqueue_message(CpuCore* core, SystemMessage message);
CpuError core_write_register(CpuCore* core, AphelGpr reg_idx, u64 value); 
CpuError core_execute_instruction(CpuCore* core, u64 instruction);

#endif /* CORE_H */