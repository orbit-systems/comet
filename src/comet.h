#pragma once
#define COMET_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

// not gonna use stdbool fuck you
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;
typedef uint64_t b64;
typedef uint32_t b32;
typedef uint16_t b16;
typedef uint8_t  b8;
typedef uint8_t  bool;
#define false 0
#define true 1

#define U64_MAX (i64)0xFFFFFFFFFFFFFFFF
#define U64_MIN (i64)0
#define I64_MAX (i64)0x7FFFFFFFFFFFFFFF
#define I64_MIN (i64)0x8000000000000000

#define MEM_PAGE_SIZE 0x4000
#define MEM_AMNT_PAGES 4096
#define MEM_PHYS_MAX (MEM_PAGE_SIZE*MEM_AMNT_PAGES-1)

#define TODO(msg) \
    printf("TODO: \"%s\" at %s:%d\n", (msg), (__FILE__), (__LINE__)); \
    exit(EXIT_FAILURE) \

#define sign(x) ((x > 0) - (x < 0))

typedef struct instruction_info_s {
    u8 opcode;
    u8 func;
    u8 rde;
    u8 rs1;
    u8 rs2;
    u64 imm;
} instruction_info;

typedef struct cpu_state_s {
    u64 registers[16];
    bool running;
    bool paused;
    bool step;
    u64 cycle;
    bool increment_next;
    u32 raw_ins;
    instruction_info ins_info;
} cpu_state;

typedef struct ic_state_s {
    u64 ivt_base_address;
} ic_state;

typedef struct emulator_state_s {
    cpu_state cpu;
    ic_state ic; // interrupt controller

    bool flag_debug;
    u64  flag_cycle_limit;
    bool flag_no_color;
    bool flag_benchmark;
    char* flag_bin_path;

    bool flag_internal_restart;
} emulator_state;

typedef u8 register_name; enum {
    r_rz,
    r_ra, r_rb, r_rc, r_rd,
    r_re, r_rf, r_rg, r_rh,
    r_ri, r_rj, r_rk,
    r_pc,
    r_sp, r_fp,
    r_st,
};

typedef u8 st_flag; enum {
    fl_sign = 0,
    fl_zero,
    fl_carry_borrow,
    fl_carry_borrow_unsigned,
    fl_equal,
    fl_less,
    fl_less_unsigned,
    fl_mode,
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
    fmt_r,
    fmt_m,
    fmt_f,
//  fmt_j,
    fmt_b
};

void raw_decode(u32 ins, instruction_info* restrict info);
char* get_ins_name(instruction_info* restrict ins);
void exec_instruction(emulator_state* restrict comet, instruction_info* restrict ins);

bool phys_read_u8 (u64 addr, u8*  restrict var);
bool phys_read_u16(u64 addr, u16* restrict var);
bool phys_read_u32(u64 addr, u32* restrict var);
bool phys_read_u64(u64 addr, u64* restrict var);

bool phys_write_u8 (u64 addr, u8  value);
bool phys_write_u16(u64 addr, u16 value);
bool phys_write_u32(u64 addr, u32 value);
bool phys_write_u64(u64 addr, u64 value);

u64 align_backwards(u64 ptr, u64 align);

bool init_memory();
void free_memory();
void load_image(FILE* bin);

u64 sign_extend(u64 val, u8 bitsize);

void set_st_flag(u64* restrict register_bank, st_flag bit, bool value);
bool get_st_flag(u64* restrict register_bank, st_flag bit);

