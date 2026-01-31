#ifndef MEMORY_H
#define MEMORY_H

#include "common/type.h"
#include "common/vec.h"

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

#endif // MEMORY_H