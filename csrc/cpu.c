#include <stdio.h>
#include "comet.h"

void exec_instruction(aphelion_cpu_state* cpu, instruction_info* ins) {

    u64 prev_pc = cpu->registers[r_pc];
    bool pc_modified = false;

    switch (ins->opcode) {

    case 0x0A: // nop
        break;
    case 0x10: // int
        interrupt((u8) ins->imm);
        break;
    case 0x11: // inv
        cpu->registers[r_pc] = read_u64(0x28);
        break;
    case 0x12: // usr
        cpu->registers[r_st] |= cpu->registers[r_st] & 0xAull;
        break;

    case 0x20: // li family
        switch (ins->func) {
        case 0: // lli
            cpu->registers[ins->rde] &= 0xFFFFFFFFFFFF0000ull;
            cpu->registers[ins->rde] |= ins->imm;
            break;
        case 1: // llis
            cpu->registers[ins->rde] |= sign_extend(ins->imm, 16);
            break;
        case 2: // lui
            cpu->registers[ins->rde] &= 0xFFFFFFFF0000FFFFull;
            cpu->registers[ins->rde] |= ins->imm << 16;
            break;
        case 3: // luis
            cpu->registers[ins->rde] |= sign_extend(ins->imm, 16) << 16;
            break;
        case 4: // lti
            cpu->registers[ins->rde] &= 0xFFFF0000FFFFFFFFull;
            cpu->registers[ins->rde] |= ins->imm << 32;
            break;
        case 5: // ltis
            cpu->registers[ins->rde] |= sign_extend(ins->imm, 16) << 32;
            break;
        case 6: // ltui
            cpu->registers[ins->rde] &= 0x0000FFFFFFFFFFFFull;
            cpu->registers[ins->rde] |= ins->imm << 48;
            break;
        case 7: // ltuis
            cpu->registers[ins->rde] |= sign_extend(ins->imm, 16) << 48;
            break;
        }
        break;
    
    case 0x21: // lw
        cpu->registers[ins->rde] = read_u64(cpu->registers[ins->rs1] + sign_extend(ins->imm, 16));
        break;
    case 0x22: // lbs
        cpu->registers[ins->rde] = read_u64(cpu->registers[ins->rs1] + sign_extend(ins->imm, 16));
        break;
    case 0x23: // lb
        cpu->registers[ins->rde] &= 0xFFFFFFFFFFFFFF00ull;
        cpu->registers[ins->rde] |= read_u8(cpu->registers[ins->rs1] + sign_extend(ins->imm, 16));
        break;
    case 0x24: // sw
        write_u64(cpu->registers[ins->rs1] + sign_extend(ins->imm, 16), cpu->registers[ins->rde]);
        break;
    case 0x25: // sb
        write_u8(cpu->registers[ins->rs1] + sign_extend(ins->imm, 16), (u8) cpu->registers[ins->rde]);
        break;
    
    case 0x26: // swp
        u64 temp = cpu->registers[ins->rde];
        cpu->registers[ins->rde] = cpu->registers[ins->rs1];
        cpu->registers[ins->rs1] = temp;
        break;
    
    case 0x27: // mov
        cpu->registers[ins->rde] = cpu->registers[ins->rs1];
        break;

    case 0x28: // cmpr
        set_flags_cmpr(cpu, ins);
        break;


    default:
        interrupt(1);
        printf("log: invalid instruction 0x%x\n", ins->opcode);
        break;
    }

}

u64 sign_extend(u64 val, u8 bitsize) {
    return (u64)((i64)(val << (64-bitsize)) >> (64-bitsize));
}

void set_st_flag(aphelion_cpu_state* cpu, u8 bit, bool value) {
    cpu->registers[r_st] &= ~(1ull << bit);
    cpu->registers[r_st] |= (u64)(value) << bit;
}

bool get_st_flag(aphelion_cpu_state* cpu, u8 bit) {
    return (cpu->registers[r_st] & (1ull << bit)) >> bit == 1;
}