#include "comet.h"
#include "cpu.h"
#include "dev.h"

void forceinline push_stack(u64 data) {
    regval(r_sp) -= 8;
    push_interrupt_from_MMU(write_u64(regval(r_sp), data));
}

void forceinline pop_stack(u64* val) {
    mmu_response res = read_u64(regval(r_sp), val);
    if (res != res_success) {
        push_interrupt_from_MMU(res);
    } else {
        regval(r_sp) += 8;
    }
}

void run() {
    comet.cpu.cycle++;

    // printf("[at 0x%016X %02x]\n", regval(r_ip), current_instr.opcode);

    // load instruction
    mmu_response res = read_instruction(regval(r_ip), &current_instr);
    if (res != res_success) {
        push_interrupt_from_MMU(res);
        return;
    }

    regval(r_ip) += 4;

    // printf("ip %08X instr %08X \n", regval(r_ip), current_instr);
    instruction ci = current_instr;
    switch (ci.opcode) {
    case 0x01: { // system control
        switch(ci.F.func) {
        case 0x00: // int
            push_interrupt((u8)ci.F.imm);
            break;
        case 0x01: // iret
            if (get_flag(flag_mode) == mode_user) {
                push_interrupt(int_invalid_instruction);
                break;
            }
            return_interrupt();
            break;
        case 0x02: // ires
            if (get_flag(flag_mode) == mode_user) {
                push_interrupt(int_invalid_instruction);
                break;
            }
            resolve_interrupt();
            break;
        case 0x03: // usr
            if (get_flag(flag_mode) == mode_user) {
                push_interrupt(int_invalid_instruction);
                break;
            }
            set_flag(flag_mode, mode_user);
            comet.cpu.registers[r_ip] = regval(ci.F.rde);
            break;
        }
        } break;

    case 0x02: { // outr
        if (get_flag(flag_mode) == mode_user) {
            push_interrupt(int_invalid_instruction);
            break;
        }
        send_out((u16)regval(ci.M.rde), regval(ci.M.rs1));
        } break;
    case 0x03: { // outi
        if (get_flag(flag_mode) == mode_user) {
            push_interrupt(int_invalid_instruction);
            break;
        }
        send_out((u16)ci.M.imm, regval(ci.M.rs1));
        } break;
    case 0x04: { // inr
        if (get_flag(flag_mode) == mode_user) {
            push_interrupt(int_invalid_instruction);
            break;
        }
        regval(ci.M.rde) = port_data((u16)regval(ci.M.rs1));
        } break;
    case 0x05: { // ini
        if (get_flag(flag_mode) == mode_user) {
            push_interrupt(int_invalid_instruction);
            break;
        }
        regval(ci.M.rde) = port_data((u16)ci.M.imm);
        } break;

    case 0x06: { // jal
        push_stack(regval(r_ip));
        regval(r_ip) = regval(ci.M.rs1) + (u64)(4 * (i64)sign_extend(ci.M.imm, 16));
        } break;
    case 0x07: { // jalr
        regval(ci.M.rde) = regval(r_ip);
        regval(r_ip) = regval(ci.M.rs1) + (u64)(4 * (i64)sign_extend(ci.M.imm, 16));
        } break;
    case 0x08: { // ret
        pop_stack(&regval(r_ip));
        } break;
    case 0x09: { // retr
        regval(r_ip) = regval(ci.M.rs1);
        } break;
    case 0x0a: { // branch instructions
        switch (ci.B.imm){
        case 0x0: // bra
            regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0x1: // beq
            if (get_flag(flag_equal))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0x2: // bez
            if (get_flag(flag_zero))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0x3: // blt
            if (get_flag(flag_less))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0x4: // ble
            if (get_flag(flag_less) || get_flag(flag_equal))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0x5: // bltu
            if (get_flag(flag_less_unsigned))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0x6: // bleu
            if (get_flag(flag_less_unsigned) || get_flag(flag_equal))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0x9: // bne
            if (!get_flag(flag_equal))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0xA: // bnz
            if (!get_flag(flag_zero))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0xB: // bge
            if (!get_flag(flag_less))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0xC: // bgt
            if (!(get_flag(flag_less) || get_flag(flag_equal)))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0xD: // bgeu
            if (!get_flag(flag_less_unsigned))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        case 0xE: // bteu
            if (!(get_flag(flag_less_unsigned) || get_flag(flag_equal)))
                regval(r_ip) += (u64)(4 * (i64)sign_extend(ci.M.imm, 20));
            break;
        default:
            push_interrupt(int_invalid_instruction);
        }
    } break;

    case 0x0b: { // push
        push_stack(regval(ci.M.rs1));
        } break;
    case 0x0c: { // pop
        pop_stack(&regval(ci.M.rde));
        } break;
    case 0x0d: { // enter
        push_stack(regval(r_fp));
        regval(r_fp) = regval(r_sp);
        } break;
    case 0x0e: { // leave
        regval(r_sp) = regval(r_fp);
        pop_stack(&regval(r_fp));
        } break;

    case 0x10: { // load immediate
        switch (ci.F.func) {
        case 0: // lli
            *(u16*)&regval(ci.F.rde) = (u16)ci.F.imm;
            break;
        case 1: // llis
            regval(ci.F.rde) = sign_extend(ci.F.imm, 16);
            break;
        case 2: // lui
            *((u16*)&regval(ci.F.rde) + 1) = (u16)ci.F.imm;
            break;
        case 3: // luis
            regval(ci.F.rde) = sign_extend(ci.F.imm, 16) << 16;
            break;
        case 4: // lti
            *((u16*)&regval(ci.F.rde) + 2) = (u16)ci.F.imm;
            break;
        case 5: // ltis
            regval(ci.F.rde) = sign_extend(ci.F.imm, 16) << 32;
            break;
        case 6: // ltui
            *((u16*)&regval(ci.F.rde) + 3) = (u16)ci.F.imm;
            break;
        case 7: // ltuis
            regval(ci.F.rde) = sign_extend(ci.F.imm, 16) << 48;
            break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;

    case 0x11: { // lw
        mmu_response res = read_u64(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            &regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        } break;
    case 0x12: { // lh
        mmu_response res = read_u32(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            (u32*)&regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        } break;
    case 0x13: { // lhs
        mmu_response res = read_u32(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            (u32*)&regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        regval(ci.E.rde) = sign_extend(regval(ci.E.rde), 32);
        } break;
    case 0x14: { // lq
        mmu_response res = read_u16(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            (u16*)&regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        } break;
    case 0x15: { // lqs
        mmu_response res = read_u16(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            (u16*)&regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        regval(ci.E.rde) = sign_extend(regval(ci.E.rde), 16);
        } break;
    case 0x16: { // lb
        mmu_response res = read_u8(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            (u8*)&regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        } break;
    case 0x17: { // lbs
        mmu_response res = read_u8(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            (u8*)&regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        regval(ci.E.rde) = sign_extend(regval(ci.E.rde), 8);
        } break;
    case 0x18: { // sw
        mmu_response res = write_u64(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        } break;
    case 0x19: { // sh
        mmu_response res = write_u32(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            (u32)regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        } break;
    case 0x1a: { // sq
        mmu_response res = write_u16(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            (u16)regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        } break;
    case 0x1b: { // sb
        mmu_response res = write_u8(
            regval(ci.E.rs1) + sign_extend(ci.E.imm, 8) + regval(ci.E.rs2) << ci.E.func,
            (u8)regval(ci.E.rde));
        if (res != res_success) push_interrupt_from_MMU(res);
        } break;
    
    case 0x1e: { // cmpr
        u64 a = regval(ci.M.rde);
        u64 b = regval(ci.M.rs1);

        set_flag(flag_equal,         a == b);
        set_flag(flag_less,          (i64)a < (i64)b);
        set_flag(flag_less_unsigned, a < b);
        set_flag(flag_sign,          (i64) a < 0);
        set_flag(flag_zero,          a == 0);
        } break;
    case 0x1f: { // cmpi
        u64 a;
        u64 b;
        if (ci.F.func == 0) {
            a = regval(ci.F.rde);
            b = sign_extend(ci.F.imm, 16);
        } else if (ci.F.func == 1) {
            b = regval(ci.F.rde);
            a = sign_extend(ci.F.imm, 16);
        } else {
            push_interrupt(int_invalid_instruction);
            break;
        }

        set_flag(flag_equal,         a == b);
        set_flag(flag_less,          (i64)a < (i64)b);
        set_flag(flag_less_unsigned, a < b);
        set_flag(flag_sign,          (i64) a < 0);
        set_flag(flag_zero,          a == 0);
        } break;

    case 0x20: { // addr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        bool u_overflow =  __builtin_uaddl_overflow(a, b, &regval(ci.R.rde));
             u_overflow |= __builtin_uaddl_overflow(regval(ci.R.rde), get_flag(flag_carry_borrow_unsigned), &regval(ci.R.rde));
        bool s_overflow =  __builtin_saddl_overflow((i64)a, (i64)b, (i64*)&regval(ci.R.rde));
             s_overflow |= __builtin_saddl_overflow(regval(ci.R.rde), get_flag(flag_carry_borrow), (i64*)&regval(ci.R.rde));
        set_flag(flag_carry_borrow_unsigned, u_overflow);
        set_flag(flag_carry_borrow, s_overflow);
        } break;
    case 0x21: { // addi
        u64 a = regval(ci.M.rs1);
        u64 b = sign_extend(ci.M.imm, 16);
        bool u_overflow =  __builtin_uaddl_overflow(a, b, &regval(ci.M.rde));
             u_overflow |= __builtin_uaddl_overflow(regval(ci.M.rde), get_flag(flag_carry_borrow_unsigned), &regval(ci.M.rde));
        bool s_overflow =  __builtin_saddl_overflow((i64)a, (i64)b, (i64*)&regval(ci.M.rde));
             s_overflow |= __builtin_saddl_overflow(regval(ci.M.rde), get_flag(flag_carry_borrow), (i64*)&regval(ci.M.rde));
        set_flag(flag_carry_borrow_unsigned, u_overflow);
        set_flag(flag_carry_borrow, s_overflow);
        } break;
    case 0x22: { // subr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        bool u_overflow =  __builtin_usubl_overflow(a, b, &regval(ci.R.rde));
             u_overflow |= __builtin_usubl_overflow(regval(ci.R.rde), get_flag(flag_carry_borrow_unsigned), &regval(ci.R.rde));
        bool s_overflow =  __builtin_ssubl_overflow((i64)a, (i64)b, (i64*)&regval(ci.R.rde));
             s_overflow |= __builtin_ssubl_overflow(regval(ci.R.rde), get_flag(flag_carry_borrow), (i64*)&regval(ci.R.rde));
        set_flag(flag_carry_borrow_unsigned, u_overflow);
        set_flag(flag_carry_borrow, s_overflow);
        } break;
    case 0x23: { // subi
        u64 a = regval(ci.M.rs1);
        u64 b = sign_extend(ci.M.imm, 16);
        bool u_overflow =  __builtin_uaddl_overflow(a, b, &regval(ci.M.rde));
             u_overflow |= __builtin_uaddl_overflow(regval(ci.M.rde), get_flag(flag_carry_borrow_unsigned), &regval(ci.M.rde));
        bool s_overflow =  __builtin_saddl_overflow((i64)a, (i64)b, (i64*)&regval(ci.M.rde));
             s_overflow |= __builtin_saddl_overflow(regval(ci.M.rde), get_flag(flag_carry_borrow), (i64*)&regval(ci.M.rde));
        set_flag(flag_carry_borrow_unsigned, u_overflow);
        set_flag(flag_carry_borrow, s_overflow);
        } break;
    case 0x24: { // imulr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = (i64)a * (i64)b;
        } break;
    case 0x25: { // imuli
        u64 a = regval(ci.M.rs1);
        u64 b = sign_extend(ci.M.imm, 16);
        regval(ci.M.rde) = (i64)a * (i64)b;
        } break;
    case 0x26: { // idivr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = (i64)a / (i64)b;
        } break;
    case 0x27: { // idivi
        if (regval(ci.E.rs2) == 0) {
            push_interrupt(int_divide_by_zero);
            break;
        }
        u64 a = regval(ci.M.rs1);
        u64 b = sign_extend(ci.M.imm, 16);
        regval(ci.M.rde) = (i64)a / (i64)b;
        } break;
    case 0x28: { // umulr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = a * b;
        } break;
    case 0x29: { // umuli
        u64 a = regval(ci.M.rs1);
        u64 b = sign_extend(ci.M.imm, 16);
        regval(ci.M.rde) = a * b;
        } break;
    case 0x2a: { // udivr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = a / b;
        } break;
    case 0x2b: { // udivi
        u64 a = regval(ci.M.rs1);
        u64 b = sign_extend(ci.M.imm, 16);
        regval(ci.M.rde) = a / b;
        } break;
    case 0x2c: { // remr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = (i64)a % (i64)b;
        } break;
    case 0x2d: { // remi
        u64 a = regval(ci.M.rs1);
        u64 b = sign_extend(ci.M.imm, 16);
        regval(ci.M.rde) = (i64)a % (i64)b;
        } break;
    case 0x2e: { // modr
        i64 a = (i64)regval(ci.R.rs1);
        i64 b = (i64)regval(ci.R.rs2);
        if (b == -1) {
            regval(ci.R.rde) = 0;
            break;
        }
        i64 r = a % b;
        if (r < 0) {
            if (b >= 0) r += b;
            else        r -= b;
        }
        regval(ci.R.rde) = r;
        } break;
    case 0x2f: { // modi
        i64 a = (i64)regval(ci.M.rs1);
        i64 b = (i64)sign_extend(ci.M.imm, 16);
        if (b == -1) {
            regval(ci.M.rde) = 0;
            break;
        }
        i64 r = a % b;
        if (r < 0) {
            if (b >= 0) r += b;
            else        r -= b;
        }
        regval(ci.M.rde) = r;
        } break;
    

    case 0x30: { // andr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = a & b;
        } break;
    case 0x31: { // andi
        u64 a = regval(ci.M.rs1);
        u64 b = (u64)(ci.M.imm);
        regval(ci.M.rde) = a & b;
        } break;
    case 0x32: { // orr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = a | b;
        } break;
    case 0x33: { // ori
        u64 a = regval(ci.M.rs1);
        u64 b = (u64)(ci.M.imm);
        regval(ci.M.rde) = a | b;
        } break;
    case 0x34: { // norr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = ~(a | b);
        } break;
    case 0x35: { // nori
        u64 a = regval(ci.M.rs1);
        u64 b = (u64)(ci.M.imm);
        regval(ci.M.rde) = ~(a | b);
        } break;
    case 0x36: { // xorr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = a ^ b;
        } break;
    case 0x37: { // xori
        u64 a = regval(ci.M.rs1);
        u64 b = (u64)(ci.M.imm);
        regval(ci.M.rde) = a ^ b;
        } break;
    case 0x38: { // shlr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = a << b;
        } break;
    case 0x39: { // shli
        u64 a = regval(ci.M.rs1);
        u64 b = (u64)(ci.M.imm);
        regval(ci.M.rde) = a << b;
        } break;
    case 0x3a: { // asrr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = (i64)a >> b;
        } break;
    case 0x3b: { // asri
        u64 a = regval(ci.M.rs1);
        u64 b = (u64)(ci.M.imm);
        regval(ci.M.rde) = (i64)a >> b;
        } break;
    case 0x3c: { // lsrr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = (u64)a >> b;
        } break;
    case 0x3d: { // lsri
        u64 a = regval(ci.M.rs1);
        u64 b = (u64)(ci.M.imm);
        regval(ci.M.rde) = (u64)a >> b;
        } break;
    case 0x3e: { // bitr
        u64 a = regval(ci.R.rs1);
        u64 b = regval(ci.R.rs2);
        regval(ci.R.rde) = (a >> b) & 1ull;
        } break;
    case 0x3f: { // biti
        u64 a = regval(ci.M.rs1);
        u64 b = (u64)(ci.M.imm);
        regval(ci.M.rde) = (a >> b) & 1ull;
        } break;

    /* Extension F- Floating-Point Operations */
    case 0x40: { // fcmp
        switch (ci.E.func) {
        case 0: {
            f16 a = *(f16*)&regval(ci.M.rde);
            f16 b = *(f16*)&regval(ci.M.rs1);
            set_flag(flag_equal,         a == b);
            set_flag(flag_less,          a < b);
            set_flag(flag_less_unsigned, a < b);
            set_flag(flag_sign,          a < 0);
            set_flag(flag_zero,          a == 0);
            } break;
        case 1: {
            f32 a = *(f32*)&regval(ci.M.rde);
            f32 b = *(f32*)&regval(ci.M.rs1);
            set_flag(flag_equal,         a == b);
            set_flag(flag_less,          a < b);
            set_flag(flag_less_unsigned, a < b);
            set_flag(flag_sign,          a < 0);
            set_flag(flag_zero,          a == 0);
            } break;
        case 2: {
            f64 a = *(f64*)&regval(ci.M.rde);
            f64 b = *(f64*)&regval(ci.M.rs1);
            set_flag(flag_equal,         a == b);
            set_flag(flag_less,          a < b);
            set_flag(flag_less_unsigned, a < b);
            set_flag(flag_sign,          a < 0);
            set_flag(flag_zero,          a == 0);
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x41: { // fto
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = (f16)regval(ci.E.rs1);
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = (f32)regval(ci.E.rs1);
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = (f64)regval(ci.E.rs1);
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x42: { // ffrom
        switch (ci.E.func) {
        case 0: {
            regval(ci.E.rde) = (u64)(i64)*(f16*)&regval(ci.E.rs1);
            } break;
        case 1: {
            regval(ci.E.rde) = (u64)(i64)*(f32*)&regval(ci.E.rs1);
            } break;
        case 2: {
            regval(ci.E.rde) = (u64)(i64)*(f64*)&regval(ci.E.rs1);
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x43: { // fneg
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = - *(f16*)&regval(ci.E.rs1);
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = - *(f32*)&regval(ci.E.rs1);
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = - *(f64*)&regval(ci.E.rs1);
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x44: { // fabs
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = *(f16*)&regval(ci.E.rs1) >= 0 ? *(f16*)&regval(ci.E.rs1) : -*(f16*)&regval(ci.E.rs1);
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = fabsf(*(f32*)&regval(ci.E.rs1));
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = fabs(*(f64*)&regval(ci.E.rs1));
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x45: { // fadd
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = *(f16*)&regval(ci.E.rs1) + *(f16*)&regval(ci.E.rs2);
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = *(f32*)&regval(ci.E.rs1) + *(f32*)&regval(ci.E.rs2);
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = *(f64*)&regval(ci.E.rs1) + *(f64*)&regval(ci.E.rs2);
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x46: { // fsub
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = *(f16*)&regval(ci.E.rs1) - *(f16*)&regval(ci.E.rs2);
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = *(f32*)&regval(ci.E.rs1) - *(f32*)&regval(ci.E.rs2);
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = *(f64*)&regval(ci.E.rs1) - *(f64*)&regval(ci.E.rs2);
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x47: { // fmul
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = *(f16*)&regval(ci.E.rs1) * *(f16*)&regval(ci.E.rs2);
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = *(f32*)&regval(ci.E.rs1) * *(f32*)&regval(ci.E.rs2);
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = *(f64*)&regval(ci.E.rs1) * *(f64*)&regval(ci.E.rs2);
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x48: { // fdiv
        if (*(f16*)&regval(ci.E.rs2) == 0) {
            push_interrupt(int_divide_by_zero);
            break;
        }
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = *(f16*)&regval(ci.E.rs1) / *(f16*)&regval(ci.E.rs2);
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = *(f32*)&regval(ci.E.rs1) / *(f32*)&regval(ci.E.rs2);
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = *(f64*)&regval(ci.E.rs1) / *(f64*)&regval(ci.E.rs2);
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x49: { // fma
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = *(f16*)&regval(ci.E.rs1) * *(f16*)&regval(ci.E.rs2) + *(f16*)&regval(ci.E.rde);
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = fmaf(*(f32*)&regval(ci.E.rs1), *(f32*)&regval(ci.E.rs2), *(f32*)&regval(ci.E.rde));
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = fma(*(f64*)&regval(ci.E.rs1), *(f64*)&regval(ci.E.rs2), *(f64*)&regval(ci.E.rde));
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x4a: { // fsqrt
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = sqrtf(*(f16*)&regval(ci.E.rs1));
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = sqrtf(*(f32*)&regval(ci.E.rs1));
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = sqrt(*(f64*)&regval(ci.E.rs1));
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x4b: { // fmin
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = *(f16*)&regval(ci.E.rs1) > *(f16*)&regval(ci.E.rs2) ? *(f16*)&regval(ci.E.rs2) : *(f16*)&regval(ci.E.rs1);
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = fminf(*(f32*)&regval(ci.E.rs1), *(f32*)&regval(ci.E.rs2));
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = fmin(*(f64*)&regval(ci.E.rs1), *(f64*)&regval(ci.E.rs2));
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x4c: { // fmax
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = *(f16*)&regval(ci.E.rs1) > *(f16*)&regval(ci.E.rs2) ? *(f16*)&regval(ci.E.rs1) : *(f16*)&regval(ci.E.rs2);
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = fmaxf(*(f32*)&regval(ci.E.rs1), *(f32*)&regval(ci.E.rs2));
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = fmax(*(f64*)&regval(ci.E.rs1), *(f64*)&regval(ci.E.rs2));
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x4d: { // fsat
        switch (ci.E.func) {
        case 0: {
            *(f16*)&regval(ci.E.rde) = ceilf((f32)*(f16*)&regval(ci.E.rs1));
            } break;
        case 1: {
            *(f32*)&regval(ci.E.rde) = ceilf(*(f32*)&regval(ci.E.rs1));
            } break;
        case 2: {
            *(f64*)&regval(ci.E.rde) = ceil(*(f64*)&regval(ci.E.rs1));
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x4e: { // fcnv
        switch (ci.E.func) {
        case 0b0000: *(f16*)&regval(ci.E.rde) = (f16) *(f16*)&regval(ci.E.rs1); break;
        case 0b0001: *(f32*)&regval(ci.E.rde) = (f32) *(f16*)&regval(ci.E.rs1); break;
        case 0b0010: *(f64*)&regval(ci.E.rde) = (f64) *(f16*)&regval(ci.E.rs1); break;
        case 0b0100: *(f16*)&regval(ci.E.rde) = (f16) *(f32*)&regval(ci.E.rs1); break;
        case 0b0101: *(f32*)&regval(ci.E.rde) = (f32) *(f32*)&regval(ci.E.rs1); break;
        case 0b0110: *(f64*)&regval(ci.E.rde) = (f64) *(f32*)&regval(ci.E.rs1); break;
        case 0b1000: *(f16*)&regval(ci.E.rde) = (f16) *(f64*)&regval(ci.E.rs1); break;
        case 0b1001: *(f32*)&regval(ci.E.rde) = (f32) *(f64*)&regval(ci.E.rs1); break;
        case 0b1010: *(f64*)&regval(ci.E.rde) = (f64) *(f64*)&regval(ci.E.rs1); break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    case 0x4f: { // fnan
        switch (ci.E.func) {
        case 0: {
            u16 b = *(u16*)&regval(ci.E.rs1);
            regval(ci.E.rde) = 0;
            if ((b >> 10) ^ 0b011111 == 0) {
                if (b & 0b1111111111) {
                    regval(ci.E.rde) = 1;
                }
            }
            } break;
        case 1: {
            regval(ci.E.rde) = !!__isnanf(*(f32*)&regval(ci.E.rs1));
            } break;
        case 2: {
            regval(ci.E.rde) = !!__isnan(*(f64*)&regval(ci.E.rs1));
            } break;
        default:
            push_interrupt(int_invalid_instruction);
        }
        } break;
    default:
        push_interrupt(int_invalid_instruction);
    }



    if (comet.ioc.out_pin) {
        dev_receive(); // run corresponding output handler
    }

    comet.cpu.registers[r_rz] = 0;

    if (regval(r_sp) > regval(r_fp)) {
        push_interrupt(int_stack_underflow);
    }
}