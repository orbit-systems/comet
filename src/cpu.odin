package comet

import "core:fmt"
import "core:intrinsics"

exec_instruction :: proc(cpu: ^aphelion_cpu_state, ins: instruction_info) {
    using register_names

    prev_pc := cpu.registers[pc]
    pc_modified := false

    switch ins.opcode {
    case 0x0A: // nop
    case 0x10: // int
        cpu.registers[pc] = read(u64, 8 * ins.imm)
        if flag_halt_inv_op && ins.imm == 1 {
            cpu.running = false
            return
        }
    case 0x11: // inv
        cpu.registers[pc] = read(u64, 0x28)
    case 0x12: // usr
        cpu.registers[st] |= cpu.registers[st] & 0xA
    case 0x20: // li family

        switch ins.func {
        case 0: // lli
            cpu.registers[ins.rde] &= 0xFFFFFFFFFFFF0000
            cpu.registers[ins.rde] |= ins.imm
        case 1: // llis
            cpu.registers[ins.rde] =  sign_extend_to_u64(ins.imm,16)
        case 2: // lui
            cpu.registers[ins.rde] &= 0xFFFFFFFF0000FFFF
            cpu.registers[ins.rde] |= ins.imm << 16
        case 3: // luis
            cpu.registers[ins.rde] =  sign_extend_to_u64(ins.imm,16) << 16
        case 4: // lti
            cpu.registers[ins.rde] &= 0xFFFF0000FFFFFFFF
            cpu.registers[ins.rde] |= ins.imm << 32
        case 5: // ltis
            cpu.registers[ins.rde] =  sign_extend_to_u64(ins.imm,16) << 32
        case 6: // ltui
            cpu.registers[ins.rde] &= 0x0000FFFFFFFFFFFF
            cpu.registers[ins.rde] |= ins.imm << 48
        case 7: // ltuis
            cpu.registers[ins.rde] =  sign_extend_to_u64(ins.imm,16) << 48
        }

    case 0x21: // lw
        cpu.registers[ins.rde] = read(u64, cpu.registers[ins.rs1] + sign_extend_to_u64(ins.imm, 16))
    case 0x22: // lbs
        cpu.registers[ins.rde] = u64(read(u8, cpu.registers[ins.rs1] + ins.imm))
    case 0x23: // lb
        cpu.registers[ins.rde] &= 0xFFFFFFFFFFFFFF00
        cpu.registers[ins.rde] += u64(read(u8, cpu.registers[ins.rs1] + ins.imm))
    
    case 0x24: // sw
        write(u64, cpu.registers[ins.rs1] + sign_extend_to_u64(ins.imm, 16), cpu.registers[ins.rde])
    case 0x25: // sb
        write(u8, cpu.registers[ins.rs1] + sign_extend_to_u64(ins.imm, 16), u8(cpu.registers[ins.rde]))
    case 0x26: // swp
        cpu.registers[ins.rs1], cpu.registers[ins.rde] = cpu.registers[ins.rde], cpu.registers[ins.rs1]
    case 0x27: // mov
        cpu.registers[ins.rde] = cpu.registers[ins.rs1]
    case 0x28: // cmpr
        set_flags_cmpr(cpu, ins)
    case 0x29: // cmpi
        set_flags_cmpi(cpu, ins)

    case 0x30: // addr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] + cpu.registers[ins.rs2]
        set_flags_addr(cpu, ins)
    case 0x31: // addi
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] + sign_extend_to_u64(ins.imm, 16)
        set_flags_addi(cpu, ins)
    case 0x32: // adcr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] + cpu.registers[ins.rs2] + u64(get_st_flag(cpu, st_flag.carry))
        set_flags_addr(cpu, ins)
    case 0x33: // adci
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] + sign_extend_to_u64(ins.imm, 16) + u64(get_st_flag(cpu, st_flag.carry))
        set_flags_addi(cpu, ins)
    case 0x34: // subr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] - cpu.registers[ins.rs2]
        set_flags_subr(cpu, ins)
    case 0x35: // subi
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] - sign_extend_to_u64(ins.imm, 16)
        set_flags_subi(cpu, ins)
    case 0x36: // sbbr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] - cpu.registers[ins.rs2] - u64(get_st_flag(cpu, st_flag.borrow))
        set_flags_subr(cpu, ins)
    case 0x37: // sbbi
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] - sign_extend_to_u64(ins.imm, 16) - u64(get_st_flag(cpu, st_flag.borrow))
        set_flags_subi(cpu, ins)
    case 0x38: // mulr
        cpu.registers[ins.rde] = u64(i64(cpu.registers[ins.rs1]) * i64(cpu.registers[ins.rs2]))
    case 0x39: // muli
        cpu.registers[ins.rde] = u64(i64(cpu.registers[ins.rs1]) * i64(sign_extend_to_u64(ins.imm, 16)))
    case 0x3a: // divr
        if i64(cpu.registers[ins.rs2]) == 0 {
            cpu.registers[pc] = read(u64, 0) // int 0 - divide by zero
            return
        }
        cpu.registers[ins.rde] = u64(i64(cpu.registers[ins.rs1]) / i64(cpu.registers[ins.rs2]))
    case 0x3b: // divi
        if ins.imm == 0 {
            cpu.registers[pc] = read(u64, 0) // int 0 - divide by zero
            return
        }
        cpu.registers[ins.rde] = u64(i64(cpu.registers[ins.rs1]) / i64(sign_extend_to_u64(ins.imm, 16)))
    
    case 0x40: // andr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] & cpu.registers[ins.rs2]
    case 0x41: // andi
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] & ins.imm
    case 0x42: // orr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] | cpu.registers[ins.rs2]
    case 0x43: // ori
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] | ins.imm
    case 0x44: // norr
        cpu.registers[ins.rde] = ~(cpu.registers[ins.rs1] | cpu.registers[ins.rs2])
        
    case 0x45: // nori
        cpu.registers[ins.rde] = ~(cpu.registers[ins.rs1] | ins.imm)
    case 0x46: // xorr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] ~ cpu.registers[ins.rs2]
    case 0x47: // xori
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] ~ ins.imm
    case 0x48: // shlr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] << cpu.registers[ins.rs2]
    case 0x49: // shli
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] << ins.imm
    case 0x4a: // asrr
        cpu.registers[ins.rde] = u64(i64(cpu.registers[ins.rs1]) >> cpu.registers[ins.rs2])
    case 0x4b: // asri
        cpu.registers[ins.rde] = u64(i64(cpu.registers[ins.rs1]) >> ins.imm)
    case 0x4c: // lsrr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] >> cpu.registers[ins.rs2]
    case 0x4d: // lsri
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] >> ins.imm
    
    case 0x50: // push
        cpu.registers[sp] -= 8
        write(u64, cpu.registers[sp], cpu.registers[ins.rs1])
    case 0x51: // pushi
        cpu.registers[sp] -= 8
        write(u64, cpu.registers[sp], sign_extend_to_u64(ins.imm, 16))
    case 0x52: 
        cpu.registers[sp] -= 8
        write(u64, cpu.registers[sp], ins.imm)
    case 0x53: // pushc
        cpu.registers[sp] -= 2
        write(u16, cpu.registers[sp], u16(ins.imm))
    case 0x54: // pop
        cpu.registers[ins.rde] = read(u64, cpu.registers[sp])
        cpu.registers[sp] += 8
    case 0x55: // enter
        cpu.registers[sp] -= 8
        write(u64, cpu.registers[sp], cpu.registers[fp]) // push fp
        cpu.registers[fp] = cpu.registers[sp]
    case 0x56: // leave
        cpu.registers[sp] = cpu.registers[fp]
        cpu.registers[fp] = read(u64, cpu.registers[sp])
        cpu.registers[sp] += 8
    case 0x57: // reloc
        cpu.registers[sp] = cpu.registers[ins.rs1]
        cpu.registers[fp] = cpu.registers[ins.rs1] - sign_extend_to_u64(ins.imm, 16)

    
    case 0x60: // ljal
        cpu.registers[sp] -= 8
        write(u64, cpu.registers[sp], cpu.registers[pc]+4)
        cpu.registers[pc] = cpu.registers[ins.rs1] + (sign_extend_to_u64(ins.imm, 16)*4)
    case 0x61: // ljalr
        cpu.registers[ins.rs1] = cpu.registers[pc]+4
        cpu.registers[pc] = cpu.registers[ins.rs1] + (sign_extend_to_u64(ins.imm, 16)*4)
    case 0x62: // ret
        cpu.registers[pc] = read(u64, cpu.registers[sp])
        cpu.registers[sp] += 8
    case 0x63: // retr
        cpu.registers[pc] = cpu.registers[ins.rde]
        
    case 0x64: // b(cc)
        // fmt.printf("sign: %t\n", get_st_flag(cpu, 0))
        // fmt.printf("zero: %t\n", get_st_flag(cpu, 1))
        // fmt.printf("parity: %t\n", get_st_flag(cpu, 2))
        // fmt.printf("carry: %t\n", get_st_flag(cpu, 3))
        // fmt.printf("borrow: %t\n", get_st_flag(cpu, 4))
        // fmt.printf("equal: %t\n", get_st_flag(cpu, 5))
        switch ins.func {
        case 0: // bra
            cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
            pc_modified = true
        case 1: // beq
            if get_st_flag(cpu, st_flag.equal) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 2: // bez
            if get_st_flag(cpu, st_flag.zero) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 3: // blt
            if get_st_flag(cpu, st_flag.less) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 4: // ble
            if get_st_flag(cpu, st_flag.less) || get_st_flag(cpu, st_flag.equal) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 5: // bltu
            if get_st_flag(cpu, st_flag.less_unsigned) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 6: // bleu
            if get_st_flag(cpu, st_flag.less_unsigned) || get_st_flag(cpu, st_flag.equal) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 7: // bpe
            if get_st_flag(cpu, st_flag.parity) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 8: // reserved ig?
        case 9: // beq
            if !get_st_flag(cpu, st_flag.equal) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 10: // bnz
            if !get_st_flag(cpu, st_flag.zero) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 11: // bge
            if get_st_flag(cpu, st_flag.greater) || get_st_flag(cpu, st_flag.equal) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 12: // bgt
            if get_st_flag(cpu, st_flag.greater) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 13: // bgeu
            if get_st_flag(cpu, st_flag.greater_unsigned) || get_st_flag(cpu, st_flag.equal) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 14: // bgtu
            if get_st_flag(cpu, st_flag.greater_unsigned) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        case 15: // bpd
            if !get_st_flag(cpu, st_flag.parity) {
                cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
                pc_modified = true
            }
        }
    case 0x65: // jal
        cpu.registers[sp] -= 8
        write(u64, cpu.registers[sp], cpu.registers[pc]+4)
        cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
        pc_modified = true
    case 0x66: // ljalr
        cpu.registers[ins.rde] = cpu.registers[pc]+4
        cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
        pc_modified = true
    case: // trigger invalid opcode interrupt
        cpu.registers[pc] = read(u64, 8)
        if flag_halt_inv_op {
            cpu.running = false
            return
        }

    }

    cpu.registers[rz] = 0
    cpu.increment_next = (prev_pc == cpu.registers[pc]) && !pc_modified

}

