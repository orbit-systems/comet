#include "comet.h"

u8* memory;

bool init_memory() {
    memory = malloc(MEM_PHYS_MAX + 1);
    return memory != NULL;
}

void free_memory() {
    free(memory);
}

bool phys_read_u8 (u64 addr, u8* restrict var) {
    if (addr > MEM_PHYS_MAX) 
        return false; // out of bounds
    *var = memory[addr];
    return true;
}

bool phys_read_u16(u64 addr, u16* restrict var) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u16) != 0) 
        return false; // out of bounds or unaligned access
    *var = ((u16*)memory)[addr/sizeof(u16)];
    return true;
}

bool phys_read_u32(u64 addr, u32* restrict var) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u32) != 0) 
        return false; // out of bounds or unaligned access
    *var = ((u32*)memory)[addr/sizeof(u32)];
    return true;
}

bool phys_read_u64(u64 addr, u64* restrict var) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u64) != 0) 
        return false; // out of bounds or unaligned access
    *var = ((u64*)memory)[addr/sizeof(u64)];
    return true;
}

bool phys_write_u8(u64 addr, u8 value) {
    if (addr > MEM_PHYS_MAX) 
        return false; // out of bounds
    memory[addr] = value;
    return true;
}

bool phys_write_u16(u64 addr, u16 value) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u16) != 0) 
        return false; // out of bounds or unaligned access
    memory[addr/sizeof(u16)] = value;
    return true;
}

bool phys_write_u32(u64 addr, u32 value) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u32) != 0) 
        return false; // out of bounds or unaligned access
    memory[addr/sizeof(u32)] = value;
    return true;
}
bool phys_write_u64(u64 addr, u64 value) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u64) != 0) 
        return false; // out of bounds or unaligned access
    memory[addr/sizeof(u64)] = value;
    return true;
}

u64 align_backwards(u64 ptr, u64 align) {
    u64 p = ptr - align + 1;
    u64 mod = p & (align - 1);
    if (mod != 0)
        p += align - mod;
    return p;
}

void load_image(FILE* bin) {
    fseek(bin, 0, SEEK_END);
    long bin_size = ftell(bin);
    fseek(bin, 0, SEEK_SET);

    fread(memory, bin_size, 1, bin);
}