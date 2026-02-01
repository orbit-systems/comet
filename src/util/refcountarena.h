#ifndef REF_COUNTED_ARENA_H
#define REF_COUNTED_ARENA_H

#include <pthread.h>
#include <stddef.h>
#include "common/type.h"

/// Refcounted arena which can defragment when the refcount reaches 0
typedef struct {
    /// Malloc'd data
    void* data;

    /// Length
    u64 length;
     
    /// Maximum capacity of arena
    u64 capacity;

    /// Refcount of arena
    u64 refcount;

    /// Lock for the arena
    pthread_mutex_t arena_lock;
} RefCountedArena;

/// Allocate memory with a global refcounted arena
void* rca_malloc(size_t size);

/// Allocate memory with a custom refcounted arena
void* rca_malloc_arena(RefCountedArena* arena, size_t size);

/// Free memory from a global refcounted arena
void rca_free(void* ptr);

/// Free memory from a custom refcounted arena
void rca_free_arena(RefCountedArena* arena, void* ptr);

/// Set the current global arena
/// NOTE: This operation is NOT thread safe, and could cause problems. FIXME?
void rca_set_global_arena(RefCountedArena* arena);

/// Create a new refcounted arena
RefCountedArena* rca_create(size_t capacity);

#endif // REF_COUNTED_ARENA_H