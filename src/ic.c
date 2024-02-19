#include "comet.h"
#include "mmu.h"
#include "ic.h"

// interrupt handling and interrupt controller

void init_IC() {
    da_init(&comet.ic.queue, 32);
}

// interpret mmu response and push interrupt if applicable
void push_interrupt_from_MMU(mmu_response res) {
    u8 code;
    switch (res) {
    case res_accviolation:
    case res_noperms:
    case res_outofbounds:
        code = int_access_violation;
        break;
    case res_unaligned:
        code = int_unaligned_access;
        break;
    case res_success:
    default: 
        return;
    }
    push_interrupt(code);
}

void push_interrupt(u8 code) {
    if (comet.ic.queue.len == 0) {
        comet.ic.ret_addr = comet.cpu.registers[r_ip];
        comet.ic.ret_status = comet.cpu.registers[r_st];
        set_flag(flag_mode, mode_kernel);
    }
    if (comet.ic.queue.len == comet.ic.queue.cap) {
        // interrupt queue overflow
        da_clear(&comet.ic.queue);
        code = int_interrupt_overflow;
    }
    // hijack instruction pointer
    mmu_response res = phys_read_u64(comet.ic.ivt_base_address + 8*code, &comet.cpu.registers[r_ip]);
    if (res != res_success) {
        push_interrupt_from_MMU(res);
    } else {
        da_push(&comet.ic.queue, (intqueue_entry){ code });
    }
}

// pop interrupt and return to the exit point
void return_interrupt() {
    if (comet.ic.queue.len == 0) return;

    da_pop_front(&comet.ic.queue);
    if (comet.ic.queue.len == 0) {
        comet.cpu.registers[r_ip] = comet.ic.ret_addr;
        comet.cpu.registers[r_st] = comet.ic.ret_status;
    } else {
        // hijack instruction pointer
        u8 code = comet.ic.queue.at[comet.ic.queue.len-1].code;
        mmu_response res = phys_read_u64(comet.ic.ivt_base_address + 8*code, &comet.cpu.registers[r_ip]);
        if (res != res_success) {
            push_interrupt_from_MMU(res);
        }
    }
}

// pop interrupt while resuming execution at the current location
void resolve_interrupt() {
    if (comet.ic.queue.len == 0) return;
    da_pop_front(&comet.ic.queue);
    comet.ic.ret_addr = comet.cpu.registers[r_ip];
    comet.ic.ret_status = comet.cpu.registers[r_st];
    if (comet.ic.queue.len != 0) {
        // hijack instruction pointer
        u8 code = comet.ic.queue.at[comet.ic.queue.len-1].code;
        mmu_response res = phys_read_u64(comet.ic.ivt_base_address + 8*code, &comet.cpu.registers[r_ip]);
        if (res != res_success) {
            push_interrupt_from_MMU(res);
        }
    }
}