#include "comet.h"
#include "decode.h"

const ins_fmt ins_formats[256] = {
    #define INSTR(name, opcode, func, format) [opcode] = format,
    INSTRUCTION_LIST
    #undef INSTR
};

const char* ins_names[] = {
    #define INSTR(name, opcode, func, format) [(opcode) * 0x10 + (func)] = name,
    INSTRUCTION_LIST
    #undef INSTR
};