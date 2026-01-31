#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "core.h"
#include "aphelion.h"


CpuCore* core_init(void) {
    CpuCore* new_core = malloc(sizeof(*new_core));
    memset(new_core, 0, sizeof(*new_core));

    return new_core;
}

CpuError core_modify_register(CpuCore* core, AphelGpr reg_idx, u64 value) {
    assert(reg_idx < GPR_COUNT && "Register index larger than Aphelion register file!");

    switch(reg_idx) {
    case GPR_ZR:
    case GPR_IP:
        break;
    default:
        core->regfile[reg_idx] = value;
    break;
    }

    return ERROR_NONE;
}

CpuError core_execute_instruction(CpuCore* core, u64 instruction) {
    return ERROR_NONE;
}