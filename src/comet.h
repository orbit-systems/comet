#pragma once
#define COMET_H

#include "orbit.h"
// typedef struct instruction_info_s {
//     u8 opcode;
//     u8 func;
//     u8 rde;
//     u8 rs1;
//     u8 rs2;
//     u64 imm;
// } instruction_info;

typedef union {
    u8 opcode;
    struct {
        u32 opcode : 8;
        u32 imm    : 8;
        u32 func   : 4;
        u32 rs2    : 4;
        u32 rs1    : 4;
        u32 rde    : 4;
    } E;
    struct {
        u32 opcode : 8;
        u32 imm    : 12;
        u32 rs2    : 4;
        u32 rs1    : 4;
        u32 rde    : 4;
    } R;
    struct {
        u32 opcode : 8;
        u32 imm    : 16;
        u32 rs1    : 4;
        u32 rde    : 4;
    } M;
    struct {
        u32 opcode : 8;
        u32 imm    : 16;
        u32 func   : 4;
        u32 rde    : 4;
    } F;
    struct {
        u32 opcode : 8;
        u32 imm    : 20;
        u32 func   : 4;
    } B;
} instruction_info;

static_assert(sizeof(instruction_info) == sizeof(u32), "sizeof(instruction_info) != sizeof(u32)");

typedef struct CPU_s {
    u64 registers[16];
    u64 cycle;
    u32 raw_ins;
    instruction_info ins_info;
    bool increment_next;
    bool running;

    bool user_mode;
} CPU;

typedef struct IC_s {
    u64 ivt_base_address;
} IC;

typedef struct MMU_s {
    u8* memory;
    u64 mem_max;

    u64 page_table_base;
} MMU;

typedef struct emulator_s {
    CPU cpu;
    IC ic; // interrupt controller
    MMU mmu; // memory management unit

    bool flag_debug;
    u64  flag_cycle_limit;
    bool flag_no_color;
    bool flag_benchmark;
    char* flag_bin_path;

    bool flag_internal_restart;
} emulator;

typedef u8 register_name; enum {
    r_rz,
    r_ra, r_rb, r_rc, r_rd,
    r_re, r_rf, r_rg, r_rh,
    r_ri, r_rj, r_rk,
    r_ip,
    r_sp, r_fp,
    r_st,
};

typedef u8 st_flag; enum {
    fl_sign = 0,
    fl_zero = 1,
    fl_carry_borrow = 2,
    fl_carry_borrow_unsigned = 3,
    fl_equal = 4,
    fl_less = 5,
    fl_less_unsigned = 6,
    fl_mode = 7,

    fl_ext_f = 31,
};

typedef u8 interrupt_code; enum {
    int_divide_by_zero = 0,
    int_breakpoint,
    int_invalid_instruction,
    int_stack_underflow,
    int_unaligned_access,
    int_access_violation,
};

typedef u8 ins_fmt; enum {
    fmt_e,
    fmt_r,
    fmt_m,
    fmt_f,
    fmt_b,
};

void raw_decode(u32 ins, instruction_info* restrict info);
char* get_ins_name(instruction_info* restrict ins);
void exec_instruction(instruction_info* restrict ins);

u64 sign_extend(u64 val, u8 bitsize);

void set_st_flag(u64* restrict register_bank, st_flag bit, bool value);
bool get_st_flag(u64* restrict register_bank, st_flag bit);

extern emulator comet;