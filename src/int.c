#include "comet.h"
#include "mem.h"

// interrupt handling and interrupt controller

void init_IC() {
    da_init(&comet.ic.queue, 32);
}

void push_interrupt(u8 code) {
    if (comet.ic.queue.len == 0) {
        comet.ic.ret_addr = comet.cpu.registers[r_ip];
        comet.ic.ret_status = comet.cpu.registers[r_st];
        set_flag(flag_mode, true);
    }
    if (comet.ic.queue.len == comet.ic.queue.cap) {
        // interrupt queue overflow
        da_clear(&comet.ic.queue);
        code = int_interrupt_overflow;
    }
    mmu_response res = phys_read_u64(comet.ic.ivt_base_address + 8*code, &comet.cpu.registers[r_ip]);
    if (res != res_success) {
        TODO("issue proper interrupts in the IC");
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

    }
}

// pop interrupt while resuming execution at the current location
void resolve_interrupt() {
    if (comet.ic.queue.len == 0) return;
}