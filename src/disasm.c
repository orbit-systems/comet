#include "orbit.h"
#include "comet.h"

char SCRATCH[50];

char* reg[] = {
    "rz",
    "ra",
    "rb",
    "rc",
    "rd",
    "re",
    "rf",
    "rg",
    "rh",
    "ri",
    "rj",
    "rk",
    "ip",
    "sp",
    "fp",
    "st",
};

// returns -1 if invalid instruction
int arglist_str(char* buf, u32 raw) {
    instruction ci;
    ci.raw = raw;

    if (buf == NULL) {
        buf = &SCRATCH[0];
        memset(SCRATCH, 0, sizeof(SCRATCH));
    }
    
    switch (ci.opcode) {
    case 0x01: // system control
        switch(ci.F.func) {
        case 0x00: // int
            return sprintf(buf, "%u", ci.F.imm % 255);
        case 0x01: // iret
        case 0x02: // ires
            return 0;
        case 0x03: // usr
            return sprintf(buf, "%s", reg[ci.F.rde]);
        default:
            return -1;
        }
    case 0x02: // outr
        return sprintf(buf, "%s, %s", reg[ci.M.rde], reg[ci.M.rs1]);
    case 0x03: // outi
        return sprintf(buf, "%s, %u", reg[ci.M.rs1], ci.M.imm);
    case 0x04: // inr
        return sprintf(buf, "%s, %s",  reg[ci.M.rde], reg[ci.M.rs1]);
    case 0x05: // ini
        return sprintf(buf, "%s, %u",  reg[ci.M.rde], ci.M.imm);
    case 0x06: // jal
        return sprintf(buf, "%s, %d",  reg[ci.M.rde], (i32)ci.M.imm);
    case 0x07: // jalr
        return sprintf(buf, "%s, %d, %d",  reg[ci.M.rde], (i32)ci.M.imm, reg[ci.M.rs1]);
    case 0x08: // ret
        return 0;
    case 0x09: // retr
        return sprintf(buf, "%s", reg[ci.M.rs1]);
    case 0x0a: // branch instructions
        char* opname = "";
        
        switch (ci.B.func){
        case 0x0: // bra
        case 0x1: // beq
        case 0x2: // bez
        case 0x3: // blt
        case 0x4: // ble
        case 0x5: // bltu
        case 0x6: // bleu
        case 0x9: // bne
        case 0xA: // bnz
        case 0xB: // bge
        case 0xC: // bgt
        case 0xD: // bgeu
        case 0xE: // bteu
            break;
        default:
            return -1;
        }
        return sprintf(buf, "%d", (i32)ci.B.imm);
    case 0x0b: // push
        return sprintf(buf, "%s", reg[ci.M.rs1]);
    case 0x0c: // pop
        return sprintf(buf, "%s",  reg[ci.M.rde]);
    case 0x0d: // enter
        return 0;
    case 0x0e: // leave
        return 0;
    case 0x10: // load immediate
        switch (ci.F.func) {
        case 0: // lli
        case 1: // llis
        case 2: // lui
        case 3: // luis
        case 4: // lti
        case 5: // ltis
        case 6: // ltui
        case 7: // ltuis
            break;
        default:
            return -1;
        }
        return sprintf(buf, "%d", (i32)ci.B.imm);
    case 0x11: // lw
    case 0x12: // lh
    case 0x13: // lhs
    case 0x14: // lq
    case 0x15: // lqs
    case 0x16: // lb
    case 0x17: // lbs
        return sprintf(buf, "%s, %s, %d, %s, %u", reg[ci.E.rde], reg[ci.E.rs1], (i32)ci.E.imm, reg[ci.E.rs2], ci.E.func);
    case 0x18: // sw
    case 0x19: // sh
    case 0x1a: // sq
    case 0x1b: // sb
        return sprintf(buf, "%s, %d, %s, %u, %s", reg[ci.E.rs1], (i32)ci.E.imm, reg[ci.E.rs2], ci.E.func, reg[ci.E.rde]);

    case 0x1e: // cmpr
    
    case 0x1f: // cmpi
    
    case 0x20: // addr
    
    case 0x21: // addi
    
    case 0x22: // subr
    
    case 0x23: // subi
    
    case 0x24: // imulr
    
    case 0x25: // imuli
    
    case 0x26: // idivr
    
    case 0x27: // idivi
    
    case 0x28: // umulr
    
    case 0x29: // umuli
    
    case 0x2a: // udivr
    
    case 0x2b: // udivi
    
    case 0x2c: // remr
    
    case 0x2d: // remi
    
    case 0x2e: // modr
    
    case 0x2f: // modi
    
    case 0x30: // andr
    
    case 0x31: // andi
    
    case 0x32: // orr
    
    case 0x33: // ori
    
    case 0x34: // norr
    
    case 0x35: // nori
    
    case 0x36: // xorr
    
    case 0x37: // xori
    
    case 0x38: // shlr
    
    case 0x39: // shli
    
    case 0x3a: // asrr
    
    case 0x3b: // asri
    
    case 0x3c: // lsrr
    
    case 0x3d: // lsri
    
    case 0x3e: // bitr
    
    case 0x3f: // biti
    
    /* Extension F- Floating-Point Operations */
    case 0x40: // fcmp
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x41: // fto
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x42: // ffrom
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x43: // fneg
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x44: // fabs
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x45: // fadd
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x46: // fsub
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x47: // fmul
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x48: // fdiv
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x49: // fma
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x4a: // fsqrt
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x4b: // fmin
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x4c: // fmax
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x4d: // fsat
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    case 0x4e: // fcnv
        switch (ci.E.func) {
        case 0b0001: // .32.16
        case 0b0010: // .64.16
        case 0b0100: // .16.32
        case 0b0110: // .64.32
        case 0b1000: // .16.64
        case 0b1001: // .32.64
        default:
        }
    case 0x4f: // fnan
        switch (ci.E.func) {
        case 0: // .16
        case 1: // .32
        case 2: // .64
        default:
        }
    default:
    }
}