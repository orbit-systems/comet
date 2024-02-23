#pragma once
#define DECODE_H

#include "comet.h"

// name, opcode, func, format
#define INSTRUCTION_LIST \
    INSTR("int",   0x01, 0, fmt_F) \
    INSTR("iret",  0x01, 1, fmt_F) \
    INSTR("ires",  0x01, 2, fmt_F) \
    INSTR("usr",   0x01, 3, fmt_F) \
    \
    INSTR("outr",  0x02, 0, fmt_M) \
    INSTR("outi",  0x03, 0, fmt_M) \
    INSTR("inr",   0x04, 0, fmt_M) \
    INSTR("ini",   0x05, 0, fmt_M) \
    \
    INSTR("jal",   0x06, 0, fmt_M) \
    INSTR("jalr",  0x07, 0, fmt_M) \
    INSTR("ret",   0x08, 0, fmt_M) \
    INSTR("retr",  0x09, 0, fmt_M) \
    INSTR("bra",   0x0a, 0, fmt_B) \
    INSTR("beq",   0x0a, 1, fmt_B) \
    INSTR("bez",   0x0a, 2, fmt_B) \
    INSTR("blt",   0x0a, 3, fmt_B) \
    INSTR("ble",   0x0a, 4, fmt_B) \
    INSTR("bltu",  0x0a, 5, fmt_B) \
    INSTR("bleu",  0x0a, 6, fmt_B) \
    INSTR("bne",   0x0a, 9, fmt_B) \
    INSTR("bnz",   0x0a, 10, fmt_B) \
    INSTR("bge",   0x0a, 11, fmt_B) \
    INSTR("bgt",   0x0a, 12, fmt_B) \
    INSTR("bgeu",  0x0a, 13, fmt_B) \
    INSTR("bgtu",  0x0a, 14, fmt_B) \
    \
    INSTR("push",  0x0b, 0, fmt_M) \
    INSTR("pop",   0x0c, 0, fmt_M) \
    INSTR("enter", 0x0d, 0, fmt_B) \
    INSTR("leave", 0x0e, 0, fmt_B) \
    \
    INSTR("lli",   0x10, 0, fmt_F) \
    INSTR("llis",  0x10, 1, fmt_F) \
    INSTR("lui",   0x10, 2, fmt_F) \
    INSTR("luis",  0x10, 3, fmt_F) \
    INSTR("lti",   0x10, 4, fmt_F) \
    INSTR("ltis",  0x10, 5, fmt_F) \
    INSTR("ltui",  0x10, 6, fmt_F) \
    INSTR("ltuis", 0x10, 7, fmt_F) \
    INSTR("lw",    0x11, 0, fmt_E) \
    INSTR("lh",    0x12, 0, fmt_E) \
    INSTR("lhs",   0x13, 0, fmt_E) \
    INSTR("lq",    0x14, 0, fmt_E) \
    INSTR("lqs",   0x15, 0, fmt_E) \
    INSTR("lb",    0x16, 0, fmt_E) \
    INSTR("lbs",   0x17, 0, fmt_E) \
    INSTR("sw",    0x18, 0, fmt_E) \
    INSTR("sh",    0x19, 0, fmt_E) \
    INSTR("sq",    0x1a, 0, fmt_E) \
    INSTR("sb",    0x1b, 0, fmt_E) \
    \
    INSTR("cmpr",  0x1e, 0, fmt_M) \
    INSTR("cmpi",  0x1f, 0, fmt_F) \
    \
    INSTR("addr",  0x20, 0, fmt_R) \
    INSTR("addi",  0x21, 0, fmt_M) \
    INSTR("subr",  0x22, 0, fmt_R) \
    INSTR("subi",  0x23, 0, fmt_M) \
    INSTR("imulr", 0x24, 0, fmt_R) \
    INSTR("imuli", 0x25, 0, fmt_M) \
    INSTR("idivr", 0x26, 0, fmt_R) \
    INSTR("idivi", 0x27, 0, fmt_M) \
    INSTR("umulr", 0x28, 0, fmt_R) \
    INSTR("umuli", 0x29, 0, fmt_M) \
    INSTR("udivr", 0x2a, 0, fmt_R) \
    INSTR("udivi", 0x2b, 0, fmt_M) \
    INSTR("remr",  0x2c, 0, fmt_R) \
    INSTR("remi",  0x2d, 0, fmt_M) \
    INSTR("modr",  0x2e, 0, fmt_R) \
    INSTR("modi",  0x2f, 0, fmt_M) \
    \
    INSTR("andr",  0x30, 0, fmt_R) \
    INSTR("andi",  0x31, 0, fmt_M) \
    INSTR("orr",   0x32, 0, fmt_R) \
    INSTR("ori",   0x33, 0, fmt_M) \
    INSTR("norr",  0x34, 0, fmt_R) \
    INSTR("nori",  0x35, 0, fmt_M) \
    INSTR("xorr",  0x36, 0, fmt_R) \
    INSTR("xori",  0x37, 0, fmt_M) \
    INSTR("shlr",  0x38, 0, fmt_R) \
    INSTR("shli",  0x39, 0, fmt_M) \
    INSTR("asrr",  0x3a, 0, fmt_R) \
    INSTR("asri",  0x3b, 0, fmt_M) \
    INSTR("lsrr",  0x3c, 0, fmt_R) \
    INSTR("lsri",  0x3d, 0, fmt_M) \
    INSTR("bitr",  0x3e, 0, fmt_R) \
    INSTR("biti",  0x3f, 0, fmt_M) \
    \
    INSTR("fcmp",  0x40, 0, fmt_E) \
    INSTR("fto",   0x41, 0, fmt_E) \
    INSTR("ffrom", 0x42, 0, fmt_E) \
    INSTR("fneg",  0x43, 0, fmt_E) \
    INSTR("fabs",  0x44, 0, fmt_E) \
    INSTR("fadd",  0x45, 0, fmt_E) \
    INSTR("fsub",  0x46, 0, fmt_E) \
    INSTR("fmul",  0x47, 0, fmt_E) \
    INSTR("fdiv",  0x48, 0, fmt_E) \
    INSTR("fma",   0x49, 0, fmt_E) \
    INSTR("fsqrt", 0x4a, 0, fmt_E) \
    INSTR("fmin",  0x4b, 0, fmt_E) \
    INSTR("fmax",  0x4c, 0, fmt_E) \
    INSTR("fsat",  0x4d, 0, fmt_E) \
    INSTR("fcnv",  0x4e, 0, fmt_E) \
    INSTR("fnan",  0x4f, 0, fmt_E)
