#include <stdint.h>
#include <stdlib.h>
#pragma once

// bullshit - yes i know about stdbool dw about it
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

typedef struct instruction_info {
    u8 opcode;
    u8 func;
    u8 rde;
    u8 rs1;
    u8 rs2;
    u64 imm;
} instruction_info;

typedef struct aphelion_cpu_state {
    u64 registers[16];
    bool running;
    bool paused;
    bool step;
    u64 cycle;
    bool increment_next;
    u32 raw_ins;
    instruction_info ins_info;
} aphelion_cpu_state;

typedef struct emulator_state {
    aphelion_cpu_state cpu;
    
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
    fl_parity,
    fl_carry,
    fl_borrow,
    fl_equal,
    fl_greater,
    fl_less,
    fl_greater_unsigned,
    fl_less_unsigned,
    fl_mode,
    fl_carry_unsigned,
    fl_borrow_unsigned,
};

void TODO(const char* msg) {
    printf("TODO: %s\n", msg);
    exit(EXIT_FAILURE);
}

int countones(u64 x) {
    return __builtin_popcountll((unsigned long long) x );
}

void interrupt(aphelion_cpu_state* cpu, u8 code);

u8  read_u8 (u64 addr);
u16 read_u16(u64 addr);
u32 read_u32(u64 addr);
u64 read_u64(u64 addr);

void write_u8 (u64 addr, u8  val);
void write_u16(u64 addr, u16 val);
void write_u32(u64 addr, u32 val);
void write_u64(u64 addr, u64 val);

u64 sign_extend(u64 val, u8 bitsize);

void set_st_flag(aphelion_cpu_state* cpu, st_flag bit, bool value);
bool get_st_flag(aphelion_cpu_state* cpu, st_flag bit);

void cmpr_set_flags(aphelion_cpu_state* cpu, instruction_info* ins);
void cmpi_set_flags(aphelion_cpu_state* cpu, instruction_info* ins);

void addr_set_flags(aphelion_cpu_state* cpu, instruction_info* ins);
void addi_set_flags(aphelion_cpu_state* cpu, instruction_info* ins);
void subr_set_flags(aphelion_cpu_state* cpu, instruction_info* ins);
void subi_set_flags(aphelion_cpu_state* cpu, instruction_info* ins);

bool flag_debug = false;
u64  flag_cycle_limit = 0;
bool flag_no_color = false;
bool flag_halt_inv_op = false;
bool flag_benchmark = false;
char* inpath = "";

bool flag_internal_restart = 0;