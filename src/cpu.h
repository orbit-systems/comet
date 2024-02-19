#pragma once
#define CPU_H

#include "comet.h"
#include "mmu.h"
#include "ic.h"

void run();

void push_stack(u64 data);
void pop_stack(u64* val);

#define sign_extend(val, bitsize) ((u64)((i64)((u64)val << (64-bitsize)) >> (64-bitsize)))
#define zero_extend(val, bitsize) ((u64)((u64)((u64)val << (64-bitsize)) >> (64-bitsize)))

#define current_instr (*((instruction*)&comet.cpu.registers[r_st] + 1))
#define regval(reg) comet.cpu.registers[reg]