do_cpu_cycle :: proc() {
    using register_names
    comet.cpu.cycle += 1
        
    comet.cpu.raw_ins = read(u32, comet.cpu.registers[pc])
    
    comet.cpu.registers[st] &= 0x00000000FFFFFFFF
    comet.cpu.registers[st] |= u64(comet.cpu.raw_ins) << 32

    comet.cpu.ins_info = raw_decode(comet.cpu.raw_ins)

    //actually do the instruction
    exec_instruction(&comet.cpu, comet.cpu.ins_info)

    comet.cpu.registers[pc] += 4 * transmute(u64)(comet.cpu.increment_next)
}

aphelion_cpu_state :: struct {
    registers       : [16]u64,
    running         : bool,
    paused          : bool,
    step            : bool,
    cycle           : u64,
    increment_next  : b64,
    raw_ins         : u32,
    ins_info        : instruction_info,
}

register_names :: enum u8 {
    rz,
    ra, rb, rc, rd,
    re, rf, rg, rh,
    ri, rj, rk,
    pc,
    sp, fp,
    st,
}

st_flag :: enum u8 {
    sign = 0,
    zero,
    parity,
    carry,
    borrow,
    equal,
    greater,
    less,
    greater_unsigned,
    less_unsigned,
    mode,
    carry_unsigned,
    borrow_unsigned,
}

