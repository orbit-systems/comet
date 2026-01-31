#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "core.h"
#include "aphelion.h"
#include "lock.h"

CpuCore* core_init(void) {
    CpuCore* new_core = malloc(sizeof(*new_core));
    memset(new_core, 0, sizeof(*new_core));
    pthread_mutex_init(&new_core->message_lock, NULL);

    return new_core;
}

CpuError core_enqueue_message(CpuCore* core, SystemMessage message) {
    comet_lock(&core->message_lock);
    {
        vec_append(&core->messages, message);
    }
    comet_unlock(&core->message_lock);
    return ERROR_NONE;
}

CpuError core_write_register(CpuCore* core, AphelGpr reg_idx, u64 value) {
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