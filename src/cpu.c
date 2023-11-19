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


    case 0x0b: // push
        {
        bool success = write_u64(comet->cpu.registers[r_sp] - 8, comet->cpu.registers[ins->rs1]);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[r_sp] -= 8;
        break;
        }
    case 0x0c: // pop
        {
        bool success = read_u64(comet->cpu.registers[r_sp], &comet->cpu.registers[ins->rde]);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[r_sp] += 8;
        break;
        }
    case 0x0d: // enter
        {
        bool success = write_u64(comet->cpu.registers[r_sp] - 8, comet->cpu.registers[r_fp]);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        else {
            comet->cpu.registers[r_sp] -= 8;
            comet->cpu.registers[r_fp] = comet->cpu.registers[r_sp];
        }
        break;
        }
    case 0x0e: // leave
        {
        u64 prev_fp = comet->cpu.registers[r_fp];
        bool success = read_u64(prev_fp, &comet->cpu.registers[r_fp]);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        else {
            comet->cpu.registers[r_sp] = prev_fp;
            comet->cpu.registers[r_sp] += 8;
        }
        break;

        }

    case 0x10: // load immediate family
        switch (ins->func) {
        case 0: // lli
            comet->cpu.registers[ins->rde] &= 0xFFFFFFFFFFFF0000ull;
            comet->cpu.registers[ins->rde] |= ins->imm;
            break;
        case 1: // llis
            comet->cpu.registers[ins->rde] |= sign_extend(ins->imm, 16);
            break;
        case 2: // lui
            comet->cpu.registers[ins->rde] &= 0xFFFFFFFF0000FFFFull;
            comet->cpu.registers[ins->rde] |= ins->imm << 16;
            break;
        case 3: // luis
            comet->cpu.registers[ins->rde] |= sign_extend(ins->imm, 16) << 16;
            break;
        case 4: // lti
            comet->cpu.registers[ins->rde] &= 0xFFFF0000FFFFFFFFull;
            comet->cpu.registers[ins->rde] |= ins->imm << 32;
            break;
        case 5: // ltis
            comet->cpu.registers[ins->rde] |= sign_extend(ins->imm, 16) << 32;
            break;
        case 6: // ltui
            comet->cpu.registers[ins->rde] &= 0x0000FFFFFFFFFFFFull;
            comet->cpu.registers[ins->rde] |= ins->imm << 48;
            break;
        case 7: // ltuis
            comet->cpu.registers[ins->rde] |= sign_extend(ins->imm, 16) << 48;
            break;
        }
        break;
    case 0x11: // lw
        {
        bool success = read_u64(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), &comet->cpu.registers[ins->rde]);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        }
        break;
    case 0x12: // lh
        {
        u32 temp;
        bool success = read_u32(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), &temp);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        else {
            comet->cpu.registers[ins->rde] &= 0xFFFFFFFF00000000;
            comet->cpu.registers[ins->rde] |= temp;
        }
        }
        break;
    case 0x13: // lhs
        {
        u32 temp;
        bool success = read_u32(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), &temp);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = sign_extend(temp, 32);
        }
        break;
    case 0x14: // lq
        {
        u16 temp;
        bool success = read_u16(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), &temp);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        else {
            comet->cpu.registers[ins->rde] &= 0xFFFFFFFFFFFF0000;
            comet->cpu.registers[ins->rde] |= temp;
        }
        }
        break;
    case 0x15: // lqs
        {
        u16 temp;
        bool success = read_u16(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), &temp);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = sign_extend(temp, 16);
        }
        break;
    case 0x16: // lb
        {
        u8 temp;
        bool success = read_u8(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), &temp);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        else {
            comet->cpu.registers[ins->rde] &= 0xFFFFFFFFFFFFFF00;
            comet->cpu.registers[ins->rde] |= temp;
        }
        }
        break;
    case 0x17: // lbs
        {
        u8 temp;
        bool success = read_u8(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), &temp);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = sign_extend(temp, 8);
        }
        break;
    case 0x18: // sw
        {
        bool success = write_u64(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), comet->cpu.registers[ins->rde]);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        }
        break;
    case 0x19: // sh
        {
        bool success = write_u32(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), comet->cpu.registers[ins->rde]);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        }
        break;
    case 0x1a: // sq
        {
        bool success = write_u16(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), comet->cpu.registers[ins->rde]);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        }
        break;
    case 0x1b: // sb
        {
        bool success = write_u8(comet->cpu.registers[ins->rs1] + sign_extend(ins->imm, 16), comet->cpu.registers[ins->rde]);
        if (!success)
            read_u64(comet->ic.ivt_base_address + 8*int_unaligned_access, &comet->cpu.registers[r_pc]);
        }
        break;


    case 0x1c: // bswp
        {
        u64 val = comet->cpu.registers[ins->rs1];
        val = ((val << 8)  & 0xFF00FF00FF00FF00ull)  | ((val >> 8)  & 0x00FF00FF00FF00FFull);
        val = ((val << 16) & 0xFFFF0000FFFF0000ull)  | ((val >> 16) & 0x0000FFFF0000FFFFull);
        comet->cpu.registers[ins->rs2] = (val << 32) | (val >> 32);
        }
        break;
    case 0x1d: // xch
        {
        u64 temp = comet->cpu.registers[ins->rs1];
        comet->cpu.registers[ins->rs1] = comet->cpu.registers[ins->rs2];
        comet->cpu.registers[ins->rs2] = temp;
        }
        break;

