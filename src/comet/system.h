#ifndef SYSTEM_H
#define SYSTEM_H

#include <pthread.h>

#include "common/type.h"
#include "common/vec.h"
#include "physmem.h"

typedef enum: u8 {
    MSG_OK,
    MSG_STORE_OK,
    MSG_STORE_BAD_ADDR,
    MSG_LOAD_OK,
    MSG_LOAD_BAD_ADDR,
} SystemMessageType;

typedef struct {
    SystemMessageType type;
    void* data;
    u64 length;
} SystemMessage;

typedef struct {
    /// Queue lock on messages
    pthread_mutex_t message_lock;

    /// Current queue of messages
    Vec(SystemMessage) messages;

    /// Physical memory unit
    PhysMemUnit* phys_mem;
} System;

#endif // SYSTEM_H