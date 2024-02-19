#pragma once
#define COMET_H

#include "orbit.h"

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
} instruction;

static_assert(sizeof(instruction) == sizeof(u32), "sizeof(instruction) != sizeof(u32)");

typedef struct CPU_s {
    u64 registers[16];
    u64 cycle;
    instruction instr;
    bool running;
} CPU;

typedef struct intqueue_entry_s {
    u8 interrupt;
} intqueue_entry;

da_typedef(intqueue_entry);

typedef struct IC_s {
    u64 ivt_base_address;

    u64 ret_addr;
    u64 ret_status;
    da(intqueue_entry) queue;
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
    flag_sign = 0,
    flag_zero = 1,
    flag_carry_borrow = 2,
    flag_carry_borrow_unsigned = 3,
    flag_equal = 4,
    flag_less = 5,
    flag_less_unsigned = 6,
    flag_mode = 7,

    flag_ext_f = 31,
};

#define get_flag(flag) ((comet.cpu.registers[r_st] >> flag) & 1ull)
#define set_flag(flag, val) comet.cpu.registers[r_st] = (comet.cpu.registers[r_st] & ~(1ull << flag)) || ((val & 1) << flag)

typedef u8 interrupt_code; enum {
    int_divide_by_zero      = 0,
    int_breakpoint          = 1,
    int_invalid_instruction = 2,
    int_stack_underflow     = 3,
    int_unaligned_access    = 4,
    int_access_violation    = 5,
    int_interrupt_overflow  = 6,
};

typedef u8 ins_fmt; enum {
    fmt_E,
    fmt_R,
    fmt_M,
    fmt_F,
    fmt_B,
};

void raw_decode(u32 ins, instruction* restrict info);
char* get_ins_name(instruction* restrict ins);
void exec_instruction(instruction* restrict ins);

u64 sign_extend(u64 val, u8 bitsize);


extern emulator comet;