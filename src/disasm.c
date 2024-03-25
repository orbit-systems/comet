#include "disasm.h"
#include "comet.h"
#include "cpu.h"

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

// returns NULL if invalid
// i KNOW theres a better way to do it
// do NOT @ me on discord to talk about this
// if you wanna do it better, submit a pr or smth lol
char* pnemonic(u32 raw) {
    instruction ci;
    ci.raw = raw;

    switch (ci.opcode) {
    case 0x01: // system control
        switch(ci.F.func) {
        case 0x00: return "int";
        case 0x01: return "iret";
        case 0x02: return "ires";
        case 0x03: return "usr";
        default:
            return NULL;
        }
    case 0x02: return "outr";
    case 0x03: return "outi";
    case 0x04: return "inr";
    case 0x05: return "ini";
    case 0x06: return "jal";
    case 0x07: return "jalr";
    case 0x08: return "ret";
    case 0x09: return "retr";
    case 0x0a: // branch instructions
        switch (ci.B.func){
        case 0x0: return "bra";
        case 0x1: return "beq";
        case 0x2: return "bez";
        case 0x3: return "blt";
        case 0x4: return "ble";
        case 0x5: return "bltu";
        case 0x6: return "bleu";
        case 0x9: return "bne";
        case 0xA: return "bnz";
        case 0xB: return "bge";
        case 0xC: return "bgt";
        case 0xD: return "bgeu";
        case 0xE: return "bteu";
        default:
            return NULL;
        }
    case 0x0b: return "push";
    case 0x0c: return "pop";
    case 0x0d: return "enter";
    case 0x0e: return "leave";
    case 0x10: // load immediate
        switch (ci.F.func) {
        case 0: return "lli";
        case 1: return "llis";
        case 2: return "lui";
        case 3: return "luis";
        case 4: return "lti";
        case 5: return "ltis";
        case 6: return "ltui";
        case 7: return "ltuis";
        default:
            return NULL;
        }
    case 0x11: return "lw";
    case 0x12: return "lh";
    case 0x13: return "lhs";
    case 0x14: return "lq";
    case 0x15: return "lqs";
    case 0x16: return "lb";
    case 0x17: return "lbs";
    case 0x18: return "sw";
    case 0x19: return "sh";
    case 0x1a: return "sq";
    case 0x1b: return "sb";

    // TODO reorder this so that its normal again
    case 0x1e: return "cmpr";
    case 0x1f: return "cmpi";  
    case 0x20: return "addr";  
    case 0x22: return "subr";
    case 0x24: return "imulr";
    case 0x26: return "idivr";
    case 0x28: return "umulr";
    case 0x2a: return "udivr";
    case 0x2c: return "remr";
    case 0x2e: return "modr";
    case 0x21: return "addi";
    case 0x23: return "subi";
    case 0x25: return "imuli";
    case 0x27: return "idivi";
    case 0x29: return "umuli";
    case 0x2b: return "udivi";
    case 0x2d: return "remi";
    case 0x2f: return "modi";
    case 0x30: return "andr";
    case 0x32: return "orr";
    case 0x34: return "norr";
    case 0x36: return "xorr";
    case 0x38: return "shlr";
    case 0x3a: return "asrr";
    case 0x3c: return "lsrr";
    case 0x3e: return "bitr";
    case 0x31: return "andi";
    case 0x33: return "ori";
    case 0x35: return "nori";
    case 0x37: return "xori";
    case 0x39: return "shli";
    case 0x3b: return "asri";
    case 0x3d: return "lsri";
    case 0x3f: return "biti";  
    
    /* Extension F- Floating-Point Operations */
    case 0x40: // fcmp
        switch (ci.E.func) {
        case 0:  return "fcmp.16";
        case 1:  return "fcmp.32";
        case 2:  return "fcmp.64";
        default: return NULL;}
    case 0x41: // fto
        switch (ci.E.func) {
        case 0:  return "fto.16";
        case 1:  return "fto.32";
        case 2:  return "fto.64";
        default: return NULL;}
    case 0x42: // ffrom
        switch (ci.E.func) {
        case 0:  return "ffrom.16";
        case 1:  return "ffrom.32";
        case 2:  return "ffrom.64";
        default: return NULL;}
    case 0x43: // fneg
        switch (ci.E.func) {
        case 0:  return "fneg.16";
        case 1:  return "fneg.32";
        case 2:  return "fneg.64";
        default: return NULL;}
    case 0x44: // fabs
        switch (ci.E.func) {
        case 0:  return "fabs.16";
        case 1:  return "fabs.32";
        case 2:  return "fabs.64";
        default: return NULL;}
    case 0x4a: // fsqrt
        switch (ci.E.func) {
        case 0:  return "fsqrt.16";
        case 1:  return "fsqrt.32";
        case 2:  return "fsqrt.64";
        default: return NULL;}
    case 0x4b: // fmin
        switch (ci.E.func) {
        case 0:  return "fmin.16";
        case 1:  return "fmin.32";
        case 2:  return "fmin.64";
        default: return NULL;}
    case 0x4c: // fmax
        switch (ci.E.func) {
        case 0:  return "fmax.16";
        case 1:  return "fmax.32";
        case 2:  return "fmax.64";
        default: return NULL;}
    case 0x4d: // fsat
        switch (ci.E.func) {
        case 0:  return "fsat.16";
        case 1:  return "fsat.32";
        case 2:  return "fsat.64";
        default: return NULL;}
    case 0x4e: // fcnv
        switch (ci.E.func) {
        case 0b0001: return "fcnv.32.16";
        case 0b0010: return "fcnv.64.16";
        case 0b0100: return "fcnv.16.32";
        case 0b0110: return "fcnv.64.32";
        case 0b1000: return "fcnv.16.64";
        case 0b1001: return "fcnv.32.64";
        default: return NULL;
        }
    case 0x4f: // fnan
        switch (ci.E.func) {
        case 0:  return "fnan.16";
        case 1:  return "fnan.32";
        case 2:  return "fnan.64";
        default: return NULL;}
    case 0x45: // fadd
        switch (ci.E.func) {
        case 0:  return "fadd.16";
        case 1:  return "fadd.32";
        case 2:  return "fadd.64";
        default: return NULL;}
    case 0x46: // fsub
        switch (ci.E.func) {
        case 0:  return "fsub.16";
        case 1:  return "fsub.32";
        case 2:  return "fsub.64";
        default: return NULL;}
    case 0x47: // fmul
        switch (ci.E.func) {
        case 0:  return "fmul.16";
        case 1:  return "fmul.32";
        case 2:  return "fmul.64";
        default: return NULL;}
    case 0x48: // fdiv
        switch (ci.E.func) {
        case 0:  return "fdiv.16";
        case 1:  return "fdiv.32";
        case 2:  return "fdiv.64";
        default: return NULL;}
    case 0x49: // fma
        switch (ci.E.func) {
        case 0:  return "fma.16";
        case 1:  return "fma.32";
        case 2:  return "fma.64";
        default: return NULL;}
    default:
        return NULL;
    }
}

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
        return sprintf(buf, "%s, %ux", reg[ci.M.rs1], ci.M.imm);
    case 0x04: // inr
        return sprintf(buf, "%s, %s",  reg[ci.M.rde], reg[ci.M.rs1]);
    case 0x05: // ini
        return sprintf(buf, "%s, x%ux",  reg[ci.M.rde], ci.M.imm);
    case 0x06: // jal
        return sprintf(buf, "%s, x%dx",  reg[ci.M.rde], (i32)ci.M.imm);
    case 0x07: // jalr
        return sprintf(buf, "%s, %d, x%x",  reg[ci.M.rde], (i32)ci.M.imm, reg[ci.M.rs1]);
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
        return sprintf(buf, "%d", sign_extend(ci.B.imm,20));
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
        return sprintf(buf, "%s, %s, %d, %s, %u", reg[ci.E.rde], reg[ci.E.rs1], (i8)ci.E.imm, reg[ci.E.rs2], ci.E.func);
    case 0x18: // sw
    case 0x19: // sh
    case 0x1a: // sq
    case 0x1b: // sb
        return sprintf(buf, "%s, %d, %s, %u, %s", reg[ci.E.rs1], (i8)ci.E.imm, reg[ci.E.rs2], ci.E.func, reg[ci.E.rde]);

    case 0x1e: // cmpr
        return sprintf(buf, "%s, %s", reg[ci.M.rde], reg[ci.M.rs1]);
    case 0x1f: // cmpi
        if      (ci.F.imm == 1) return sprintf(buf, "%d, %s", (i16)(u16)ci.F.imm, reg[ci.F.rde]);
        else if (ci.F.imm == 0) return sprintf(buf, "%s, %d", reg[ci.F.rde], (i16)(u16)ci.F.imm);
        else return -1;
    
    case 0x20: // addr    
    case 0x22: // subr
    case 0x24: // imulr
    case 0x26: // idivr
    case 0x28: // umulr
    case 0x2a: // udivr
    case 0x2c: // remr
    case 0x2e: // modr
            return sprintf(buf, "%s, %s, %s", reg[ci.R.rde], reg[ci.R.rs1], reg[ci.R.rs2]);
    case 0x21: // addi
    case 0x23: // subi
    case 0x25: // imuli
    case 0x27: // idivi
    case 0x29: // umuli
    case 0x2b: // udivi
    case 0x2d: // remi
    case 0x2f: // modi
        return sprintf(buf, "%s, %s, %d", reg[ci.M.rde], reg[ci.M.rs1], (i32)(i16)(u16)ci.M.imm);
    case 0x30: // andr
    case 0x32: // orr
    case 0x34: // norr
    case 0x36: // xorr
    case 0x38: // shlr
    case 0x3a: // asrr
    case 0x3c: // lsrr
    case 0x3e: // bitr
        return sprintf(buf, "%s, %s, %s", reg[ci.R.rde], reg[ci.R.rs1], reg[ci.R.rs2]);
    case 0x31: // andi
    case 0x33: // ori
    case 0x35: // nori
    case 0x37: // xori
    case 0x39: // shli
    case 0x3b: // asri
    case 0x3d: // lsri
    case 0x3f: // biti
        return sprintf(buf, "%s, %s, %d", reg[ci.M.rde], reg[ci.M.rs1], (u16)ci.M.imm);
    
    
    /* Extension F- Floating-Point Operations */
    case 0x40: // fcmp
    case 0x41: // fto
    case 0x42: // ffrom
    case 0x43: // fneg
    case 0x44: // fabs
    case 0x4a: // fsqrt
    case 0x4b: // fmin
    case 0x4c: // fmax
    case 0x4d: // fsat
    case 0x4e: // fcnv
        // switch (ci.E.func) {
        // case 0b0001: // .32.16
        // case 0b0010: // .64.16
        // case 0b0100: // .16.32
        // case 0b0110: // .64.32
        // case 0b1000: // .16.64
        // case 0b1001: // .32.64
        // default:
        // }
    case 0x4f: // fnan
        return sprintf(buf, "%s, %s", reg[ci.E.rde], reg[ci.E.rs1]);
    case 0x45: // fadd
    case 0x46: // fsub
    case 0x47: // fmul
    case 0x48: // fdiv
    case 0x49: // fma
        return sprintf(buf, "%s, %s, %s", reg[ci.E.rde], reg[ci.E.rs1], reg[ci.E.rs2]);
    default:
        return -1;
    }
}