#include "comet.h"
#pragma once

u8  read_u8 (u64 addr) {TODO("memory");return 0;}
u16 read_u16(u64 addr) {TODO("memory");return 0;}
u32 read_u32(u64 addr) {TODO("memory");return 0;}
u64 read_u64(u64 addr) {TODO("memory");return 0;}

void write_u8 (u64 addr, u8  val) {TODO("memory");}
void write_u16(u64 addr, u16 val) {TODO("memory");}
void write_u32(u64 addr, u32 val) {TODO("memory");}
void write_u64(u64 addr, u64 val) {TODO("memory");}

void interrupt(aphelion_cpu_state* cpu, u8 code) {
    if (flag_halt_inv_op && code == 1) {
        cpu->running = false;
        return;
    }
    cpu->registers[r_pc] = read_u64(code*8);
}