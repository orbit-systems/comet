package comet

import "core:fmt"
import "core:strings"

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

    switch current_fmt {
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
print_asm :: proc(ins: instruction_info) -> (out: string) {

    if !([2]u8{ins.opcode, ins.func} in ins_names) {
        out = "INVALID"
        return
    }

    name := ins_names[[2]u8{ins.opcode, ins.func}]

    //set_style(ANSI.Bold)
    out = name
    for i in 0..<(5-len(name)) {
        out = fmt.aprintf("%s ", out)
    }
    //set_style(ANSI.Reset)
    delete(name)

    for x in ins_op_to_field[{ins.opcode, ins.func}] {
        //set_style(ANSI.FG_Blue)
        switch x {
        case iff.RDE:
            out = fmt.aprintf("%s %s", out, register_names(ins.rde))
        case iff.RS1:
            out = fmt.aprintf("%s %s", out, register_names(ins.rs1))
        case iff.RS2:
            out = fmt.aprintf("%s %s", out, register_names(ins.rs2))
        case iff.IMM:
            //set_style(ANSI.Reset)
            switch ins.opcode {
            case 0x63..=0x65:
                out = fmt.aprintf("%s 0x%x", out, i64(sign_extend_to_u64(ins.imm, 20))*4)
            case:
                out = fmt.aprintf("%s 0x%x", out, i64(sign_extend_to_u64(ins.imm, 16)))
            }
        }
    }
    //set_style(ANSI.Reset)

    return
}

print_registers :: proc(cpu: ^aphelion_cpu_state) {

    using register_names
    
    fmt.print("\t")
    set_style(ANSI.Bold); fmt.print("pc: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[pc])
    set_style(ANSI.Bold); fmt.print("st: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[st])
    set_style(ANSI.Bold); fmt.print("sp: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[sp])
    set_style(ANSI.Bold); fmt.print("fp: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[fp])
    fmt.print("\n\t")
    set_style(ANSI.Bold); fmt.print("ra: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[ra])
    set_style(ANSI.Bold); fmt.print("rb: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rb])
    set_style(ANSI.Bold); fmt.print("rc: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rc])
    set_style(ANSI.Bold); fmt.print("rd: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rd])
    fmt.print("\n\t")
    set_style(ANSI.Bold); fmt.print("re: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[re])
    set_style(ANSI.Bold); fmt.print("rf: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rf])
    set_style(ANSI.Bold); fmt.print("rg: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rg])
    set_style(ANSI.Bold); fmt.print("rh: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rh])
    fmt.print("\n\t")
    set_style(ANSI.Bold); fmt.print("ri: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[ri])
    set_style(ANSI.Bold); fmt.print("rj: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rj])
    set_style(ANSI.Bold); fmt.print("rk: "); set_style(ANSI.Reset); fmt.printf("0x%16x ", cpu.registers[rk])
    fmt.print("\n")
}

ins_formats : [256]ins_fmt

dynamic_map_ins_formats := map[u8]ins_fmt{
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
    0x27 = ins_fmt.M,
    0x28 = ins_fmt.R,
    0x29 = ins_fmt.M,

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

    0x50 = ins_fmt.M,
    0x51 = ins_fmt.M,
    0x52 = ins_fmt.M,
    0x53 = ins_fmt.M,
    0x54 = ins_fmt.M,
    0x55 = ins_fmt.B,
    0x56 = ins_fmt.B,
    0x57 = ins_fmt.M,

    0x60 = ins_fmt.M,
    0x61 = ins_fmt.M,
    0x62 = ins_fmt.M,
    0x63 = ins_fmt.M,
    0x65 = ins_fmt.J,
    0x66 = ins_fmt.J,

    0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
    //0x64 = ins_fmt.B,
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
    {0x21, 0}   = "lw"   ,
    {0x22, 0}   = "lbs"  ,
    {0x23, 0}   = "lb"   ,
    {0x24, 0}   = "sw"   ,
    {0x25, 0}   = "sb"   ,
    {0x26, 0}   = "swp"  ,
    {0x27, 0}   = "mov"  ,
    {0x28, 0}   = "cmpr" ,
    {0x29, 0}   = "cmpi" ,

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
    {0x51, 0}   = "pushi",
    {0x52, 0}   = "pushz",
    {0x53, 0}   = "pushc",
    {0x54, 0}   = "pop"  ,
    {0x55, 0}   = "enter",
    {0x56, 0}   = "leave",
    {0x57, 0}   = "reloc",

    {0x60, 0}   = "ljal" ,
    {0x61, 0}   = "ljalr",
    {0x62, 0}   = "ret"  ,
    {0x63, 0}   = "retr" ,
    {0x65, 0}   = "jal"  ,
    {0x66, 0}   = "jalr" ,

    {0x64, 0x0} = "bra"  ,
    {0x64, 0x1} = "beq"  ,
    {0x64, 0x2} = "bez"  ,
    {0x64, 0x3} = "blt"  ,
    {0x64, 0x4} = "ble"  ,
    {0x64, 0x5} = "bltu" ,
    {0x64, 0x6} = "bleu" ,
    {0x64, 0x7} = "bpe"  ,
    {0x64, 0x9} = "bne"  ,
    {0x64, 0xa} = "bnz"  ,
    {0x64, 0xb} = "bge"  ,
    {0x64, 0xc} = "bgt"  ,
    {0x64, 0xd} = "bgeu" ,
    {0x64, 0xe} = "bgtu" ,
    {0x64, 0xf} = "bpd"  ,
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
    {0x27, 0}   = []iff{iff.RDE, iff.RS1},
    {0x28, 0}   = []iff{iff.RS1, iff.RS2},
    {0x29, 0}   = []iff{iff.RS1, iff.IMM},

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
    {0x51, 0}   = []iff{iff.IMM},
    {0x52, 0}   = []iff{iff.IMM},
    {0x53, 0}   = []iff{iff.IMM},
    {0x54, 0}   = []iff{iff.RDE},
    {0x55, 0}   = []iff{},
    {0x56, 0}   = []iff{},
    {0x57, 0}   = []iff{iff.RS1, iff.IMM},

    {0x60, 0}   = []iff{iff.RS1, iff.IMM},
    {0x61, 0}   = []iff{iff.RS1, iff.IMM, iff.RDE},
    {0x62, 0}   = []iff{},
    {0x63, 0}   = []iff{iff.RDE},
    {0x65, 0}   = []iff{iff.IMM},
    {0x66, 0}   = []iff{iff.IMM, iff.RDE},

    {0x64, 0x0} = []iff{iff.IMM},
    {0x64, 0x1} = []iff{iff.IMM},
    {0x64, 0x2} = []iff{iff.IMM},
    {0x64, 0x3} = []iff{iff.IMM},
    {0x64, 0x4} = []iff{iff.IMM},
    {0x64, 0x5} = []iff{iff.IMM},
    {0x64, 0x6} = []iff{iff.IMM},
    {0x64, 0x7} = []iff{iff.IMM},
    {0x64, 0x9} = []iff{iff.IMM},
    {0x64, 0xa} = []iff{iff.IMM},
    {0x64, 0xb} = []iff{iff.IMM},
    {0x64, 0xc} = []iff{iff.IMM},
    {0x64, 0xd} = []iff{iff.IMM},
    {0x64, 0xe} = []iff{iff.IMM},
    {0x64, 0xf} = []iff{iff.IMM},
}