//     case 0x1e: // cmpr
//     case 0x1f: // cmpr


    case 0x20: // addr
        {
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] + comet->cpu.registers[ins->rs2];
        
        u64  scratch; // to discard result of builtin checks
        bool carry =          __builtin_saddll_overflow(comet->cpu.registers[ins->rs1], comet->cpu.registers[ins->rs2], (i64*)&scratch);
        bool carry_unsigned = __builtin_uaddll_overflow(comet->cpu.registers[ins->rs1], comet->cpu.registers[ins->rs2], &scratch);
        
        set_st_flag(comet->cpu.registers, fl_carry_borrow, carry);
        set_st_flag(comet->cpu.registers, fl_carry_borrow_unsigned, carry_unsigned);
        }

        break;
    case 0x21: // addi
        {
        ins->imm = sign_extend(ins->imm, 16);
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] + ins->imm;
        
        u64  scratch;
        bool carry =          __builtin_saddll_overflow(comet->cpu.registers[ins->rs1], ins->imm, (i64*)&scratch);
        bool carry_unsigned = __builtin_uaddll_overflow(comet->cpu.registers[ins->rs1], ins->imm, &scratch);
        
        set_st_flag(comet->cpu.registers, fl_carry_borrow,          carry);
        set_st_flag(comet->cpu.registers, fl_carry_borrow_unsigned, carry_unsigned);
        }

        break;

    case 0x22: // subr
        {
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] - comet->cpu.registers[ins->rs2];
        
        u64  scratch;
        bool borrow =          __builtin_ssubll_overflow(comet->cpu.registers[ins->rs1], comet->cpu.registers[ins->rs2], (i64*)&scratch);
        bool borrow_unsigned = __builtin_usubll_overflow(comet->cpu.registers[ins->rs1], comet->cpu.registers[ins->rs2], &scratch);
        
        set_st_flag(comet->cpu.registers, fl_carry_borrow,          borrow);
        set_st_flag(comet->cpu.registers, fl_carry_borrow_unsigned, borrow_unsigned);
        }

        break;

    case 0x23: // subi
        {
        ins->imm = sign_extend(ins->imm, 16);
        comet->cpu.registers[ins->rde] = comet->cpu.registers[ins->rs1] - ins->imm;
        
        u64  scratch;
        bool borrow =          __builtin_ssubll_overflow(comet->cpu.registers[ins->rs1], ins->imm, (i64*)&scratch);
        bool borrow_unsigned = __builtin_usubll_overflow(comet->cpu.registers[ins->rs1], ins->imm, &scratch);
        
        set_st_flag(comet->cpu.registers, fl_carry_borrow,          borrow);
        set_st_flag(comet->cpu.registers, fl_carry_borrow_unsigned, borrow_unsigned);
        }

        break;

    case 0x24: // imulr
        comet->cpu.registers[ins->rde] = (i64)comet->cpu.registers[ins->rs1] * (i64)comet->cpu.registers[ins->rs2];
        break;

    case 0x25: // imuli
        comet->cpu.registers[ins->rde] = (i64)comet->cpu.registers[ins->rs1] * (i64)sign_extend(ins->imm, 16);
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
            comet->cpu.registers[ins->rde] = (i64)comet->cpu.registers[ins->rs1] / (i64)sign_extend(ins->imm, 16);
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

    // this might also be bullshit?
    // TODO test this too
    case 0x2c: // remr
        if (comet->cpu.registers[ins->rs2] == 0)
            read_u64(comet->ic.ivt_base_address + 8*int_divide_by_zero, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = (i64)comet->cpu.registers[ins->rs1] % (i64)comet->cpu.registers[ins->rs2];
        break;
    case 0x2d: // remi
        if (ins->imm == 0)
            read_u64(comet->ic.ivt_base_address + 8*int_divide_by_zero, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = (i64)comet->cpu.registers[ins->rs1] % (i64)ins->imm;
        break;
    case 0x2e: // modr
        if (comet->cpu.registers[ins->rs2] == 0)
            read_u64(comet->ic.ivt_base_address + 8*int_divide_by_zero, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = ((i64)comet->cpu.registers[ins->rs1] % (i64)comet->cpu.registers[ins->rs2]) * sign((i64)comet->cpu.registers[ins->rs2]);
        break;
    case 0x2f: // modi
        if (ins->imm == 0)
            read_u64(comet->ic.ivt_base_address + 8*int_divide_by_zero, &comet->cpu.registers[r_pc]);
        else
            comet->cpu.registers[ins->rde] = ((i64)comet->cpu.registers[ins->rs1] % (i64)ins->imm) * sign((i64)ins->imm);
        break;

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
        printf("Aphelion Extension P \"Posit Operations\" is not currently supported by comet.\n");
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