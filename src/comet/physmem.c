#include "physmem.h"
#include "message.h"

PhysMemUnit* physmem_init(void) {
    PhysMemUnit* new_physmem = malloc(sizeof(*new_physmem));
    new_physmem->blocks = vec_new(PhysMemBlock, 1);
    return new_physmem;
}

void physmem_create_block(PhysMemUnit* pmu, u64 size, u64 addr) {
    PhysMemBlock new_block;
    new_block.address = addr;
    new_block.length = size;
    new_block.data = malloc(size);
    vec_append(&pmu->blocks, new_block); 
}

PhysMemBlock* physmem_find_block(PhysMemUnit* pmu, u64 addr) {
    for (size_t i = 0; i < vec_len(pmu); i++) {
        PhysMemBlock* block = &pmu->blocks[i];
        if (block->address <= addr && addr <= block->address + block->length)
            return block;
    }
    return NULL;
}

SystemMessage physmem_read(PhysMemUnit* pmu, u64 addr, u64 size) {
    /* Read size bytes from addr */
    PhysMemBlock* block = physmem_find_block(pmu, addr);
    if (block == NULL) 
        return CREATE_MESSAGE(MSG_SYS_LOAD_BAD_ADDR, addr, 8);
    
    if (addr + size > block->address + block->length)
        return CREATE_MESSAGE(MSG_SYS_LOAD_BAD_ADDR, addr, 8);

    /* Convert address to local block address */
    u64 local_addr = addr - block->address;

    return CREATE_MESSAGE_FROM_BUFF(MSG_SYS_LOAD_OK, block->data + local_addr, size);
}

SystemMessage physmem_write(PhysMemUnit* pmu, u64 addr, u64 size, void* buf) {
    /* Read size bytes from addr */
    PhysMemBlock* block = physmem_find_block(pmu, addr);
    if (block == NULL) 
        return CREATE_MESSAGE(MSG_SYS_STORE_BAD_ADDR, NULL, 0);
    
    if (addr + size > block->address + block->length)
        return CREATE_MESSAGE(MSG_SYS_STORE_BAD_ADDR, NULL, 0);

    /* Convert address to local block address */
    u64 local_addr = addr - block->address;
    memcpy(block->data + local_addr, buf, size);

    return CREATE_MESSAGE(MSG_SYS_STORE_OK, NULL, 0);
}