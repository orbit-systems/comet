#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdbit.h>

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

    /* TODO: handle interrupts correctly */

    switch(interrupt) {
    case INT_INVALID:
    case INT_BUSR:
    case INT_BUSW:
    case INT_BUSX:
    case INT_SYSCALL:
    case INT_BREAKPT:
        /* Force jump to 0x0 */
        core->regfile[GPR_IP] = 0;
        core->will_inc = false;
        break;

    default:
        assert(0 && "Unhandled interrupt!");
    }
}

/// Sign extend value to 64 from `size` bits
static inline u64 core_sign_extend(u64 original, u64 size) {
    if (size == 64) return original;
    u64 mask = 1U << (size - 1); 

    u64 zero_above = original & ((1U << size) - 1);
    return (zero_above ^ mask) - mask;
}

/// Zero extend value to 64 from `size` bits
static inline u64 core_zero_extend(u64 original, u64 size) {
    (void)size;
    return original;
}

/// Signed right shift by `shamt`
static inline u64 core_isr(u64 original, u64 shamt) {
    u64 ret_val = original >> shamt;
    if ((original >> 63) & 1) {
        u64 mask = ((1 << shamt) - 1) << (64 - shamt);
        ret_val |= mask;
    }

    return ret_val;
}

/// Write to register `reg_idx` with value `value`
static inline CpuError core_write_register(CpuCore* core, AphelGpr reg_idx, u64 value) {
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

/// Read from register `reg_idx`
static inline u64 core_read_register(CpuCore* core, AphelGpr reg_idx) {
    return core->regfile[reg_idx];
}

/// Execute an instruction on the core
static CpuError core_execute_instruction(CpuCore* core, u32 instruction) {
    AphelDecodedInst inst;
    inst.inst = instruction;
    DPRINTF("Got inst: %"PRIx32", op: %s\n", instruction, op_name[inst.fmtA.op]);

    switch (inst.fmtA.op) {
    case OP_SSI: { // fmtA
        u16 val = inst.fmtA.imm >> 3;
        u16 sh = (inst.fmtA.imm >> 1) & 0b11;
        u16 c = inst.fmtA.imm & 1;
        
        u16 shift = sh << 4;
        u8 r1 = inst.fmtA.r1;

        if (c == 1) {
            core_write_register(core, r1, ((i64)val << 48) >> (64 - shift));
        } else {
            u64 mask = ~(0xFFFF << shift);
            core_write_register(core, r1, (core_read_register(core, r1) & mask) | (val << shift));
        }

        break;
    }
    case OP_FENCE: { // fmtA
        /* TODO: handle active memory operations */

        break;
    }
    case OP_CINVAL: {
        /* TODO: implement cache */

        break;
    }
    case OP_CFETCH: {
        /* TODO: implement cache */
    
        break;
    }
    case OP_JLR: { // fmtB
        u64 ip = core_read_register(core, GPR_IP);
        core->regfile[GPR_IP] += core_read_register(core, inst.fmtB.r2) + (core_zero_extend(inst.fmtB.imm, 14) << 2);
        core_write_register(core, inst.fmtB.r1, ip);
        break;
    }
    case OP_JL: { // fmtB
        u64 ip = core_read_register(core, GPR_IP);
        core->regfile[GPR_IP] = core_read_register(core, inst.fmtB.r2) + (core_zero_extend(inst.fmtB.imm, 14) << 2);
        core_write_register(core, inst.fmtB.r1, ip);
        break;
    }
    case OP_BZ: { // fmtA
        u8 r1 = inst.fmtA.r1;
        if (core_read_register(core, r1) == 0) {
            core->regfile[GPR_IP] += (core_sign_extend(inst.fmtA.imm, 19) << 2);
        }
        break;
    }
    case OP_BN: { // fmtA
        u8 r1 = inst.fmtA.r1;
        if (core_read_register(core, r1) != 0) {
            core->regfile[GPR_IP] += (core_sign_extend(inst.fmtA.imm, 19) << 2);
        }
        break;
    }
    case OP_SYSCALL: // fmtA
        core_trigger_interrupt(core, INT_SYSCALL);
        break;

    case OP_BREAKPT: // fmtA
        core_trigger_interrupt(core, OP_BREAKPT);
        break;
    case OP_SPIN: // fmtA
        /* TODO: handle spin hint */
        break;
    
    case OP_IRET: // fmtA
        if ((core->control_reg[CTRL_STAT] & STAT_U_MASK) != 0) {
            core_trigger_interrupt(core, INT_INVALID);
            break;
        }
        core->control_reg[CTRL_STAT] = core->control_reg[CTRL_INTSTAT];
        core->regfile[GPR_IP] = core->control_reg[CTRL_INTIP];
        break;

    case OP_LCTRL: // fmtA
        if ((core->control_reg[CTRL_STAT] & STAT_U_MASK) != 0 || !aphel_is_valid_ctrl(inst.fmtA.imm)) {
            core_trigger_interrupt(core, INT_INVALID);
            break;
        }
        core_write_register(core, inst.fmtA.r1, core->control_reg[inst.fmtA.imm]);
        break;

    case OP_SCTRL: // fmtA
        if ((core->control_reg[CTRL_STAT] & STAT_U_MASK) != 0 || !aphel_is_valid_ctrl(inst.fmtA.imm)) {
            core_trigger_interrupt(core, INT_INVALID);
            break;
        }
        core->control_reg[inst.fmtA.imm] = core_read_register(core, inst.fmtA.r1);
        break;
    
    case OP_WAIT: { // fmtA
        /* TODO: implement WAIT */
        break;
    }

    case OP_ADDI: // fmtB
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) + core_zero_extend(inst.fmtB.imm, 14));
        break;

    case OP_SUBI: // fmtB
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) - core_zero_extend(inst.fmtB.imm, 14));
        break;
    
    case OP_MULI: // fmtB
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) * core_sign_extend(inst.fmtB.imm, 14));
        break;
    
    case OP_UDIVI: // fmtB
        if (inst.fmtB.imm == 0) {
            core_write_register(core, inst.fmtB.r1, UINT64_MAX);
            break;
        }
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) / core_zero_extend(inst.fmtB.imm, 14));
        break;

    case OP_IDIVI: // fmtB
        if (inst.fmtB.imm == 0) {
            core_write_register(core, inst.fmtB.r1, UINT64_MAX);
            break;
        }
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) / core_sign_extend(inst.fmtB.imm, 14));
        break;

    case OP_UREMI: // fmtB
        if (inst.fmtB.imm == 0) {
            core_write_register(core, inst.fmtB.r1, UINT64_MAX);
            break;
        }
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) % core_zero_extend(inst.fmtB.imm, 14));
        break;

    case OP_IREMI: // fmtB
        if (inst.fmtB.imm == 0) {
            core_write_register(core, inst.fmtB.r1, UINT64_MAX);
            break;
        }
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) % core_sign_extend(inst.fmtB.imm, 14));
        break;
    
    case OP_AND: // fmtC
        core_write_register(core, inst.fmtC.r1, core_read_register(core, inst.fmtC.r2) & (core_read_register(core, inst.fmtC.r3) | core_zero_extend(inst.fmtC.imm, 9)));
        break;
    
    case OP_OR: // fmtC
        core_write_register(core, inst.fmtC.r1, core_read_register(core, inst.fmtC.r2) | (core_read_register(core, inst.fmtC.r3) | core_zero_extend(inst.fmtC.imm, 9)));
        break;
    
    case OP_NOR: // fmtC
        core_write_register(core, inst.fmtC.r1, ~(core_read_register(core, inst.fmtC.r2) | (core_read_register(core, inst.fmtC.r3) | core_zero_extend(inst.fmtC.imm, 9))));
        break;    
    
    case OP_XOR: // fmtC
        core_write_register(core, inst.fmtC.r1, core_read_register(core, inst.fmtC.r2) ^ (core_read_register(core, inst.fmtC.r3) | core_zero_extend(inst.fmtC.imm, 9)));
        break;

    case OP_ANDI: // fmtB
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) & core_zero_extend(inst.fmtB.imm, 14));
        break;

    case OP_ORI: // fmtB
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) | core_zero_extend(inst.fmtB.imm, 14));
        break;

    case OP_NORI: // fmtB
        core_write_register(core, inst.fmtB.r1, ~(core_read_register(core, inst.fmtB.r2) | core_zero_extend(inst.fmtB.imm, 14)));
        break;

    case OP_XORI: // fmtB
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) ^ core_zero_extend(inst.fmtB.imm, 14));
        break;
    
    case OP_SL: { // fmtC
        u16 shamt = (core_read_register(core, inst.fmtC.r3) + core_zero_extend(inst.fmtC.imm, 9)) & 0b1111111;
        core_write_register(core, inst.fmtC.r1, core_read_register(core, inst.fmtC.r2) << shamt);
        break;
    }

    case OP_USR: { // fmtC
        u16 shamt = (core_read_register(core, inst.fmtC.r3) + core_zero_extend(inst.fmtC.imm, 9)) & 0b1111111;
        core_write_register(core, inst.fmtC.r1, core_read_register(core, inst.fmtC.r2) >> shamt);
        break;
    }

    case OP_ISR: { // fmtC
        u16 shamt = (core_read_register(core, inst.fmtC.r3) + core_zero_extend(inst.fmtC.imm, 9)) & 0b1111111;
        core_write_register(core, inst.fmtC.r1, core_isr(core_read_register(core, inst.fmtC.r2), shamt));
        
        break;
    }

    case OP_SI: { // fmtB
        u8 i = (inst.fmtB.imm >> 12) & 1;
        u8 rsh = (inst.fmtB.imm >> 6) & 0b111111;
        u8 lsh = inst.fmtB.imm & 0b111111;
        u64 r2_val = core_read_register(core, inst.fmtB.r2);

        if (i == 0) {
            core_write_register(core, inst.fmtB.r1, (r2_val << lsh) >> rsh);
        } else {
            core_write_register(core, inst.fmtB.r1, core_isr(r2_val << lsh, rsh)); 
        }
        
        break;
    }

    case OP_CB: { // fmtB
        u8 rsh = (inst.fmtB.imm >> 6) & 0b111111;
        u8 lsh = inst.fmtB.imm & 0b111111;
        core_write_register(core, inst.fmtB.r1, core_read_register(core, inst.fmtB.r2) & ~((UINT64_MAX << lsh) >> rsh));
        break;
    }

    case OP_ROR: { // fmtC
        u16 shamt = (core_read_register(core, inst.fmtC.r3) + core_zero_extend(inst.fmtC.imm, 9)) & 0b1111111;
        u64 r2_val = core_read_register(core, inst.fmtB.r2);
        core_write_register(core, inst.fmtB.r1, (r2_val >> shamt) | (r2_val << (64 - shamt)));
        break;
    }

    case OP_ROL: { // fmtC
        u16 shamt = (core_read_register(core, inst.fmtC.r3) + core_zero_extend(inst.fmtC.imm, 9)) & 0b1111111;
        u64 r2_val = core_read_register(core, inst.fmtB.r2);
        core_write_register(core, inst.fmtB.r1, (r2_val << shamt) | (r2_val >> (64 - shamt)));
        break;
    }

    case OP_REV: { // fmtB
        u16 set = inst.fmtB.imm & 0b111111;
        u64 r2_val = core_read_register(core, inst.fmtB.r2);

        if ((set & 0b100000) != 0) r2_val = (0xFFFFFFFF00000000 & r2_val) >> 32 | (0x00000000FFFFFFFF & r2_val) << 32;
        if ((set & 0b10000) != 0) r2_val = (0xFFFF0000FFFF0000 & r2_val) >> 16 | (0x0000FFFF0000FFFF & r2_val) << 16;
        if ((set & 0b1000) != 0) r2_val = (0xFF00FF00FF00FF00 & r2_val) >> 8 | (0x00FF00FF00FF00FF & r2_val) << 8;
        if ((set & 0b100) != 0) r2_val = (0xF0F0F0F0F0F0F0F0 & r2_val) >> 4 | (0x0F0F0F0F0F0F0F0F & r2_val) << 4;
        if ((set & 0b10) != 0) r2_val = (0xCCCCCCCCCCCCCCCC & r2_val) >> 2 | (0x3333333333333333 & r2_val) << 2;
        if ((set & 0b1) != 0) r2_val = (0xAAAAAAAAAAAAAAAA & r2_val) >> 1 | (0x5555555555555555 & r2_val) << 1;
        
        core_write_register(core, inst.fmtB.r1, r2_val);
        break;
    }
    
    case OP_CSB: // fmtB
        core_write_register(core, inst.fmtB.r1, stdc_count_ones(core_read_register(core, inst.fmtB.r2)));
        break;

    case OP_CTZ: // fmtB
        /* builtin only works if value != 0*/
        if (core_read_register(core, inst.fmtB.r2)) {
            core_write_register(core, inst.fmtB.r1, 0);
            break;
        }
        core_write_register(core, inst.fmtB.r1, stdc_trailing_zeros(core_read_register(core, inst.fmtB.r2)));
        break;

    case OP_EXT: { // fmtC
        u64 r2_val = core_read_register(core, inst.fmtC.r2);
        u64 r3_val = core_read_register(core, inst.fmtC.r3);
        u64 result = 0;
        u16 k = 0;
        for (u64 i = 0; i < 64; i++) {
            if (((1 << i) & r3_val) != 0) {
                u64 bit = (r2_val >> i) & 1;
                result |= bit << k;
                k += 1;
            }
        }

        core_write_register(core, inst.fmtC.r1, result);
        break;
    }
    
    case OP_DEP: { // fmtC
        u64 r2_val = core_read_register(core, inst.fmtC.r2);
        u64 r3_val = core_read_register(core, inst.fmtC.r3);
        u64 result = 0;
        u16 k = 0;
        for (u64 i = 0; i < 64; i++) {
            if (((1 << i) & r3_val) != 0) {
                u64 bit = (r2_val >> k) & 1;
                result |= bit << i;
                k += 1;
            }
        }

        core_write_register(core, inst.fmtC.r1, result);
        break;
    }

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