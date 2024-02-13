#include "comet.h"

// this is much more sensible


bool init_memory() {
    comet.mmu.memory = malloc(MEM_PHYS_MAX + 1);
    return comet.mmu.memory != NULL;
}

void free_memory() {
    free(comet.mmu.memory);
}

mmu_response phys_read_u8 (u64 addr, u8* restrict var) {
    if (addr > MEM_PHYS_MAX) 
        return false; // out of bounds
    *var = comet.mmu.memory[addr];
    return true;
}

mmu_response phys_read_u16(u64 addr, u16* restrict var) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u16) != 0) 
        return false; // out of bounds or unaligned access
    *var = ((u16*)comet.mmu.memory)[addr/sizeof(u16)];
    return true;
}

mmu_response phys_read_u32(u64 addr, u32* restrict var) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u32) != 0) 
        return false; // out of bounds or unaligned access
    *var = ((u32*)comet.mmu.memory)[addr/sizeof(u32)];
    return true;
}

mmu_response phys_read_u64(u64 addr, u64* restrict var) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u64) != 0) 
        return false; // out of bounds or unaligned access
    *var = ((u64*)comet.mmu.memory)[addr/sizeof(u64)];
    return true;
}

mmu_response phys_write_u8(u64 addr, u8 value) {
    if (addr > MEM_PHYS_MAX) 
        return false; // out of bounds
    comet.mmu.memory[addr] = value;
    return true;
}

mmu_response phys_write_u16(u64 addr, u16 value) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u16) != 0) 
        return false; // out of bounds or unaligned access
    comet.mmu.memory[addr/sizeof(u16)] = value;
    return true;
}

mmu_response phys_write_u32(u64 addr, u32 value) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u32) != 0) 
        return false; // out of bounds or unaligned access
    comet.mmu.memory[addr/sizeof(u32)] = value;
    return true;
}
mmu_response phys_write_u64(u64 addr, u64 value) {
    if (addr > MEM_PHYS_MAX || addr % sizeof(u64) != 0) 
        return false; // out of bounds or unaligned access
    comet.mmu.memory[addr/sizeof(u64)] = value;
    return true;
}

u64 align_backwards(u64 ptr, u64 align) {
    u64 p = ptr - align + 1;
    u64 mod = p & (align - 1);
    if (mod != 0)
        p += align - mod;
    return p;
}

bool load_image(FILE* bin) {
    fseek(bin, 0, SEEK_END);
    long bin_size = ftell(bin);
    fseek(bin, 0, SEEK_SET);
    size_t ret_code = fread(comet.mmu.memory, bin_size, 1, bin);
    return !(ret_code != bin_size && ferror(bin));
}

