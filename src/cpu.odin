package comet

exec_instruction :: proc(cpu: ^aphelion_cpu_state, ins: ins_info) {
    using register_names

    prev_pc := cpu.registers[pc]

    switch ins.opcode {
    case 0x0A: // nop
    case 0x10: // int
        cpu.registers[pc] = read_u64((8 * ins.imm) - 1)
    case 0x11: // inv
        cpu.registers[pc] = read_u64(0x27)
    case 0x12: // usr
        cpu.registers[st] |= cpu.registers[st] & 0xA
    case 0x20: // li family

        switch ins.func {
        case 0: // lli
            cpu.registers[ins.rde] &= 0xFFFFFFFFFFFF0000
            cpu.registers[ins.rde] |= ins.imm
        case 1: // llis
            cpu.registers[ins.rde] =  ins.imm
        case 2: // lui
            cpu.registers[ins.rde] &= 0xFFFFFFFF0000FFFF
            cpu.registers[ins.rde] |= ins.imm << 16
        case 3: // luis
            cpu.registers[ins.rde] =  ins.imm << 16
        case 4: // lti
            cpu.registers[ins.rde] &= 0xFFFF0000FFFFFFFF
            cpu.registers[ins.rde] |= ins.imm << 32
        case 5: // ltis
            cpu.registers[ins.rde] =  ins.imm << 32
        case 6: // ltui
            cpu.registers[ins.rde] &= 0x0000FFFFFFFFFFFF
            cpu.registers[ins.rde] |= ins.imm << 48
        case 7: // ltuis
            cpu.registers[ins.rde] =  ins.imm << 48
        }

    case 0x21: // ld
        cpu.registers[ins.rde] = read_u64(cpu.registers[ins.rs1] + ins.imm)
    case 0x22: // lbs
        cpu.registers[ins.rde] = u64(read_u8(cpu.registers[ins.rs1] + ins.imm))
    case 0x23: // lb
        cpu.registers[ins.rde] &= 0xFFFFFFFFFFFFFF00
        cpu.registers[ins.rde] |= u64(read_u8(cpu.registers[ins.rs1] + ins.imm))
    
    case 0x24: // st
        write_u64(cpu.registers[ins.rs2] + ins.imm, cpu.registers[ins.rs1])
    case 0x25: // stb
        write_u8(cpu.registers[ins.rs2] + ins.imm, u8(cpu.registers[ins.rs1]))
    case 0x26: // swp
        temp := cpu.registers[ins.rs1]
        cpu.registers[ins.rs1] = cpu.registers[ins.rs2]
        cpu.registers[ins.rs2] = cpu.registers[ins.rs1]
    case 0x27: // mov
        cpu.registers[ins.rde] = cpu.registers[ins.rs1]

    case 0x30: // addr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] + cpu.registers[ins.rs2]
        set_flags_arithmetic_reg(cpu, ins)
    case 0x31: // addi
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] + sign_extend_to_u64(ins.imm, 16)
        set_flags_arithmetic_imm(cpu, ins)
    case 0x32: // adcr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] + cpu.registers[ins.rs2] + u64(get_st_flag(cpu, st_flag.carry))
        set_flags_arithmetic_reg(cpu, ins)
    case 0x33: // adci
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] + sign_extend_to_u64(ins.imm, 16) + u64(get_st_flag(cpu, st_flag.carry))
        set_flags_arithmetic_imm(cpu, ins)
    case 0x34: // subr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] - cpu.registers[ins.rs2]
        set_flags_arithmetic_reg(cpu, ins)
    case 0x35: // subi
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] - sign_extend_to_u64(ins.imm, 16)
        set_flags_arithmetic_imm(cpu, ins)
    case 0x36: // sbbr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] - cpu.registers[ins.rs2] - u64(get_st_flag(cpu, st_flag.borrow))
        set_flags_arithmetic_reg(cpu, ins)
    case 0x37: // sbbi
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] - sign_extend_to_u64(ins.imm, 16) - u64(get_st_flag(cpu, st_flag.borrow))
        set_flags_arithmetic_imm(cpu, ins)
    case 0x38: // mulr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] * cpu.registers[ins.rs2]
        set_flags_arithmetic_reg(cpu, ins)
    case 0x39: // muli
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] * sign_extend_to_u64(ins.imm, 16)
        set_flags_arithmetic_imm(cpu, ins)
    case 0x3a: // divr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] / cpu.registers[ins.rs2]
        set_flags_arithmetic_reg(cpu, ins)
    case 0x3b: // divi
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] / sign_extend_to_u64(ins.imm, 16)
        set_flags_arithmetic_imm(cpu, ins)
    
    case 0x40: // andr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] & cpu.registers[ins.rs2]
        set_flags_logical_reg(cpu, ins)
    case 0x41: // andi
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] & ins.imm
        set_flags_logical_imm(cpu, ins)
    case 0x42: // orr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] | cpu.registers[ins.rs2]
        set_flags_logical_reg(cpu, ins)
    case 0x43: // ori
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] | ins.imm
        set_flags_logical_imm(cpu, ins)
    case 0x44: // norr
        cpu.registers[ins.rde] = ~(cpu.registers[ins.rs1] | cpu.registers[ins.rs2])
        set_flags_logical_reg(cpu, ins)
    case 0x45: // nori
        cpu.registers[ins.rde] = ~(cpu.registers[ins.rs1] | ins.imm)
        set_flags_logical_imm(cpu, ins)
    case 0x46: // xorr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] ~ cpu.registers[ins.rs2]
        set_flags_logical_reg(cpu, ins)
    case 0x47: // xori
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] ~ ins.imm
        set_flags_logical_imm(cpu, ins)
    case 0x48: // shlr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] << cpu.registers[ins.rs2]
        set_flags_logical_reg(cpu, ins)
    case 0x49: // shli
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] << ins.imm
        set_flags_logical_imm(cpu, ins)
    case 0x4a: // asrr
        cpu.registers[ins.rde] = u64(i64(cpu.registers[ins.rs1]) >> cpu.registers[ins.rs2])
        set_flags_logical_reg(cpu, ins)
    case 0x4b: // asri
        cpu.registers[ins.rde] = u64(i64(cpu.registers[ins.rs1]) >> ins.imm)
        set_flags_logical_imm(cpu, ins)
    case 0x4c: // lsrr
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] >> cpu.registers[ins.rs2]
        set_flags_logical_reg(cpu, ins)
    case 0x4d: // lsri
        cpu.registers[ins.rde] = cpu.registers[ins.rs1] >> ins.imm
        set_flags_logical_imm(cpu, ins)
    
    case 0x50: // stack controls
        switch ins.func {
        case 0: // push

        }    

    case 0x63: // b(cc)
        switch ins.func {
        case 0: // bra
            cpu.registers[pc] += sign_extend_to_u64(ins.imm, 20)*4
            
        }
    case:
        die("invalid instruction\n")

    }

    cpu_state.increment_next = (prev_pc == cpu.registers[pc])

}


