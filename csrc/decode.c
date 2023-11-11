#include "comet.h"
#pragma once

typedef u8 ins_fmt; enum {
    fmt_R,
    fmt_M,
    fmt_F,
    fmt_J,
    fmt_B
};

const ins_fmt ins_formats[256] = {
    [0x0A] = fmt_B,
    [0x10] = fmt_B,
    [0x11] = fmt_M,
    [0x12] = fmt_B,

    [0x20] = fmt_F,
    [0x21] = fmt_M,
    [0x22] = fmt_M,
    [0x23] = fmt_M,
    [0x24] = fmt_M,
    [0x25] = fmt_M,
    [0x26] = fmt_M,
    [0x27] = fmt_M,
    [0x28] = fmt_R,
    [0x29] = fmt_M,

    [0x30] = fmt_R,
    [0x31] = fmt_M,
    [0x32] = fmt_R,
    [0x33] = fmt_M,
    [0x34] = fmt_R,
    [0x35] = fmt_M,
    [0x36] = fmt_R,
    [0x37] = fmt_M,
    [0x38] = fmt_R,
    [0x39] = fmt_M,
    [0x3a] = fmt_R,
    [0x3b] = fmt_M,

    [0x40] = fmt_R,
    [0x41] = fmt_M,
    [0x42] = fmt_R,
    [0x43] = fmt_M,
    [0x44] = fmt_R,
    [0x45] = fmt_M,
    [0x46] = fmt_R,
    [0x47] = fmt_M,
    [0x48] = fmt_R,
    [0x49] = fmt_M,
    [0x4a] = fmt_R,
    [0x4b] = fmt_M,
    [0x4c] = fmt_R,
    [0x4d] = fmt_M,

    [0x50] = fmt_M,
    [0x51] = fmt_M,
    [0x52] = fmt_M,
    [0x53] = fmt_M,
    [0x54] = fmt_M,
    [0x55] = fmt_B,
    [0x56] = fmt_B,
    [0x57] = fmt_M,

    [0x60] = fmt_M,
    [0x61] = fmt_M,
    [0x62] = fmt_M,
    [0x64] = fmt_B,
    [0x63] = fmt_M,
    [0x65] = fmt_J,
    [0x66] = fmt_J,
};

void raw_decode(u32 ins, instruction_info* info) {
    info->opcode = (u8) (ins & 0xFF);
    ins_fmt current_fmt = ins_formats[info->opcode];

    switch (current_fmt) {
    case fmt_R:
        info->rde  = (u8)  (ins >> 28 & 0xF);
        info->rs1  = (u8)  (ins >> 24 & 0xF);
        info->rs2  = (u8)  (ins >> 20 & 0xF);
        info->imm  = (u64) (ins >> 8  & 0xFFF);
        break;
    case fmt_M:
        info->rde  = (u8)  (ins >> 28 & 0xF);
        info->rs1  = (u8)  (ins >> 24 & 0xF);
        info->imm  = (u64) (ins >> 8  & 0xFFFF);
        break;
    case fmt_F:
        info->rde  = (u8)  (ins >> 28 & 0xF);
        info->func = (u8)  (ins >> 24 & 0xF);
        info->imm  = (u64) (ins >> 8  & 0xFFFF);
        break;
    case fmt_J:
        info->rde  = (u8)  (ins >> 28 & 0xF);
        info->imm  = (u64) (ins >> 8  & 0xFFFFF);
        break;
    case fmt_B:
        info->func = (u8)  (ins >> 28 & 0xF);
        info->imm  = (u64) (ins >> 8  & 0xFFFFF);
        break;
    }
}

#define i(op, f) [op*0x10 + f]
const char* ins_names[] = {
    i(0x0A, 0)   = "nop"  ,
    i(0x10, 0)   = "int"  ,
    i(0x11, 0)   = "inv"  ,
    i(0x12, 0)   = "usr"  ,

    i(0x20, 0)   = "lli"  ,
    i(0x20, 1)   = "llis" ,
    i(0x20, 2)   = "lui"  ,
    i(0x20, 3)   = "luis" ,
    i(0x20, 4)   = "lti"  ,
    i(0x20, 5)   = "ltis" ,
    i(0x20, 6)   = "ltui" ,
    i(0x20, 7)   = "ltuis",
    i(0x21, 0)   = "lw"   ,
    i(0x22, 0)   = "lbs"  ,
    i(0x23, 0)   = "lb"   ,
    i(0x24, 0)   = "sw"   ,
    i(0x25, 0)   = "sb"   ,
    i(0x26, 0)   = "swp"  ,
    i(0x27, 0)   = "mov"  ,
    i(0x28, 0)   = "cmpr" ,
    i(0x29, 0)   = "cmpi" ,

    i(0x30, 0)   = "addr" ,
    i(0x31, 0)   = "addi" ,
    i(0x32, 0)   = "adcr" ,
    i(0x33, 0)   = "adci" ,
    i(0x34, 0)   = "subr" ,
    i(0x35, 0)   = "subi" ,
    i(0x36, 0)   = "sbbr" ,
    i(0x37, 0)   = "sbbi" ,
    i(0x38, 0)   = "mulr" ,
    i(0x39, 0)   = "muli" ,
    i(0x3a, 0)   = "divr" ,
    i(0x3b, 0)   = "divi" ,

    i(0x40, 0)   = "andr" ,
    i(0x41, 0)   = "andi" ,
    i(0x42, 0)   = "orr"  ,
    i(0x43, 0)   = "ori"  ,
    i(0x44, 0)   = "norr" ,
    i(0x45, 0)   = "nori" ,
    i(0x46, 0)   = "xorr" ,
    i(0x47, 0)   = "xori" ,
    i(0x48, 0)   = "shlr" ,
    i(0x49, 0)   = "shli" ,
    i(0x4a, 0)   = "asrr" ,
    i(0x4b, 0)   = "asri" ,
    i(0x4c, 0)   = "lsrr" ,
    i(0x4d, 0)   = "lsri" ,

    i(0x50, 0)   = "push" ,
    i(0x51, 0)   = "pushi",
    i(0x52, 0)   = "pushz",
    i(0x53, 0)   = "pushc",
    i(0x54, 0)   = "pop"  ,
    i(0x55, 0)   = "enter",
    i(0x56, 0)   = "leave",
    i(0x57, 0)   = "reloc",

    i(0x60, 0)   = "ljal" ,
    i(0x61, 0)   = "ljalr",
    i(0x62, 0)   = "ret"  ,
    i(0x63, 0)   = "retr" ,
    i(0x65, 0)   = "jal"  ,
    i(0x66, 0)   = "jalr" ,

    i(0x64, 0x0) = "bra"  ,
    i(0x64, 0x1) = "beq"  ,
    i(0x64, 0x2) = "bez"  ,
    i(0x64, 0x3) = "blt"  ,
    i(0x64, 0x4) = "ble"  ,
    i(0x64, 0x5) = "bltu" ,
    i(0x64, 0x6) = "bleu" ,
    i(0x64, 0x7) = "bpe"  ,
    i(0x64, 0x9) = "bne"  ,
    i(0x64, 0xa) = "bnz"  ,
    i(0x64, 0xb) = "bge"  ,
    i(0x64, 0xc) = "bgt"  ,
    i(0x64, 0xd) = "bgeu" ,
    i(0x64, 0xe) = "bgtu" ,
    i(0x64, 0xf) = "bpd"  ,
};
#undef i

// #define instruction_name(opcode, func) ins_names[opcode*0x10 + func]

const char* instruction_name(u8 opcode, u8 func) {
    return ins_names[opcode*0x10 + func];
}