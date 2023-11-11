#include <stdio.h>
#include "comet.h"
#include "mem.c"
#pragma once

void exec_instruction(aphelion_cpu_state* cpu, instruction_info* ins) {

    u64 prev_pc = cpu->registers[r_pc];
    bool pc_modified = false;

    switch (ins->opcode) {

    case 0x0A: // nop
        break;
    case 0x10: // int
        interrupt(cpu, (u8) ins->imm);
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
        {
        u64 temp = cpu->registers[ins->rde];
        cpu->registers[ins->rde] = cpu->registers[ins->rs1];
        cpu->registers[ins->rs1] = temp;
        }
        break;
    case 0x27: // mov
        cpu->registers[ins->rde] = cpu->registers[ins->rs1];
        break;
    case 0x28: // cmpr
        cmpr_set_flags(cpu, ins);
        break;
    case 0x29: // cmpi
        cmpi_set_flags(cpu, ins);
        break;

    case 0x30: // addr
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] + cpu->registers[ins->rs2];
        addr_set_flags(cpu, ins);
        break;
    case 0x31: // addi
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] + sign_extend(ins->imm, 16);
        addi_set_flags(cpu, ins);
        break;
    case 0x32: // adcr
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] + cpu->registers[ins->rs2] + (u64)get_st_flag(cpu, fl_carry);
        addr_set_flags(cpu, ins);
        break;
    case 0x33: // adci
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] + sign_extend(ins->imm, 16) + (u64)get_st_flag(cpu, fl_carry);
        addi_set_flags(cpu, ins);
        break;

    case 0x34: // subr
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] - cpu->registers[ins->rs2];
        subr_set_flags(cpu, ins);
        break;
    case 0x35: // subi
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] - sign_extend(ins->imm, 16);
        subi_set_flags(cpu, ins);
        break;
    case 0x36: // sbbr
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] - cpu->registers[ins->rs2] - (u64) get_st_flag(cpu, fl_borrow);
        subr_set_flags(cpu, ins);
        break;
    case 0x37: // sbbi
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] - sign_extend(ins->imm, 16) - (u64) get_st_flag(cpu, fl_borrow);
        subi_set_flags(cpu, ins);
        break;
    
    case 0x38: // mulr
        cpu->registers[ins->rde] = (u64) ((i64) cpu->registers[ins->rs1] * (i64) cpu->registers[ins->rs2]);
        break;
    case 0x39: // muli
        cpu->registers[ins->rde] = (u64) ((i64) cpu->registers[ins->rs1] * (i64) sign_extend(ins->imm, 16));
        break;
    case 0x3a: // divr
        if (cpu->registers[ins->rs2] == 0) {
            interrupt(cpu, 0); // divide by zero
            break;
        }
        cpu->registers[ins->rde] = (u64)((i64)cpu->registers[ins->rs1] / (i64)cpu->registers[ins->rs2]);
        break;
    case 0x3b: // divi
        if (ins->imm == 0) {
            interrupt(cpu, 0); // divide by zero
            break;
        }
        cpu->registers[ins->rde] = (u64)((i64)cpu->registers[ins->rs1] / (i64)sign_extend(ins->imm, 16));
        break;

    case 0x40: // andr
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] & cpu->registers[ins->rs2];
        break;
    case 0x41: // andi
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] & ins->imm;
        break;
    case 0x42: // orr
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] | cpu->registers[ins->rs2];
        break;
    case 0x43: // ori
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] | ins->imm;
        break;
    case 0x44: // orr
        cpu->registers[ins->rde] = ~(cpu->registers[ins->rs1] | cpu->registers[ins->rs2]);
        break;
    case 0x45: // ori
        cpu->registers[ins->rde] = ~(cpu->registers[ins->rs1] | ins->imm);
        break;
    case 0x46: // xorr
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] ^ cpu->registers[ins->rs2];
        break;
    case 0x47: // xori
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] ^ ins->imm;
        break;
    case 0x48: // shlr
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] << cpu->registers[ins->rs2];
        break;
    case 0x49: // shli
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] << ins->imm;
        break;
    case 0x4a: // asrr
        cpu->registers[ins->rde] = (u64)((i64)cpu->registers[ins->rs1] >> cpu->registers[ins->rs2]);
        break;
    case 0x4b: // asri
        cpu->registers[ins->rde] = (u64)((i64)cpu->registers[ins->rs1] >> ins->imm);
        break;
    case 0x4c: // lsrr
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] >> cpu->registers[ins->rs2];
        break;
    case 0x4d: // lsri
        cpu->registers[ins->rde] = cpu->registers[ins->rs1] >> ins->imm;
        break;
    
    case 0x50: // push
        cpu->registers[r_sp] -= 8;
        write_u64(cpu->registers[r_sp], cpu->registers[ins->rs1]);
        break;
    case 0x51: // pushi
        cpu->registers[r_sp] -= 8;
        write_u64(cpu->registers[r_sp], sign_extend(ins->imm, 16));
        break;
    case 0x52: // pushi
        cpu->registers[r_sp] -= 8;
        write_u64(cpu->registers[r_sp], ins->imm);
        break;
    case 0x53: // pushc
        cpu->registers[r_sp] -= 2;
        write_u16(cpu->registers[r_sp], ins->imm);
        break;
    case 0x54: // pop
        cpu->registers[ins->rde] = read_u64(cpu->registers[r_sp]);
        cpu->registers[r_sp] += 8;
        break;
    case 0x55: // enter
        cpu->registers[r_sp] -= 8;
        write_u64(cpu->registers[r_sp], cpu->registers[r_fp]); // push fp
        cpu->registers[r_fp] = cpu->registers[r_sp];
        break;
    case 0x56: // leave
        cpu->registers[r_sp] = cpu->registers[r_fp];
        cpu->registers[r_fp] = read_u64(cpu->registers[r_sp]);
        cpu->registers[r_sp] += 8;
        break;
    case 0x57: // reloc
        cpu->registers[r_sp] = cpu->registers[ins->rs1];
        cpu->registers[r_fp] = cpu->registers[ins->rs1] - sign_extend(ins->imm, 16);
        break;
    
    case 0x60: // ljal
        cpu->registers[r_sp] -= 8;
        write_u64(cpu->registers[r_sp], cpu->registers[r_pc] + 4);
        cpu->registers[r_pc] = cpu->registers[ins->rs1] + sign_extend(ins->imm, 16);
        break;
    case 0x61: // ljalr
        cpu->registers[ins->rs1] = cpu->registers[r_pc]+4;
        cpu->registers[r_pc] = cpu->registers[ins->rs1] + sign_extend(ins->imm, 16);
        break;
    case 0x62: // ret
        cpu->registers[r_pc] = read_u64(cpu->registers[r_sp]);
        cpu->registers[r_sp] += 8;
        break;
    case 0x63: // retr
        cpu->registers[r_pc] = cpu->registers[ins->rde];
        break;
    case 0x64: // b(cc)
        switch (ins->func) {
        case 0: // bra
            cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
            break;
        case 1: // beq
            if (get_st_flag(cpu, fl_equal)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 2: // bez
            if (get_st_flag(cpu, fl_zero)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 3: // blt
            if (get_st_flag(cpu, fl_less)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 4: // ble
            if (get_st_flag(cpu, fl_less) || get_st_flag(cpu, fl_equal)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 5: // bltu
            if (get_st_flag(cpu, fl_less_unsigned)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 6: // bleu
            if (get_st_flag(cpu, fl_less_unsigned) || get_st_flag(cpu, fl_equal)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 7: // bpe
            if (get_st_flag(cpu, fl_parity)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        
        case 9: // bne
            if (!get_st_flag(cpu, fl_equal)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 10: // bnz
            if (!get_st_flag(cpu, fl_zero)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 11: // bge
            if (get_st_flag(cpu, fl_greater) || get_st_flag(cpu, fl_equal)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 12: // bgt
            if (get_st_flag(cpu, fl_greater)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 13: // bgeu
            if (get_st_flag(cpu, fl_greater_unsigned) || get_st_flag(cpu, fl_equal)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 14: // bgtu
            if (get_st_flag(cpu, fl_greater_unsigned)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        case 15: // bpd
            if (!get_st_flag(cpu, fl_parity)) {
                cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
                pc_modified = true;
            } break;
        default:
            interrupt(cpu, 1);
            printf("log: unrecognized branch func 0x%x\n", ins->func);
            break;
        } break;

    case 0x65: // jal
        cpu->registers[r_sp] -= 8;
        write_u64(cpu->registers[r_sp], cpu->registers[r_pc]+4);
        cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
        pc_modified = true;
        break;
    case 0x66: // ljalr
        cpu->registers[ins->rde] = cpu->registers[r_pc]+4;
        cpu->registers[r_pc] += sign_extend(ins->imm, 20)*4;
        pc_modified = true;
        break;

    default:
        interrupt(cpu, 1);
        //printf("log: unrecognized opcode 0x%x\n", ins->opcode);
        break;
    }

    cpu->registers[r_rz] = 0;
    cpu->increment_next = (prev_pc == cpu->registers[r_pc]) && !pc_modified;

}

void do_cpu_cycle(aphelion_cpu_state* cpu) {
    cpu->cycle++;

    cpu->raw_ins = read_u32(cpu->registers[r_pc]);
    raw_decode(cpu->raw_ins, &cpu->ins_info);
    cpu->registers[r_st] &= 0x00000000FFFFFFFFull;
    cpu->registers[r_st] |= (u64) cpu->raw_ins << 32;

    exec_instruction(cpu, &cpu->ins_info);

    cpu->registers[r_pc] += 4 * (u64) cpu->increment_next;
}

void cmpr_set_flags(aphelion_cpu_state* cpu, instruction_info* ins) {
    set_st_flag(cpu, fl_sign,   cpu->registers[ins->rs1] < 0);
    set_st_flag(cpu, fl_zero,   cpu->registers[ins->rs1] == 0);
    set_st_flag(cpu, fl_parity, countones(cpu->registers[ins->rs1]) % 2 == 0);

    set_st_flag(cpu, fl_equal,                  cpu->registers[ins->rs1] ==      cpu->registers[ins->rs2]);
    set_st_flag(cpu, fl_greater,          (i64) cpu->registers[ins->rs1] > (i64) cpu->registers[ins->rs2]);
    set_st_flag(cpu, fl_less,             (i64) cpu->registers[ins->rs1] < (i64) cpu->registers[ins->rs2]);
    set_st_flag(cpu, fl_greater_unsigned,       cpu->registers[ins->rs1] >       cpu->registers[ins->rs2]);
    set_st_flag(cpu, fl_less_unsigned,          cpu->registers[ins->rs1] <       cpu->registers[ins->rs2]);
}

void cmpi_set_flags(aphelion_cpu_state* cpu, instruction_info* ins) {
    set_st_flag(cpu, fl_sign,   cpu->registers[ins->rs1] < 0);
    set_st_flag(cpu, fl_zero,   cpu->registers[ins->rs1] == 0);
    set_st_flag(cpu, fl_parity, countones(cpu->registers[ins->rs1]) % 2 == 0);

    set_st_flag(cpu, fl_equal,                  cpu->registers[ins->rs1] ==      ins->imm);
    set_st_flag(cpu, fl_greater,          (i64) cpu->registers[ins->rs1] > (i64) ins->imm);
    set_st_flag(cpu, fl_less,             (i64) cpu->registers[ins->rs1] < (i64) ins->imm);
    set_st_flag(cpu, fl_greater_unsigned,       cpu->registers[ins->rs1] >       ins->imm);
    set_st_flag(cpu, fl_less_unsigned,          cpu->registers[ins->rs1] <       ins->imm);
}

void addi_set_flags(aphelion_cpu_state* cpu, instruction_info* ins) {
    set_st_flag(cpu, fl_carry, (i64)(cpu->registers[ins->rde]) < (i64)(cpu->registers[ins->rs1]) || (i64)(cpu->registers[ins->rde]) < (i64)(ins->imm));
    set_st_flag(cpu, fl_carry_unsigned, cpu->registers[ins->rde] < cpu->registers[ins->rs1] || cpu->registers[ins->rde] < ins->imm);
}

void addr_set_flags(aphelion_cpu_state* cpu, instruction_info* ins) {
    set_st_flag(cpu, fl_carry, (i64)(cpu->registers[ins->rde]) < (i64)(cpu->registers[ins->rs1]) || (i64)(cpu->registers[ins->rde]) < (i64)(cpu->registers[ins->rs2]));
    set_st_flag(cpu, fl_carry_unsigned, cpu->registers[ins->rde] < cpu->registers[ins->rs1] || cpu->registers[ins->rde] < cpu->registers[ins->rs2]);
}

void subi_set_flags(aphelion_cpu_state* cpu, instruction_info* ins) {
    set_st_flag(cpu, fl_carry, (i64)(cpu->registers[ins->rde]) > (i64)(cpu->registers[ins->rs1]) || (i64)(cpu->registers[ins->rde]) > (i64)(ins->imm));
    set_st_flag(cpu, fl_carry_unsigned, cpu->registers[ins->rde] > cpu->registers[ins->rs1] || cpu->registers[ins->rde] > ins->imm);
}

void subr_set_flags(aphelion_cpu_state* cpu, instruction_info* ins) {
    set_st_flag(cpu, fl_carry, (i64)(cpu->registers[ins->rde]) > (i64)(cpu->registers[ins->rs1]) || (i64)(cpu->registers[ins->rde]) > (i64)(cpu->registers[ins->rs2]));
    set_st_flag(cpu, fl_carry_unsigned, cpu->registers[ins->rde] > cpu->registers[ins->rs1] || cpu->registers[ins->rde] > cpu->registers[ins->rs2]);
}


u64 sign_extend(u64 val, u8 bitsize) {
    return (u64)((i64)(val << (64-bitsize)) >> (64-bitsize));
}

void set_st_flag(aphelion_cpu_state* cpu, st_flag bit, bool value) {
    cpu->registers[r_st] &= ~(1ull << bit);
    cpu->registers[r_st] |= (u64)(value) << bit;
}

bool get_st_flag(aphelion_cpu_state* cpu, st_flag bit) {
    return (cpu->registers[r_st] & (1ull << bit)) >> bit == 1;
}