aphelion_cpu_state :: struct {
    registers       : [16]u64,
    running         : bool,
    cycle           : u64,
    increment_next  : bool,
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
}

set_flags_arithmetic_reg :: proc(cpu: ^aphelion_cpu_state, ins: ins_info) {
    set_st_flag(cpu, st_flag.sign,   cpu.registers[ins.rde] < 0)
    set_st_flag(cpu, st_flag.zero,   cpu.registers[ins.rde] == 0)
    set_st_flag(cpu, st_flag.parity, cpu.registers[ins.rde] % 2 == 0)
    set_st_flag(cpu, st_flag.carry,  cpu.registers[ins.rde] < cpu.registers[ins.rs1] || cpu.registers[ins.rde] < cpu.registers[ins.rs2])
    set_st_flag(cpu, st_flag.borrow, cpu.registers[ins.rde] > cpu.registers[ins.rs1] || cpu.registers[ins.rde] > cpu.registers[ins.rs2])

    set_st_flag(cpu, st_flag.equal,   cpu.registers[ins.rs1] == cpu.registers[ins.rs2])
    set_st_flag(cpu, st_flag.greater, i64(cpu.registers[ins.rs1]) > i64(cpu.registers[ins.rs2]))
    set_st_flag(cpu, st_flag.less,    i64(cpu.registers[ins.rs1]) < i64(cpu.registers[ins.rs2]))
    set_st_flag(cpu, st_flag.greater, cpu.registers[ins.rs1] > cpu.registers[ins.rs2])
    set_st_flag(cpu, st_flag.less,    cpu.registers[ins.rs1] < cpu.registers[ins.rs2])
}