// this is probably bad code but its ok :3
mmu_response translate_address(u64 virtual, u64* restrict physical, access_mode mode) {
    u64 level_1_index = ((0b111111ull << 58) & virtual) >> 58;
    u64 level_2_index = ((0b11111111111ull << 47) & virtual) >> 47;
    u64 level_3_index = ((0b11111111111ull << 36) & virtual) >> 36;
    u64 level_4_index = ((0b11111111111ull << 25) & virtual) >> 25;
    u64 level_5_index = ((0b11111111111ull << 14) & virtual) >> 14;
    u64 level_6_index =   0b11111111111111ull & virtual;

    u64 authoritative_pde = 0;
    u64 pde;
    u64 next;
    bool read_result;

    // get PDE
    read_result = phys_read_u64(comet.mmu.page_table_base + level_1_index * 8, &pde);
    if (!read_result || (pde & 1ull) == 0) {
        return tr_invalid;
    }
    // set authoritative perms
    if ((pde & (1ull << 1)) >> 1 == 1) {
        authoritative_pde = pde;
    }
    // check perms
    if (authoritative_pde == 0) {
        if (((pde & (1ull << 2)) >> 2 == 0 && mode == access_read) ||
            ((pde & (1ull << 3)) >> 3 == 0 && mode == access_write)||
            ((pde & (1ull << 4)) >> 4 == 0 && mode == access_execute)) {
            return tr_noperms;
        }
    } else {
        if (((authoritative_pde & (1ull << 2)) >> 2 == 0 && mode == access_read) ||
            ((authoritative_pde & (1ull << 3)) >> 3 == 0 && mode == access_write)||
            ((authoritative_pde & (1ull << 4)) >> 4 == 0 && mode == access_execute)) {
            return tr_noperms;
        }
    }
    next = 0xFFFFFFFFFFFFC000ull & pde;

    // level two

    // get PDE
    read_result = phys_read_u64(next + level_2_index * 8, &pde);
    if (!read_result || (pde & 1ull) == 0) {
        return tr_invalid;
    }
    // set authoritative perms
    if ((pde & (1ull << 1)) >> 1 == 1) {
        authoritative_pde = pde;
    }
    // check perms
    if (authoritative_pde == 0) {
        if (((pde & (1ull << 2)) >> 2 == 0 && mode == access_read) ||
            ((pde & (1ull << 3)) >> 3 == 0 && mode == access_write)||
            ((pde & (1ull << 4)) >> 4 == 0 && mode == access_execute)) {
            return tr_noperms;
        }
    } else {
        if (((authoritative_pde & (1ull << 2)) >> 2 == 0 && mode == access_read) ||
            ((authoritative_pde & (1ull << 3)) >> 3 == 0 && mode == access_write)||
            ((authoritative_pde & (1ull << 4)) >> 4 == 0 && mode == access_execute)) {
            return tr_noperms;
        }
    }
    next = 0xFFFFFFFFFFFFC000ull & pde;

    // level three

    // get PDE
    read_result = phys_read_u64(next + level_3_index * 8, &pde);
    if (!read_result || (pde & 1ull) == 0) {
        return tr_invalid;
    }
    // set authoritative perms
    if ((pde & (1ull << 1)) >> 1 == 1) {
        authoritative_pde = pde;
    }
    // check perms
    if (authoritative_pde == 0) {
        if (((pde & (1ull << 2)) >> 2 == 0 && mode == access_read) ||
            ((pde & (1ull << 3)) >> 3 == 0 && mode == access_write)||
            ((pde & (1ull << 4)) >> 4 == 0 && mode == access_execute)) {
            return tr_noperms;
        }
    } else {
        if (((authoritative_pde & (1ull << 2)) >> 2 == 0 && mode == access_read) ||
            ((authoritative_pde & (1ull << 3)) >> 3 == 0 && mode == access_write)||
            ((authoritative_pde & (1ull << 4)) >> 4 == 0 && mode == access_execute)) {
            return tr_noperms;
        }
    }
    next = 0xFFFFFFFFFFFFC000ull & pde;

    // level four

    // get PDE
    read_result = phys_read_u64(next + level_4_index * 8, &pde);
    if (!read_result || (pde & 1ull) == 0) {
        return tr_invalid;
    }
    // set authoritative perms
    if ((pde & (1ull << 1)) >> 1 == 1) {
        authoritative_pde = pde;
    }
    // check perms
    if (authoritative_pde == 0) {
        if (((pde & (1ull << 2)) >> 2 == 0 && mode == access_read) ||
            ((pde & (1ull << 3)) >> 3 == 0 && mode == access_write)||
            ((pde & (1ull << 4)) >> 4 == 0 && mode == access_execute)) {
            return tr_noperms;
        }
    } else {
        if (((authoritative_pde & (1ull << 2)) >> 2 == 0 && mode == access_read) ||
            ((authoritative_pde & (1ull << 3)) >> 3 == 0 && mode == access_write)||
            ((authoritative_pde & (1ull << 4)) >> 4 == 0 && mode == access_execute)) {
            return tr_noperms;
        }
    }
    next = 0xFFFFFFFFFFFFC000ull & pde;

    // level five

    // get PDE
    read_result = phys_read_u64(next + level_5_index * 8, &pde);
    if (!read_result || (pde & 1ull) == 0) {
        return tr_invalid;
    }
    // set authoritative perms
    if ((pde & (1ull << 1)) >> 1 == 1) {
        authoritative_pde = pde;
    }
    // check perms
    if (authoritative_pde == 0) {
        if (((pde & (1ull << 2)) >> 2 == 0 && mode == access_read) ||
            ((pde & (1ull << 3)) >> 3 == 0 && mode == access_write)||
            ((pde & (1ull << 4)) >> 4 == 0 && mode == access_execute)) {
            return tr_noperms;
        }
    } else {
        if (((authoritative_pde & (1ull << 2)) >> 2 == 0 && mode == access_read) ||
            ((authoritative_pde & (1ull << 3)) >> 3 == 0 && mode == access_write)||
            ((authoritative_pde & (1ull << 4)) >> 4 == 0 && mode == access_execute)) {
            return tr_noperms;
        }
    }
    next = 0xFFFFFFFFFFFFC000ull & pde;

    read_result = phys_read_u64(next + level_6_index, physical);
    if (!read_result) {
        return tr_invalid;
    }

    return tr_success;
}