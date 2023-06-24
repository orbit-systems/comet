package comet

import "core:fmt"

ins_fmt :: enum {
    R, M, F, J, B,
}

instruction_info :: struct {
    opcode : u8,
    func   : u8,
    rde    : u8,
    rs1    : u8,
    rs2    : u8,
    imm    : u64,
}

raw_decode :: proc(ins: u32) -> (ins_dec: instruction_info) {

    ins_dec.opcode = cast(u8) (ins & 0xFF)
    current_fmt := ins_formats[ins_dec.opcode]

    switch ins_formats[ins_dec.opcode] {
    case ins_fmt.R:
        ins_dec.rde  = cast(u8)  (ins >> 28 & 0xF)
        ins_dec.rs1  = cast(u8)  (ins >> 24 & 0xF)
        ins_dec.rs2  = cast(u8)  (ins >> 20 & 0xF)
        ins_dec.imm  = cast(u64) (ins >> 8  & 0xFFF)
    case ins_fmt.M:
        ins_dec.rde  = cast(u8)  (ins >> 28 & 0xF)
        ins_dec.rs1  = cast(u8)  (ins >> 24 & 0xF)
        ins_dec.imm  = cast(u64) (ins >> 8  & 0xFFFF)
    case ins_fmt.F:
        ins_dec.rde  = cast(u8)  (ins >> 28 & 0xF)
        ins_dec.func = cast(u8)  (ins >> 24 & 0xF)
        ins_dec.imm  = cast(u64) (ins >> 8  & 0xFFFF)
    case ins_fmt.J:
        ins_dec.rde  = cast(u8)  (ins >> 28 & 0xF)
        ins_dec.imm  = cast(u64) (ins >> 8  & 0xFFFFF)
    case ins_fmt.B:
        ins_dec.func = cast(u8)  (ins >> 28 & 0xF)
        ins_dec.imm  = cast(u64) (ins >> 8  & 0xFFFFF)
    }
    
    return
}

// lmao
sign_extend_to_u64 :: proc(val:u64, bitsize: u8) -> u64 {
    return u64(i64(val << (64-bitsize)) >> (64-bitsize))
}

// for debug output
print_asm :: proc(ins: instruction_info) {
    

    if !([2]u8{ins.opcode, ins.func} in ins_names) {
        set_style(ANSI.Bold)
        set_style(ANSI.FG_Red)
        fmt.print("INVALID\n")
        set_style(ANSI.Reset)
        return
    }

    name := ins_names[[2]u8{ins.opcode, ins.func}]

    set_style(ANSI.Bold)
    fmt.print(name)
    set_style(ANSI.Reset)
    delete(name)

    for x in ins_op_to_field[{ins.opcode, ins.func}] {
        set_style(ANSI.FG_Blue)
        switch x {
        case iff.RDE:
            fmt.printf(" %s", register_names(ins.rde))
        case iff.RS1:
            fmt.printf(" %s", register_names(ins.rs1))
        case iff.RS2:
            fmt.printf(" %s", register_names(ins.rs2))
        case iff.IMM:
            set_style(ANSI.Reset)
            switch ins.opcode {
            case 0x63..=0x65:
                fmt.printf(" 0x%x", i64(sign_extend_to_u64(ins.imm, 20))*4)
            case:
                fmt.printf(" 0x%x", i64(sign_extend_to_u64(ins.imm, 16)))
            }
        }
    }
    set_style(ANSI.Reset)
    fmt.print("\n")

    return
}

