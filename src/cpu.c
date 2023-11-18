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

    
//     case 0x02: // outr
//     case 0x03: // outi
//     case 0x04: // inr
//     case 0x05: // ini


//     case 0x06: // jal
//     case 0x07: // jalr
//     case 0x08: // ret
//     case 0x09: // retr
//     case 0x0a: // branch family


//     case 0x0b: // push
//     case 0x0c: // pop
//     case 0x0d: // enter
//     case 0x0e: // leave


//     case 0x10: // load immediate family
//     case 0x11: // lw
//     case 0x12: // lh
//     case 0x13: // lhs
//     case 0x14: // lq
//     case 0x15: // lqs
//     case 0x16: // lb
//     case 0x17: // lbs
//     case 0x18: // sw
//     case 0x19: // sh
//     case 0x1a: // sq
//     case 0x1b: // sb
//     case 0x1c: // sq
//     case 0x1d: // sb


    case 0x1e: // bswp
        {
        u64 val = comet->cpu.registers[ins->rs1];
        val = ((val << 8)  & 0xFF00FF00FF00FF00ull)  | ((val >> 8)  & 0x00FF00FF00FF00FFull);
        val = ((val << 16) & 0xFFFF0000FFFF0000ull)  | ((val >> 16) & 0x0000FFFF0000FFFFull);
        comet->cpu.registers[ins->rs2] = (val << 32) | (val >> 32);
        }
        break;
    case 0x1f: // xch
        {    
        u64 temp = comet->cpu.registers[ins->rs1];
        comet->cpu.registers[ins->rs1] = comet->cpu.registers[ins->rs2];
        comet->cpu.registers[ins->rs2] = temp;
        }

        break;


    case 0x20: // addr
        {
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] + comet->cpu.registers[ins->rs2];
        
        bool carry = (comet->cpu.registers[ins->rs1] > 0 && comet->cpu.registers[ins->rs2] > I64_MAX - comet->cpu.registers[ins->rs1]);
        bool carry_unsigned = (comet->cpu.registers[ins->rs2] > U64_MAX - comet->cpu.registers[ins->rs1]);
        
        set_st_flag(comet->cpu.registers, fl_carry_borrow, carry);
        set_st_flag(comet->cpu.registers, fl_carry_borrow_unsigned, carry_unsigned);
        }

        break;
    case 0x21: // addi
        {
        ins->imm = sign_extend(ins->imm, 16);
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] + ins->imm;
        
        bool carry = (comet->cpu.registers[ins->rs1] > 0 && ins->imm > I64_MAX - comet->cpu.registers[ins->rs1]);
        bool carry_unsigned = (ins->imm > U64_MAX - comet->cpu.registers[ins->rs1]);
        
        set_st_flag(comet->cpu.registers, fl_carry_borrow,          carry);
        set_st_flag(comet->cpu.registers, fl_carry_borrow_unsigned, carry_unsigned);
        }

        break;

    case 0x22: // subr
        {
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] - comet->cpu.registers[ins->rs2];
        
        // this might be complete bullshit
        bool borrow = -comet->cpu.registers[ins->rs2] < I64_MIN - comet->cpu.registers[ins->rs1];
        bool borrow_unsigned = -comet->cpu.registers[ins->rs2] < U64_MIN - comet->cpu.registers[ins->rs1];
        
        set_st_flag(comet->cpu.registers, fl_carry_borrow,          borrow);
        set_st_flag(comet->cpu.registers, fl_carry_borrow_unsigned, borrow_unsigned);
        }

        break;

    case 0x23: // subi
        {
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] - comet->cpu.registers[ins->rs2];
        
        // this might be complete bullshit
        bool borrow = -comet->cpu.registers[ins->rs2] < I64_MIN - comet->cpu.registers[ins->rs1];
        bool borrow_unsigned = -comet->cpu.registers[ins->rs2] < U64_MIN - comet->cpu.registers[ins->rs1];
        
        set_st_flag(comet->cpu.registers, fl_carry_borrow,          borrow);
        set_st_flag(comet->cpu.registers, fl_carry_borrow_unsigned, borrow_unsigned);
        }

        break;

    case 0x24: // imulr
        comet->cpu.registers[ins->rde] = (i64)comet->cpu.registers[ins->rs1] * (i64)comet->cpu.registers[ins->rs2];
        break;

    case 0x25: // imuli
        comet->cpu.registers[ins->rde] = (i64)comet->cpu.registers[ins->rs1] * (i64)ins->imm;
        break;

    case 0x26: // idivr
        if (comet->cpu.registers[ins->rs2] == 0)
            read_u64(comet->ic.ivt_base_address + 8*int_divide_by_zero, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = (i64)comet->cpu.registers[ins->rs1] / (i64)comet->cpu.registers[ins->rs2];
        break;

    case 0x27: // idivi
        if (ins->imm == 0)
            read_u64(comet->ic.ivt_base_address + 8*int_divide_by_zero, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = (i64)comet->cpu.registers[ins->rs1] / (i64)ins->imm;
        break;

    case 0x28: // umulr
        comet->cpu.registers[ins->rde] = (u64)comet->cpu.registers[ins->rs1] * (u64)comet->cpu.registers[ins->rs2];
        break;

    case 0x29: // umuli
        comet->cpu.registers[ins->rde] = (u64)comet->cpu.registers[ins->rs1] * (u64)ins->imm;
        break;

    case 0x2a: // udivr
        if (comet->cpu.registers[ins->rs2] == 0)
            read_u64(comet->ic.ivt_base_address + 8*int_divide_by_zero, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = (u64)comet->cpu.registers[ins->rs1] / (u64)comet->cpu.registers[ins->rs2];
        break;

    case 0x2b: // udivi
        if (ins->imm == 0)
            read_u64(comet->ic.ivt_base_address + 8*int_divide_by_zero, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = (u64)comet->cpu.registers[ins->rs1] / (u64)ins->imm;
        break;

//     case 0x2c: // remr
//     case 0x2d: // remi
//     case 0x2e: // modr
//     case 0x2f: // modi


    case 0x30: // andr
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] & comet->cpu.registers[ins->rs2];
        break;
    case 0x31: // andi
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] & ins->imm;
        break;
    case 0x32: // orr
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] | comet->cpu.registers[ins->rs2];
        break;
    case 0x33: // ori
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] | ins->imm;
        break;
    case 0x34: // norr
        comet->cpu.registers[ins->rde] = ~(comet->cpu.registers[ins->rs1] | comet->cpu.registers[ins->rs2]);
        break;
    case 0x35: // nori
        comet->cpu.registers[ins->rde] = ~(comet->cpu.registers[ins->rs1] | ins->imm);
        break;
    case 0x36: // xorr
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] ^ comet->cpu.registers[ins->rs2];
        break;
    case 0x37: // xori
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] ^ ins->imm;
        break;
    case 0x38: // shlr
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] << comet->cpu.registers[ins->rs2];
        break;
    case 0x39: // shli
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] << ins->imm;
        break;
    case 0x3a: // asrr
        comet->cpu.registers[ins->rde] = (u64)((i64)comet->cpu.registers[ins->rs1] >> comet->cpu.registers[ins->rs2]);
        break;
    case 0x3b: // asri
        comet->cpu.registers[ins->rde] = (u64)((i64)comet->cpu.registers[ins->rs1] >> ins->imm);
        break;
    case 0x3c: // lsrr
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] >> comet->cpu.registers[ins->rs2];
        break;
    case 0x3d: // lsri
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] >> ins->imm;
        break;
    case 0x3e: // bitr
        comet->cpu.registers[ins->rde] = (comet->cpu.registers[ins->rs1] & (1 << comet->cpu.registers[ins->rs2])) >> comet->cpu.registers[ins->rs2];
        break;
    case 0x3f: // biti
        comet->cpu.registers[ins->rde] = (comet->cpu.registers[ins->rs1] & (1 << ins->imm)) >> ins->imm;
        break;


    case 0x40: // pto
    case 0x41: // pfrom
    case 0x42: // pneg
    case 0x43: // pabs
    case 0x44: // padd
    case 0x45: // psub
    case 0x46: // pmul
    case 0x47: // pdiv
        {
        printf("Aphelion Extension P \"Posit Operations\" is not currently supported by comet. go bother kayla about it /j\n");
        read_u64(comet->ic.ivt_base_address + 8*int_invalid_instruction, &comet->cpu.registers[r_pc]);
        }
        break;

    default: {
        char* name = get_ins_name(ins);
        if (name != NULL) {
            printf("unimplemented: \"%s\" opcode 0x%x func 0x%1x\n", name, ins->opcode, ins->func);
            read_u64(comet->ic.ivt_base_address + 8*int_invalid_instruction, &comet->cpu.registers[r_pc]);
            break;
        }
        
        bool success = read_u64(comet->ic.ivt_base_address + 8*int_invalid_instruction, &comet->cpu.registers[r_pc]);
        }
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

void set_st_flag(u64* register_bank, st_flag bit, bool value) {
    register_bank[r_st] &= ~(1ull << bit);
    register_bank[r_st] |= (u64)(value) << bit;
}

bool get_st_flag(u64* register_bank, st_flag bit) {
    return (register_bank[r_st] & (1ull << bit)) >> bit == 1;
}