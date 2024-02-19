#include "comet.h"
#include "cpu.h"
#include "dev.h"

void run() {
    
    // the top half of the status register

    // load instruction
    mmu_response res = read_instruction(comet.cpu.registers[r_ip], &current_instr);
    if (res != res_success) {
        push_interrupt_from_MMU(res);
        return; // restart
    }

    comet.cpu.registers[r_ip] += 4;

    switch (current_instr.opcode) {
    
    
    
    
    }

    if (comet.ioc.out_pin) {
        dev_receive(); // run receive handler
    }

    TODO("work lmfao");
}

// yeah
i64 forceinline mod64(i64 a, i64 b) {
    if (b == -1) return 0;
    i64 r = a % b;
    if (r < 0) {
        if (b >= 0) r += b;
        else        r -= b;
    }
    return r;
}