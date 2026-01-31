#ifndef MESSAGE_H
#define MESSAGE_H

#include "common/type.h"

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
    MSG(REMOVED) 
    
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
    void* data;
    
    /// Length of the data
    u64 length;
} SystemMessage;


#endif // MESSAGE_H