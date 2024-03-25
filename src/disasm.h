#pragma once
#define DISASM_H

#include "orbit.h"
#include "comet.h"

extern char SCRATCH[50];

extern char* reg[16];

char* pnemonic(u32 raw);

int arglist_str(char* buf, u32 raw);