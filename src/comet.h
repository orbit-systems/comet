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
#define true (!false)

#define U64_MAX ((i64)0xFFFFFFFFFFFFFFFF)
#define U64_MIN ((i64)0)
#define I64_MAX ((i64)0x7FFFFFFFFFFFFFFF)
#define I64_MIN ((i64)0x8000000000000000)

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

typedef struct CPU_s {
    u64 registers[16];
    u64 cycle;
    u32 raw_ins;
    instruction_info ins_info;
    bool increment_next;
    bool running;
    bool paused;
    bool step;

    bool user_mode;
} CPU;

typedef struct IC_s {
    u64 ivt_base_address;
} IC;

typedef struct MMU_s {
    char* memory;
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
    fmt_r,
    fmt_m,
    fmt_f,
    fmt_b,
    fmt_e,
};

typedef u8 access_mode; enum {
    access_translate, // dont care about permissions, just map the address
    access_read,
    access_write,
    access_execute,
};

void raw_decode(u32 ins, instruction_info* restrict info);
char* get_ins_name(instruction_info* restrict ins);
void exec_instruction(instruction_info* restrict ins);

typedef u8 mmu_response; enum {
    tr_success,
    tr_invalid,
    tr_noperms,
};

mmu_response translate_address(u64 virtual, u64* physical, access_mode mode);

mmu_response phys_read_u8 (u64 addr, u8*  restrict var);
mmu_response phys_read_u16(u64 addr, u16* restrict var);
mmu_response phys_read_u32(u64 addr, u32* restrict var);
mmu_response phys_read_u64(u64 addr, u64* restrict var);

mmu_response phys_write_u8 (u64 addr, u8  value);
mmu_response phys_write_u16(u64 addr, u16 value);
mmu_response phys_write_u32(u64 addr, u32 value);
mmu_response phys_write_u64(u64 addr, u64 value);

u64 align_backwards(u64 ptr, u64 align);

bool init_memory();
void free_memory();
bool load_image(FILE* bin);

u64 sign_extend(u64 val, u8 bitsize);

void set_st_flag(u64* restrict register_bank, st_flag bit, bool value);
bool get_st_flag(u64* restrict register_bank, st_flag bit);

extern emulator comet;