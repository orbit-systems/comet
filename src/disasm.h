#pragma once
#define DISASM_H

#include "orbit.h"
#include "comet.h"

extern char SCRATCH[50];

extern char* reg[16];

char* mnemonic(u32 raw);

int arglist_str(char* buf, u32 raw);