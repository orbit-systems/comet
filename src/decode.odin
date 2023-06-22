package comet

ins_fmt :: enum {
    R, M, F, J, B,
}

ins_info :: struct {
    opcode : u8,
    func   : u8,
    rde    : u8,
    rs1    : u8,
    rs2    : u8,
    imm    : u64,
}

raw_decode :: proc(ins: u32) -> (ins_dec: ins_info) {

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
se_to_u64 :: proc(val:u64, bitsize: u8) -> u64 {
    return u64(i64(val << (64-bitsize)) >> (64-bitsize))
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