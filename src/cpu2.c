#include "comet.h"

void exec_instruction(emulator_state* comet, instruction_info* ins) {

    u64 prev_pc = comet->cpu.registers[r_pc];
    bool pc_modified = false;

    switch (ins->opcode) {
    case 0x01: // system control
        switch (ins->func) {
        case 0: // int
            read_u64(comet->ic.ivt_base_address + 8*(ins->func % 256), &comet->cpu.registers[r_pc]);
            break;
        case 1:
            comet->cpu.registers[r_st] |= 1ull << fl_mode;
            break;
        case 2: // fbf
            comet->cpu.registers[r_st] ^= 0b1111111ull;
            break;
        default:
            read_u64(comet->ic.ivt_base_address + 8*int_invalid_instruction, &comet->cpu.registers[r_pc]);
            break;
        }
        break;


    case 0x02: // outr
    case 0x03: // outi
    case 0x04: // inr
    case 0x05: // ini


    case 0x06: // jal
    case 0x07: // jalr
    case 0x08: // ret
    case 0x09: // retr
    case 0x0a: // branch family


    case 0x0b: // push
    case 0x0c: // pop
    case 0x0d: // enter
    case 0x0e: // leave


    case 0x10: // load immediate family
    case 0x11: // lw
    case 0x12: // lh
    case 0x13: // lhs
    case 0x14: // lq
    case 0x15: // lqs
    case 0x16: // lb
    case 0x17: // lbs
    case 0x18: // sw
    case 0x19: // sh
    case 0x1a: // sq
    case 0x1b: // sb
    case 0x1c: // sq
    case 0x1d: // sb


    case 0x1e: // bswp
    case 0x1f: // xch


    case 0x20: // addr
    case 0x21: // addi
    case 0x22: // subr
    case 0x23: // subi
    case 0x24: // imulr
    case 0x25: // imuli
    case 0x26: // idivr
    case 0x27: // idivi
    case 0x28: // umulr
    case 0x29: // umuli
    case 0x2a: // udivr
    case 0x2b: // udivi
    case 0x2c: // remr
    case 0x2d: // remi
    case 0x2e: // modr
    case 0x2f: // modi


    case 0x30: // andr
    case 0x31: // andi
    case 0x32: // orr
    case 0x33: // ori
    case 0x34: // norr
    case 0x35: // nori
    case 0x36: // xorr
    case 0x37: // xori
    case 0x38: // shlr
    case 0x39: // shli
    case 0x3a: // asrr
    case 0x3b: // asri
    case 0x3c: // lsrr
    case 0x3d: // lsri
    case 0x3e: // bitr
    case 0x3f: // biti


    case 0x40: // pto
    case 0x41: // pfrom
    case 0x42: // pneg
    case 0x43: // pabs
    case 0x44: // padd
    case 0x45: // psub
    case 0x46: // pmul
    case 0x47: // pdiv
        printf("unimplemented: 0x%2x %s\n", ins->opcode, get_ins_name(ins));
        read_u64(comet->ic.ivt_base_address + 8*int_invalid_instruction, &comet->cpu.registers[r_pc]);
        break;


    default:
        ;
        bool success = read_u64(comet->ic.ivt_base_address + 8*int_invalid_instruction, &comet->cpu.registers[r_pc]);
        
        break;
    }

    comet->cpu.registers[r_rz] = 0;
    comet->cpu.increment_next = (prev_pc == comet->cpu.registers[r_pc]) && !pc_modified;

}

void do_cpu_cycle(emulator_state* comet) {

    comet->cpu.cycle++;

    // attempt to read instruction
    
    bool success = read_u32(comet->cpu.registers[r_pc], &comet->cpu.raw_ins);
    if (!success) { // if it did not work (unaligned access)
        // retrieve interrupt handler address
        read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        
        // read new instruction
        read_u32(comet->cpu.registers[r_pc], &comet->cpu.raw_ins);
    }

    raw_decode(comet->cpu.raw_ins, &comet->cpu.ins_info);
    comet->cpu.registers[r_st] &= 0x00000000FFFFFFFFull;
    comet->cpu.registers[r_st] |= (u64) comet->cpu.raw_ins << 32;

    exec_instruction(comet, &comet->cpu.ins_info);

    comet->cpu.registers[r_pc] += 4 * (u64) comet->cpu.increment_next;
}