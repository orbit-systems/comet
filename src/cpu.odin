package comet

exec_instruction :: proc(cpu: ^aphelion_cpu_state, ins: ins_info) {
    using register_names

    switch (ins.opcode) {
        case 0x20: //load immediate family
            switch(ins.func) {
                case 0: //LLI
                    cpu.registers[ins.rde] &= 0xFFFFFFFFFFFF0000
                    cpu.registers[ins.rde] |= cast(u64)ins.imm
            }
        case 0x30: //ADDR
            cpu.registers[ins.rde] = cpu.registers[ins.rs1] + cpu.registers[ins.rs2]
    
        case 0x63: //branching instructions
            switch(ins.func) {
                case 0: //BRA
                    cpu.registers[pc] += se_to_u64(ins.imm, 20)*4
                    cpu.increment_next = false
            }
    }
}

aphelion_cpu_state :: struct {
    registers       : [16]u64,
    running         : bool,
    cycle           : u64,
    increment_next  : bool,
}

register_names :: enum {
    rz,
    ra, rb, rc, rd,
    re, rf, rg, rh,
    ri, rj, rk,
    pc,
    sp, fp,
    st,
}

register_name_strings := map[u8]string {
    0  = "rz",  
    1  = "ra",  
    2  = "rb",  
    3  = "rc",
    4  = "rd",
    5  = "re",
    6  = "rf",
    7  = "rg",
    8  = "rh",
    9  = "ri",
    10 = "rj",
    11 = "rk",
    12 = "pc",
    13 = "sp",
    14 = "fp",
    15 = "st",
}