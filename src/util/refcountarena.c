#include "comet.h"
#include "common/type.h"
#include "refcountarena.h"
#include "lock.h"

#include <inttypes.h>
#include <stdlib.h>

static RefCountedArena* global_arena;

RefCountedArena* rca_create(size_t capacity) {
    RefCountedArena* new_arena = malloc(sizeof(*new_arena));
    new_arena->data = malloc(capacity);
    new_arena->length = 0;
    new_arena->capacity = capacity;
    pthread_mutex_init(&new_arena->arena_lock, NULL);
    new_arena->refcount = 0;
    return new_arena;
}

void* rca_malloc(size_t size) {
    return rca_malloc_arena(NULL, size);
}

void* rca_malloc_arena(RefCountedArena* arena, size_t size) {
    if (arena == NULL) {
        /* Use global arena, if it exists*/
        if (global_arena == NULL) {
            WPRINTF("Attempted to allocate memory with no arena set!");
            return NULL;
        }
        arena = global_arena;
    }
    comet_lock(&arena->arena_lock);

    /* Ensure size is a multiple of 8 */
    size += 8 - size % 8;
    
    if (arena->length + size >= arena->capacity) {
        WPRINTF("Attempted to allocate memory in a full arena! Capacity: 0x%"PRIx64" Length: 0x%"PRIx64" Size: 0x%"PRIx64"\n", arena->capacity, arena->length, (u64)size);
        comet_unlock(&arena->arena_lock);
        return NULL;
    }

    void* ret_ptr = arena->data + arena->length;

    arena->length += size;
    arena->refcount++;

    comet_unlock(&arena->arena_lock);
    return ret_ptr;
}

void rca_free(void* ptr) {
    rca_free_arena(NULL, ptr);
}

void rca_free_arena(RefCountedArena* arena, void* ptr) {
    if (arena == NULL) {
        /* Use global arena, if it exists*/
        if (global_arena == NULL) {
            WPRINTF("Attempted to free memory with no arena set!");
            comet_unlock(&arena->arena_lock);
            return;
        }
        arena = global_arena;
    }
    comet_lock(&arena->arena_lock);

    /* Decrease the refcount */
    arena->refcount--;

    if (arena->refcount == 0) {
        /* We can reset the arena! */
        arena->length = 0;
    }

    comet_unlock(&arena->arena_lock);
    return;
}

void rca_set_global_arena(RefCountedArena* arena) {
    global_arena = arena;
}