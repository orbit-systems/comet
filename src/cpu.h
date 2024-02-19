#pragma once
#define CPU_H

#include "comet.h"
#include "mmu.h"
#include "ic.h"

void run();

#define current_instr (*((instruction*)&comet.cpu.registers[r_st] + 1))