#include "comet.h"
#include "cpu.h"
#include "dev.h"

void run() {
    
    // load instruction
    mmu_response res = read_instruction(regval(r_ip), &current_instr);
    if (res != res_success) {
        push_interrupt_from_MMU(res);
        return; // restart
    }

    regval(r_ip) += 4;

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
    }

    if (comet.ioc.out_pin) {
        dev_receive(); // run corresponding output handler
    }

    comet.cpu.registers[r_rz] = 0;

    if (regval(r_sp) > regval(r_fp)) {
        push_interrupt(int_stack_underflow);
    }

    TODO("work lmfao");
}

void push_stack(u64 data) {
    regval(r_sp) -= 8;
    push_interrupt_from_MMU(write_u64(regval(r_sp), data));
}

void pop_stack(u64* val) {
    mmu_response res = read_u64(regval(r_sp), val);
    if (res != res_success) {
        push_interrupt_from_MMU(res);
    } else {
        regval(r_sp) += 8;
    }
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
