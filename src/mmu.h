#pragma once
#define MMU_H

#include "orbit.h"

#define MEM_PAGE_SIZE 0x4000
#define MEM_DEFAULT_SIZE 4096*MEM_PAGE_SIZE

typedef u8 access_mode; enum {
    access_translate, // dont care about permissions, just map the address
    access_read,
    access_write,
    access_execute,
};

typedef u8 mmu_response; enum {
    res_success,
    res_accviolation,
    res_noperms,
    res_outofbounds,
    res_unaligned,
};


mmu_response read_instruction(u64 addr, instruction* restrict var);
mmu_response read_u8 (u64 addr, u8*  restrict var);
mmu_response read_u16(u64 addr, u16* restrict var);
mmu_response read_u32(u64 addr, u32* restrict var);
mmu_response read_u64(u64 addr, u64* restrict var);

mmu_response write_u8 (u64 addr, u8  val);
mmu_response write_u16(u64 addr, u16 val);
mmu_response write_u32(u64 addr, u32 val);
mmu_response write_u64(u64 addr, u64 val);


mmu_response phys_read_u8 (u64 addr, u8*  restrict var);
mmu_response phys_read_u16(u64 addr, u16* restrict var);
mmu_response phys_read_u32(u64 addr, u32* restrict var);
mmu_response phys_read_u64(u64 addr, u64* restrict var);

mmu_response phys_write_u8 (u64 addr, u8  value);
mmu_response phys_write_u16(u64 addr, u16 value);
mmu_response phys_write_u32(u64 addr, u32 value);
mmu_response phys_write_u64(u64 addr, u64 value);

mmu_response translate_address(u64 virtual, u64* restrict physical, access_mode mode);
u64 align_backwards(u64 ptr, u64 align);

bool init_MMU();
void destroy_mmu();
bool load_image(FILE* bin);