print_registers :: proc(cpu: ^aphelion_cpu_state) {
    using register_names
    set_style(ANSI.Bold); fmt.print("\tpc: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[pc])
    set_style(ANSI.Bold); fmt.print("\tst: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[st])
    set_style(ANSI.Bold); fmt.print("\tsp: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[sp])
    set_style(ANSI.Bold); fmt.print("\tfp: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[fp])
    fmt.print("\n")
    set_style(ANSI.Bold); fmt.print("\tra: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[ra])
    set_style(ANSI.Bold); fmt.print("\trb: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rb])
    set_style(ANSI.Bold); fmt.print("\trc: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rc])
    set_style(ANSI.Bold); fmt.print("\trd: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rd])
    fmt.print("\n")
    set_style(ANSI.Bold); fmt.print("\tre: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[re])
    set_style(ANSI.Bold); fmt.print("\trf: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rf])
    set_style(ANSI.Bold); fmt.print("\trg: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rg])
    set_style(ANSI.Bold); fmt.print("\trh: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rh])
    fmt.print("\n")
    set_style(ANSI.Bold); fmt.print("\tri: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[ri])
    set_style(ANSI.Bold); fmt.print("\trj: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rj])
    set_style(ANSI.Bold); fmt.print("\trk: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rk])
    fmt.print("\n")
}

ins_formats := map[u8]ins_fmt{
    0x0A = ins_fmt.B,
    0x10 = ins_fmt.B,
    0x11 = ins_fmt.M,
    0x12 = ins_fmt.B,

    0x20 = ins_fmt.F,
    0x20 = ins_fmt.F,
    0x20 = ins_fmt.F,
    0x20 = ins_fmt.F,
    0x20 = ins_fmt.F,
    0x20 = ins_fmt.F,
    0x20 = ins_fmt.F,
    0x20 = ins_fmt.F,
    0x21 = ins_fmt.M,
    0x22 = ins_fmt.M,
    0x23 = ins_fmt.M,
    0x24 = ins_fmt.M,
    0x25 = ins_fmt.M,
    0x26 = ins_fmt.M,
    0x26 = ins_fmt.M,

    0x30 = ins_fmt.R,
    0x31 = ins_fmt.M,
    0x32 = ins_fmt.R,
    0x33 = ins_fmt.M,
    0x34 = ins_fmt.R,
    0x35 = ins_fmt.M,
    0x36 = ins_fmt.R,
    0x37 = ins_fmt.M,
    0x38 = ins_fmt.R,
    0x39 = ins_fmt.M,
    0x3a = ins_fmt.R,
    0x3b = ins_fmt.M,

    0x40 = ins_fmt.R,
    0x41 = ins_fmt.M,
    0x42 = ins_fmt.R,
    0x43 = ins_fmt.M,
    0x44 = ins_fmt.R,
    0x45 = ins_fmt.M,
    0x46 = ins_fmt.R,
    0x47 = ins_fmt.M,
    0x48 = ins_fmt.R,
    0x49 = ins_fmt.M,
    0x4a = ins_fmt.R,
    0x4b = ins_fmt.M,
    0x4c = ins_fmt.R,
    0x4d = ins_fmt.M,

    0x50 = ins_fmt.F,
    0x50 = ins_fmt.F,
    0x50 = ins_fmt.F,
    0x50 = ins_fmt.F,
    0x50 = ins_fmt.F,
    0x50 = ins_fmt.F,
    0x50 = ins_fmt.F,
    0x50 = ins_fmt.F,

    0x60 = ins_fmt.M,
    0x61 = ins_fmt.M,
    0x62 = ins_fmt.F,
    0x62 = ins_fmt.F,
    0x64 = ins_fmt.J,
    0x65 = ins_fmt.J,

    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
    0x63 = ins_fmt.B,
}

ins_names := map[[2]u8]string{
    {0x0A, 0}   = "nop"  ,
    {0x10, 0}   = "int"  ,
    {0x11, 0}   = "inv"  ,
    {0x12, 0}   = "usr"  ,

    {0x20, 0}   = "lli"  ,
    {0x20, 1}   = "llis" ,
    {0x20, 2}   = "lui"  ,
    {0x20, 3}   = "luis" ,
    {0x20, 4}   = "lti"  ,
    {0x20, 5}   = "ltis" ,
    {0x20, 6}   = "ltui" ,
    {0x20, 7}   = "ltuis",
    {0x21, 0}   = "ld"   ,
    {0x22, 0}   = "lbs"  ,
    {0x23, 0}   = "lb"   ,
    {0x24, 0}   = "st"   ,
    {0x25, 0}   = "stb"  ,
    {0x26, 0}   = "swp"  ,
    {0x27, 0}   = "mov"  ,

    {0x30, 0}   = "addr" ,
    {0x31, 0}   = "addi" ,
    {0x32, 0}   = "adcr" ,
    {0x33, 0}   = "adci" ,
    {0x34, 0}   = "subr" ,
    {0x35, 0}   = "subi" ,
    {0x36, 0}   = "sbbr" ,
    {0x37, 0}   = "sbbi" ,
    {0x38, 0}   = "mulr" ,
    {0x39, 0}   = "muli" ,
    {0x3a, 0}   = "divr" ,
    {0x3b, 0}   = "divi" ,

    {0x40, 0}   = "andr" ,
    {0x41, 0}   = "andi" ,
    {0x42, 0}   = "orr"  ,
    {0x43, 0}   = "ori"  ,
    {0x44, 0}   = "norr" ,
    {0x45, 0}   = "nori" ,
    {0x46, 0}   = "xorr" ,
    {0x47, 0}   = "xori" ,
    {0x48, 0}   = "shlr" ,
    {0x49, 0}   = "shli" ,
    {0x4a, 0}   = "asrr" ,
    {0x4b, 0}   = "asri" ,
    {0x4c, 0}   = "lsrr" ,
    {0x4d, 0}   = "lsri" ,

    {0x50, 0}   = "push" ,
    {0x50, 1}   = "pushi",
    {0x50, 2}   = "pushz",
    {0x50, 3}   = "pushc",
    {0x50, 4}   = "pop"  ,
    {0x50, 5}   = "enter",
    {0x50, 6}   = "leave",
    {0x50, 7}   = "reloc",

    {0x60, 0}   = "ljal" ,
    {0x61, 0}   = "ljalr",
    {0x62, 0}   = "ret"  ,
    {0x62, 1}   = "retr" ,
    {0x64, 0}   = "jal"  ,
    {0x65, 0}   = "jalr" ,

    {0x63, 0x0} = "bra"  ,
    {0x63, 0x1} = "beq"  ,
    {0x63, 0x2} = "bez"  ,
    {0x63, 0x3} = "blt"  ,
    {0x63, 0x4} = "ble"  ,
    {0x63, 0x5} = "bltu" ,
    {0x63, 0x6} = "bleu" ,
    {0x63, 0x7} = "bpe"  ,
    {0x63, 0x9} = "bne"  ,
    {0x63, 0xa} = "bnz"  ,
    {0x63, 0xb} = "bge"  ,
    {0x63, 0xc} = "bgt"  ,
    {0x63, 0xd} = "bgeu" ,
    {0x63, 0xe} = "bgtu" ,
    {0x63, 0xf} = "bpd"  ,
}

ins_fields :: enum {
    RDE, RS1, RS2, IMM,
}

iff :: ins_fields
ins_op_to_field := map[[2]u8][]iff{
    // op, func
     {0x0A, 0}   = []iff{},
     {0x10, 0}   = []iff{iff.IMM},
     {0x11, 0}   = []iff{},
     {0x12, 0}   = []iff{},

     {0x20, 0}   = []iff{iff.RDE, iff.IMM},
     {0x20, 1}   = []iff{iff.RDE, iff.IMM},
     {0x20, 2}   = []iff{iff.RDE, iff.IMM},
     {0x20, 3}   = []iff{iff.RDE, iff.IMM},
     {0x20, 4}   = []iff{iff.RDE, iff.IMM},
     {0x20, 5}   = []iff{iff.RDE, iff.IMM},
     {0x20, 6}   = []iff{iff.RDE, iff.IMM},
     {0x20, 7}   = []iff{iff.RDE, iff.IMM},
     {0x21, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x22, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x23, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x24, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x25, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x26, 0}   = []iff{iff.RDE, iff.RS1},
     {0x26, 0}   = []iff{iff.RDE, iff.RS1},

     {0x30, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x31, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x32, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x33, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x34, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x35, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x36, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x37, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x38, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x39, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x3a, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x3b, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},

     {0x40, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x41, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x42, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x43, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x44, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x45, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x46, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x47, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x48, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x49, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x4a, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x4b, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},
     {0x4c, 0}   = []iff{iff.RDE, iff.RS1, iff.RS2},
     {0x4d, 0}   = []iff{iff.RDE, iff.RS1, iff.IMM},

     {0x50, 0}   = []iff{iff.RS1},
     {0x50, 1}   = []iff{iff.IMM},
     {0x50, 2}   = []iff{iff.IMM},
     {0x50, 3}   = []iff{iff.IMM},
     {0x50, 4}   = []iff{iff.RDE},
     {0x50, 5}   = []iff{},
     {0x50, 6}   = []iff{},
     {0x50, 7}   = []iff{iff.RS1, iff.IMM},

     {0x60, 0}   = []iff{iff.RS1, iff.IMM},
     {0x61, 0}   = []iff{iff.RS1, iff.IMM, iff.RDE},
     {0x62, 0}   = []iff{},
     {0x62, 1}   = []iff{iff.RDE},
     {0x64, 0}   = []iff{iff.IMM},
     {0x65, 0}   = []iff{iff.IMM, iff.RDE},

     {0x63, 0x0} = []iff{iff.IMM},
     {0x63, 0x1} = []iff{iff.IMM},
     {0x63, 0x2} = []iff{iff.IMM},
     {0x63, 0x3} = []iff{iff.IMM},
     {0x63, 0x4} = []iff{iff.IMM},
     {0x63, 0x5} = []iff{iff.IMM},
     {0x63, 0x6} = []iff{iff.IMM},
     {0x63, 0x7} = []iff{iff.IMM},
     {0x63, 0x9} = []iff{iff.IMM},
     {0x63, 0xa} = []iff{iff.IMM},
     {0x63, 0xb} = []iff{iff.IMM},
     {0x63, 0xc} = []iff{iff.IMM},
     {0x63, 0xd} = []iff{iff.IMM},
     {0x63, 0xe} = []iff{iff.IMM},
     {0x63, 0xf} = []iff{iff.IMM},
}