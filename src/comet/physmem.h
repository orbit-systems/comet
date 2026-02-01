#ifndef MEMORY_H
#define MEMORY_H

#include "common/type.h"
#include "common/vec.h"
#include "message.h"

/// Physical memory block
typedef struct {
    /// Pointer to underlying data
    void* data;

    /// Length of physical memory block
    u64 length;

    /// Position in the address map
    u64 address;
} PhysMemBlock;

/// Physical memory unit
typedef struct {
    /// List of physical blocks
    Vec(PhysMemBlock) blocks;
} PhysMemUnit;

/// Create a physical memory unit
PhysMemUnit* physmem_init(void);

/// Create a physical block of memory connected to the memory unit
void physmem_create_block(PhysMemUnit* pmu, u64 size, u64 addr);

/// Read a chunk of memory
SystemMessage physmem_read(PhysMemUnit* pmu, u64 addr, u64 size);

/// Write a chunk of memory
SystemMessage physmem_write(PhysMemUnit* pmu, u64 addr, u64 size, void* buf);

#endif // MEMORY_H