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
#include "system.h"

/// Trigger interrupt in the core
static void core_trigger_interrupt(CpuCore* core, AphelInterrupt interrupt) {
    assert(interrupt <= INT_COUNT && "Out of bounds interrupt!");

    DPRINTF("Firing interrupt: %s\n", int_name[interrupt]);

    switch(interrupt) {
    case INT_INVALID:
    case INT_BUSR:
    case INT_BUSW:
    case INT_BUSX:
        /* Force jump to 0x0 */
        core->regfile[GPR_IP] = 0;
        core->will_inc = false;
        break;

    default:
        assert(0 && "Unhandled interrupt!");
    }
}

/// Sign extend value to 64 bits
static u64 core_sign_extend(u64 original, u64 size) {
    if (size == 64) return original;
    u64 mask = 1U << (size - 1); 

    u64 zero_above = original & ((1U << size) - 1);
    return (zero_above ^ mask) - mask;
}

/// Execute an instruction on the core
static CpuError core_execute_instruction(CpuCore* core, u32 instruction) {
    AphelDecodedInst inst;
    inst.inst = instruction;
    DPRINTF("Got inst: %"PRIx32", op: %s\n", instruction, op_name[inst.fmtA.op]);

    switch (inst.fmtA.op) {
    case OP_BZ: // FmtA
        u8 r1 = inst.fmtA.r1;
        printf("IMM: %"PRIx32"\n", inst.fmtA.imm);
        if (core->regfile[r1] == 0) {
            DPRINTF("Updating IP to %"PRIx64"\n", core->regfile[GPR_IP] + (core_sign_extend(inst.fmtA.imm, 19) << 2));
            core->regfile[GPR_IP] = core->regfile[GPR_IP] + (core_sign_extend(inst.fmtA.imm, 19) << 2);
        }
        break;
    default:
        WPRINTF("Unknown opcode: 0x%02x\n", inst.fmtA.op);
        core_trigger_interrupt(core, INT_INVALID);
        /* TODO: remove this */
        return ERROR_CORE_INVALID;
    }

    return ERROR_NONE;
}

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
    new_msg.type = MSG_NONE;
    new_msg.data = NULL;
    comet_lock(&core->message_lock);
    {
        if (vec_len(core->messages) != 0) {
            new_msg = core->messages[0];
            vec_remove_ordered(&core->messages, 0);
        }
    }
    comet_unlock(&core->message_lock);

    if (new_msg.type != MSG_NONE) 
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

SystemMessageType core_process_message(CpuCore* core) {
    SystemMessage curr_msg = core_dequeue_message(core);

    switch (curr_msg.type) {
    case MSG_SYS_STOP:
        DPRINTF("Stopping core\n");
        core->running = false;
        break;

    case MSG_SYS_LOAD_OK:
        /* Get information about message and free */
        u32 data = *(u32*)curr_msg.data;

        /* Store loaded information inside the core */
        /* TODO: Store this inside of D$ */
        core->loaded_value = data;
        break;

    case MSG_SYS_LOAD_BAD_ADDR:
        /* Trigger an interrupt! */
        if (core->is_waiting_load_ok) {
            /* Trigger a BUSX interrupt */
            core_trigger_interrupt(core, INT_BUSX);
        } else {
            core_trigger_interrupt(core, INT_BUSR);
        }

        assert(curr_msg.data != NULL && "Expected MSG_SYS_BAD_LOAD_ADDR's data field to be non-null!");

        /* Write to intval */
        core->control_reg[CTRL_INTVAL] = *(u64*)curr_msg.data;
        break;
        
    case MSG_NONE:
        break;

    default:
        assert(0 && "Got unhandled message!");
    }

    if (curr_msg.data != NULL)
        rca_free(curr_msg.data);

    return curr_msg.type;
}

void* core_thread_main(void* arguments) {
    DPRINTF("Running core\n");
    CpuCore* core = (CpuCore*)arguments;
    while (core->running) {
        /* Get instruction from main memory */
        if (core->will_inc == false) {
            core->will_inc = true;
        } else {
            core->regfile[GPR_IP] += 4;
        }

        SystemMessageCoreLoad load = (SystemMessageCoreLoad){.addr = core->regfile[GPR_IP], .size = 4};
        system_enqueue_message(CREATE_MESSAGE(MSG_CORE_LOAD, load, sizeof(load)));
        
        u32 instruction = 0;
        bool bad_load = false;
        /* Process messages until LOAD_OK occurs */
        core->is_waiting_load_ok = true;
        DPRINTF("Waiting on LOAD_OK\n");
        while (core->is_waiting_load_ok) {
            SystemMessageType curr_msg = core_process_message(core);
            switch (curr_msg) {
            case MSG_SYS_LOAD_BAD_ADDR:
                core->is_waiting_load_ok = false;
                bad_load = true;
                break;
            case MSG_SYS_LOAD_OK:
                core->is_waiting_load_ok = false;
                break;
            default:
                break;
            }
        }

        /* Skip rest of the CPU kick, continue to next inst */
        if (bad_load == true) 
            continue;

        instruction = core->loaded_value;

        switch (core_execute_instruction(core, instruction)) {
            case ERROR_NONE:
                break;

            case ERROR_CORE_INVALID:
                WPRINTF("Invalid core state detected, stopping execution!\n");
                core->running = false;
                break;

            default:
                assert(0 && "Unhandled core error\n");
        }

        sched_yield();
    }

    /* Core is no longer running, signal to the harness to stop */
    /* TODO: This isn't always required, since the aphelion core could be halted. Change this for SMP */
    system_enqueue_message(CREATE_MESSAGE(MSG_CORE_STOP, NULL, 0));

    return NULL;
}