set_flags_arithmetic_imm :: proc(cpu: ^aphelion_cpu_state, ins: ins_info) {
    set_st_flag(cpu, st_flag.sign,   cpu.registers[ins.rde] < 0)
    set_st_flag(cpu, st_flag.zero,   cpu.registers[ins.rde] == 0)
    set_st_flag(cpu, st_flag.parity, cpu.registers[ins.rde] % 2 == 0)
    set_st_flag(cpu, st_flag.carry,  cpu.registers[ins.rde] < cpu.registers[ins.imm] || cpu.registers[ins.rde] < cpu.registers[ins.imm])
    set_st_flag(cpu, st_flag.borrow, cpu.registers[ins.rde] > cpu.registers[ins.imm] || cpu.registers[ins.rde] > cpu.registers[ins.imm])

    set_st_flag(cpu, st_flag.equal,   cpu.registers[ins.rs1] == cpu.registers[ins.imm])
    set_st_flag(cpu, st_flag.greater, i64(cpu.registers[ins.rs1]) > i64(cpu.registers[ins.imm]))
    set_st_flag(cpu, st_flag.less,    i64(cpu.registers[ins.rs1]) < i64(cpu.registers[ins.imm]))
    set_st_flag(cpu, st_flag.greater, cpu.registers[ins.rs1] > cpu.registers[ins.imm])
    set_st_flag(cpu, st_flag.less,    cpu.registers[ins.rs1] < cpu.registers[ins.imm])
}

set_flags_logical_reg :: proc(cpu: ^aphelion_cpu_state, ins: ins_info) {
    set_st_flag(cpu, st_flag.sign,   cpu.registers[ins.rde] < 0)
    set_st_flag(cpu, st_flag.zero,   cpu.registers[ins.rde] == 0)
    set_st_flag(cpu, st_flag.parity, cpu.registers[ins.rde] % 2 == 0)

    set_st_flag(cpu, st_flag.equal,   cpu.registers[ins.rs1] == cpu.registers[ins.rs2])
    set_st_flag(cpu, st_flag.greater, i64(cpu.registers[ins.rs1]) > i64(cpu.registers[ins.rs2]))
    set_st_flag(cpu, st_flag.less,    i64(cpu.registers[ins.rs1]) < i64(cpu.registers[ins.rs2]))
    set_st_flag(cpu, st_flag.greater, cpu.registers[ins.rs1] > cpu.registers[ins.rs2])
    set_st_flag(cpu, st_flag.less,    cpu.registers[ins.rs1] < cpu.registers[ins.rs2])
}

set_flags_logical_imm :: proc(cpu: ^aphelion_cpu_state, ins: ins_info) {
    set_st_flag(cpu, st_flag.sign,   cpu.registers[ins.rde] < 0)
    set_st_flag(cpu, st_flag.zero,   cpu.registers[ins.rde] == 0)
    set_st_flag(cpu, st_flag.parity, cpu.registers[ins.rde] % 2 == 0)

    set_st_flag(cpu, st_flag.equal,   cpu.registers[ins.rs1] == cpu.registers[ins.imm])
    set_st_flag(cpu, st_flag.greater, i64(cpu.registers[ins.rs1]) > i64(cpu.registers[ins.imm]))
    set_st_flag(cpu, st_flag.less,    i64(cpu.registers[ins.rs1]) < i64(cpu.registers[ins.imm]))
    set_st_flag(cpu, st_flag.greater, cpu.registers[ins.rs1] > cpu.registers[ins.imm])
    set_st_flag(cpu, st_flag.less,    cpu.registers[ins.rs1] < cpu.registers[ins.imm])
}

get_st_flag :: proc{get_st_flag_fl, get_st_flag_u8}
set_st_flag :: proc{set_st_flag_fl, set_st_flag_u8}

get_st_flag_fl :: proc(cpu: ^aphelion_cpu_state, bit: st_flag) -> bool {
    return (cpu.registers[register_names.st] & (1 << u8(bit))) == 1
}

get_st_flag_u8 :: proc(cpu: ^aphelion_cpu_state, bit: u8) -> bool {
    return (cpu.registers[register_names.st] & (1 << bit)) == 1
}

set_st_flag_fl :: proc(cpu: ^aphelion_cpu_state, bit: st_flag, value: bool) {
    cpu.registers[register_names.st] &= ~u64(1 << u8(bit))
    cpu.registers[register_names.st] |= u64(value) << u8(bit)
}

set_st_flag_u8 :: proc(cpu: ^aphelion_cpu_state, bit: u8, value: bool) {
    cpu.registers[register_names.st] &= ~u64(1 << bit)
    cpu.registers[register_names.st] |= u64(value) << bit
}