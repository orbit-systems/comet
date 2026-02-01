#ifndef MESSAGE_H
#define MESSAGE_H

#include <string.h>
#include "common/type.h"
#include "refcountarena.h"

#define MESSAGES \
    MSG(SYS_NONE) \
    MSG(SYS_STORE_OK) \
    MSG(SYS_STORE_BAD_ADDR) \
    MSG(SYS_LOAD_OK) \
    MSG(SYS_LOAD_BAD_ADDR) \
    MSG(SYS_FAULT) \
    MSG(SYS_STOP) \
    MSG(CORE_NONE) \
    MSG(CORE_STORE) \
    MSG(CORE_LOAD) \
    MSG(CORE_RESET) \
    MSG(CORE_FAULT) \
    MSG(CORE_STOP) \
    MSG(NONE) 
    
/// Message Type->char* array
extern char* message_str[];

/// Enum of message types
typedef enum: u8 {
    #define MSG(msg) MSG_##msg,
        MESSAGES
    #undef MSG
} SystemMessageType;

/// System message
typedef struct {
    /// Type of message
    SystemMessageType type;

    /// Data in the message
    /// NOTE: data is assumed to be owned by the SystemMessage. Do not rely on it being valid after a message has been handled!
    void* data;
    
    /// Length of the data
    u64 length;
} SystemMessage;

typedef struct {
    u64 addr;
    u64 size;
} SystemMessageCoreLoad;

// TODO: replace this
#define CREATE_MESSAGE(t, d, l) \
({ \
    void* __d = NULL; \
    if (l != 0) { \
        __d = rca_malloc((l)); \
        typeof(d) __v = (d); \
        memcpy(__d, &__v, (l)); \
    } \
    (SystemMessage){.type=(t), .data=__d, .length=(l)}; \
})

#define CREATE_MESSAGE_FROM_BUFF(t, d, l) \
({ \
    void* __d = NULL; \
    __d = rca_malloc((l)); \
    memcpy(__d, d, (l)); \
    (SystemMessage){.type=(t), .data=__d, .length=(l)}; \
})


#endif // MESSAGE_H