set_flags_addi :: proc(cpu: ^aphelion_cpu_state, ins: instruction_info) {
    set_st_flag(cpu, st_flag.carry,  i64(cpu.registers[ins.rde]) < i64(cpu.registers[ins.rs1]) || i64(cpu.registers[ins.rde]) < i64(ins.imm))
    set_st_flag(cpu, st_flag.carry_unsigned,  cpu.registers[ins.rde] < ins.imm || cpu.registers[ins.rde] < ins.imm)
}

set_flags_addr :: proc(cpu: ^aphelion_cpu_state, ins: instruction_info) {
    set_st_flag(cpu, st_flag.carry,  i64(cpu.registers[ins.rde]) < i64(cpu.registers[ins.rs1]) || i64(cpu.registers[ins.rde]) < i64(cpu.registers[ins.rs2]))
    set_st_flag(cpu, st_flag.carry_unsigned,  cpu.registers[ins.rde] < cpu.registers[ins.rs1] || cpu.registers[ins.rde] < cpu.registers[ins.rs2])
}

set_flags_subi :: proc(cpu: ^aphelion_cpu_state, ins: instruction_info) {
    set_st_flag(cpu, st_flag.borrow, i64(cpu.registers[ins.rde]) > i64(cpu.registers[ins.rs1]) || i64(cpu.registers[ins.rde]) > i64(ins.imm))
    set_st_flag(cpu, st_flag.borrow_unsigned, cpu.registers[ins.rde] > ins.imm || cpu.registers[ins.rde] > ins.imm)
}

