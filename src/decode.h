#pragma once
#define DECODE_H

#include "comet.h"

// name, opcode, func, format
#define INSTRUCTION_LIST \
    INSTR("int",   0x01, 0, fmt_f) \
    INSTR("iret",  0x01, 1, fmt_f) \
    INSTR("ires",  0x01, 2, fmt_f) \
    INSTR("usr",   0x01, 3, fmt_f) \
    \
    INSTR("outr",  0x02, 0, fmt_m) \
    INSTR("outi",  0x03, 0, fmt_f) \
    INSTR("inr",   0x04, 0, fmt_m) \
    INSTR("ini",   0x05, 0, fmt_f) \
    \
    INSTR("jal",   0x06, 0, fmt_m) \
    INSTR("jalr",  0x07, 0, fmt_m) \
    INSTR("ret",   0x08, 0, fmt_m) \
    INSTR("retr",  0x09, 0, fmt_m) \
    INSTR("bra",   0x0a, 0, fmt_b) \
    INSTR("beq",   0x0a, 1, fmt_b) \
    INSTR("bez",   0x0a, 2, fmt_b) \
    INSTR("blt",   0x0a, 3, fmt_b) \
    INSTR("ble",   0x0a, 4, fmt_b) \
    INSTR("bltu",  0x0a, 5, fmt_b) \
    INSTR("bleu",  0x0a, 6, fmt_b) \
    INSTR("beq",   0x0a, 9, fmt_b) \
    INSTR("bez",   0x0a, 10, fmt_b) \
    INSTR("blt",   0x0a, 11, fmt_b) \
    INSTR("ble",   0x0a, 12, fmt_b) \
    INSTR("bltu",  0x0a, 13, fmt_b) \
    INSTR("bleu",  0x0a, 14, fmt_b) \
    \
    INSTR("push",  0x0b, 0, fmt_m) \
    INSTR("pop",   0x0c, 0, fmt_m) \
    INSTR("enter", 0x0d, 0, fmt_b) \
    INSTR("leave", 0x0e, 0, fmt_b) \
    \
    INSTR("lli",   0x10, 0, fmt_f) \
    INSTR("llis",  0x10, 1, fmt_f) \
    INSTR("lui",   0x10, 2, fmt_f) \
    INSTR("luis",  0x10, 3, fmt_f) \
    INSTR("lti",   0x10, 4, fmt_f) \
    INSTR("ltis",  0x10, 5, fmt_f) \
    INSTR("ltui",  0x10, 6, fmt_f) \
    INSTR("ltuis", 0x10, 7, fmt_f) \
    INSTR("lw",    0x11, 0, fmt_e) \
    INSTR("lh",    0x12, 0, fmt_e) \
    INSTR("lhs",   0x13, 0, fmt_e) \
    INSTR("lq",    0x14, 0, fmt_e) \
    INSTR("lqs",   0x15, 0, fmt_e) \
    INSTR("lb",    0x16, 0, fmt_e) \
    INSTR("lbs",   0x17, 0, fmt_e) \
    INSTR("sw",    0x18, 0, fmt_e) \
    INSTR("sh",    0x19, 0, fmt_e) \
    INSTR("sq",    0x1a, 0, fmt_e) \
    INSTR("sb",    0x1b, 0, fmt_e) \
    \
    INSTR("cmpr",  0x1e, 0, fmt_m) \
    INSTR("cmpi",  0x1f, 0, fmt_f) \
    \
    INSTR("addr",  0x20, 0, fmt_r) \
    INSTR("addi",  0x21, 0, fmt_m) \
    INSTR("subr",  0x22, 0, fmt_r) \
    INSTR("subi",  0x23, 0, fmt_m) \
    INSTR("imulr", 0x24, 0, fmt_r) \
    INSTR("imuli", 0x25, 0, fmt_m) \
    INSTR("idivr", 0x26, 0, fmt_r) \
    INSTR("idivi", 0x27, 0, fmt_m) \
    INSTR("umulr", 0x28, 0, fmt_r) \
    INSTR("umuli", 0x29, 0, fmt_m) \
    INSTR("udivr", 0x2a, 0, fmt_r) \
    INSTR("udivi", 0x2b, 0, fmt_m) \
    INSTR("remr",  0x2c, 0, fmt_r) \
    INSTR("remi",  0x2d, 0, fmt_m) \
    INSTR("modr",  0x2e, 0, fmt_r) \
    INSTR("modi",  0x2f, 0, fmt_m) \
    \
    INSTR("andr",  0x30, 0, fmt_r) \
    INSTR("andi",  0x31, 0, fmt_m) \
    INSTR("orr",   0x32, 0, fmt_r) \
    INSTR("ori",   0x33, 0, fmt_m) \
    INSTR("norr",  0x34, 0, fmt_r) \
    INSTR("nori",  0x35, 0, fmt_m) \
    INSTR("xorr",  0x36, 0, fmt_r) \
    INSTR("xori",  0x37, 0, fmt_m) \
    INSTR("shlr",  0x38, 0, fmt_r) \
    INSTR("shli",  0x39, 0, fmt_m) \
    INSTR("asrr",  0x3a, 0, fmt_r) \
    INSTR("asri",  0x3b, 0, fmt_m) \
    INSTR("lsrr",  0x3c, 0, fmt_r) \
    INSTR("lsri",  0x3d, 0, fmt_m) \
    INSTR("bitr",  0x3e, 0, fmt_r) \
    INSTR("biti",  0x3f, 0, fmt_m) \
    \
    INSTR("fcmp",  0x40, 0, fmt_e) \
    INSTR("fto",   0x41, 0, fmt_e) \
    INSTR("ffrom", 0x42, 0, fmt_e) \
    INSTR("fneg",  0x43, 0, fmt_e) \
    INSTR("fabs",  0x44, 0, fmt_e) \
    INSTR("fadd",  0x45, 0, fmt_e) \
    INSTR("fsub",  0x46, 0, fmt_e) \
    INSTR("fmul",  0x47, 0, fmt_e) \
    INSTR("fdiv",  0x48, 0, fmt_e) \
    INSTR("fma",   0x49, 0, fmt_e) \
    INSTR("fsqrt", 0x4a, 0, fmt_e) \
    INSTR("fmin",  0x4b, 0, fmt_e) \
    INSTR("fmax",  0x4c, 0, fmt_e) \
    INSTR("fsat",  0x4d, 0, fmt_e) \
    INSTR("fcnv",  0x4e, 0, fmt_e) \


u64 sign_extend(u64 val, u8 bitsize);
u64 zero_extend(u64 val, u8 bitsize);