set_flags_subr :: proc(cpu: ^aphelion_cpu_state, ins: instruction_info) {
    set_st_flag(cpu, st_flag.borrow, i64(cpu.registers[ins.rde]) > i64(cpu.registers[ins.rs1]) || i64(cpu.registers[ins.rde]) > i64(cpu.registers[ins.rs1]))
    set_st_flag(cpu, st_flag.borrow_unsigned, cpu.registers[ins.rde] > cpu.registers[ins.rs1] || cpu.registers[ins.rde] > cpu.registers[ins.rs1])
}

set_flags_cmpi :: proc(cpu: ^aphelion_cpu_state, ins: instruction_info) {
    set_st_flag(cpu, st_flag.sign,   cpu.registers[ins.rs1] < 0)
    set_st_flag(cpu, st_flag.zero,   cpu.registers[ins.rs1] == 0)
    set_st_flag(cpu, st_flag.parity, intrinsics.count_ones(cpu.registers[ins.rs1]) % 2 == 0)

    set_st_flag(cpu, st_flag.equal,   cpu.registers[ins.rs1] == ins.imm)
    set_st_flag(cpu, st_flag.greater, i64(cpu.registers[ins.rs1]) > i64(ins.imm))
    set_st_flag(cpu, st_flag.less,    i64(cpu.registers[ins.rs1]) < i64(ins.imm))
    set_st_flag(cpu, st_flag.greater_unsigned, cpu.registers[ins.rs1] > ins.imm)
    set_st_flag(cpu, st_flag.less_unsigned,    cpu.registers[ins.rs1] < ins.imm)
}

set_flags_cmpr :: proc(cpu: ^aphelion_cpu_state, ins: instruction_info) {
    set_st_flag(cpu, st_flag.sign,   cpu.registers[ins.rs1] < 0)
    set_st_flag(cpu, st_flag.zero,   cpu.registers[ins.rs1] == 0)
    set_st_flag(cpu, st_flag.parity, intrinsics.count_ones(cpu.registers[ins.rs1]) % 2 == 0)

    set_st_flag(cpu, st_flag.equal,   cpu.registers[ins.rs1] == cpu.registers[ins.rs2])
    set_st_flag(cpu, st_flag.greater, i64(cpu.registers[ins.rs1]) > i64(cpu.registers[ins.rs2]))
    set_st_flag(cpu, st_flag.less,    i64(cpu.registers[ins.rs1]) < i64(cpu.registers[ins.rs2]))
    set_st_flag(cpu, st_flag.greater_unsigned, cpu.registers[ins.rs1] > cpu.registers[ins.rs2])
    set_st_flag(cpu, st_flag.less_unsigned,    cpu.registers[ins.rs1] < cpu.registers[ins.rs2])
}

get_st_flag :: proc{get_st_flag_fl, get_st_flag_u8}
set_st_flag :: proc{set_st_flag_fl, set_st_flag_u8}

get_st_flag_fl :: proc(cpu: ^aphelion_cpu_state, bit: st_flag) -> bool {
    return (cpu.registers[register_names.st] & (1 << u8(bit))) >> u8(bit) == 1
}

get_st_flag_u8 :: proc(cpu: ^aphelion_cpu_state, bit: u8) -> bool {
    return (cpu.registers[register_names.st] & (1 << bit)) >> bit == 1
}

set_st_flag_fl :: proc(cpu: ^aphelion_cpu_state, bit: st_flag, value: bool) {
    cpu.registers[register_names.st] &= ~u64(1 << u8(bit))
    cpu.registers[register_names.st] |= u64(value) << u8(bit)
}

set_st_flag_u8 :: proc(cpu: ^aphelion_cpu_state, bit: u8, value: bool) {
    cpu.registers[register_names.st] &= ~u64(1 << bit)
    cpu.registers[register_names.st] |= u64(value) << bit
}