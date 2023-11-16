#include "comet.h"

#define MEM_PAGE_SIZE 0x1000
#define PAGE_MAP_GROWTH_FACTOR 2

typedef struct mem_page {
    u8 data[MEM_PAGE_SIZE];
    u64 base;
} mem_page;

typedef struct page_map {
    size_t len; // how many pages exist
    size_t cap; // how many pages can be added before the map must be resized
    mem_page** pages; // array of pointers to memory pages
} page_map;

page_map memory; // global page map

// basically just a dynamic array
void init_page_map(size_t capacity) {
    if (capacity == 0) 
        capacity = 1; // the resize wont work if its 0 because it uses multiplication

    memory = (page_map){0, capacity, NULL};
    memory.pages = (mem_page**) malloc(sizeof(mem_page*) * capacity);
}

void free_page_map() {
    for (size_t i = 0; i < memory.len; i++)
        free(memory.pages[i]);
    
    free(memory.pages);
}

mem_page* new_page(u64 base) {
    // allocate new page
    mem_page* page = (mem_page*) malloc(sizeof(mem_page));
    memset(page->data, 0, MEM_PAGE_SIZE);
    page->base = base;

    // append page
    if (memory.len == memory.cap) {
        // resize if needed
        memory.cap *= PAGE_MAP_GROWTH_FACTOR;
        memory.pages = (mem_page**) realloc(memory.pages, sizeof(mem_page*) * memory.cap);
    }
    memory.pages[memory.len] = page;
    memory.len++;

    // sort page into page map - probably a better way to do this
    for (size_t i = memory.len-1; i >= 0; i--) {
        if (i == 0 || memory.pages[i]->base > memory.pages[i-1]->base) {
            break;
        }
        mem_page* temp_page_ptr = memory.pages[i];
        memory.pages[i] = memory.pages[i-1];
        memory.pages[i-1] = temp_page_ptr;
    }

    return page;
}

int binary_find_page(u64 address) {
    if (memory.len == 0) return -1;
    size_t L = 0;
    size_t R = memory.len - 1;
    while (L <= R) {
        size_t m = (L + R) / 2;
        u64 dist = address - memory.pages[m]->base;
        if (dist < 0) {
            R = m - 1;
        } else if (dist > MEM_PAGE_SIZE) {
            L = m + 1;
        } else return m;
    }
    return -1;
}

bool read_u8 (u64 addr, u8* var) {
    int page = binary_find_page(addr);
    if (page == -1) *var = 0;
    *var = memory.pages[page]->data[addr % MEM_PAGE_SIZE];
    return true;
}

bool read_u16(u64 addr, u16* var) {
    if (addr % sizeof(u16) != 0) return false; // unaligned access
    int page = binary_find_page(addr);
    if (page == -1) *var = 0;
    else *var = ((u16*)(memory.pages[page]->data)) [addr % MEM_PAGE_SIZE / sizeof(u16)];
    return true;
    
}

bool read_u32(u64 addr, u32* var) {
    if (addr % sizeof(u32) != 0) return false; // unaligned access
    int page = binary_find_page(addr);
    if (page == -1) *var = 0;
    else *var = ((u32*)(memory.pages[page]->data)) [addr % MEM_PAGE_SIZE / sizeof(u32)];
    return true;
}

bool read_u64(u64 addr, u64* var) {
    if (addr % sizeof(u64) != 0) return false; // unaligned access
    int page = binary_find_page(addr);
    if (page == -1) *var = 0;
    else *var = ((u64*)(memory.pages[page]->data)) [addr % MEM_PAGE_SIZE / sizeof(u64)];
    return true;
}

bool write_u8(u64 addr, u8 value) {
    int page = binary_find_page(addr); // unaligned access
    if (page == -1) { // page not found - track new page
        mem_page* p = new_page(align_backwards(addr, MEM_PAGE_SIZE));
        p->data[addr % MEM_PAGE_SIZE] = value;
    } else {
        memory.pages[page]->data[addr % MEM_PAGE_SIZE] = value;
    }
    return true;

}
bool write_u16(u64 addr, u16 value) {
    if (addr % sizeof(u16) != 0) return false; // unaligned access
    int page = binary_find_page(addr);
    if (page == -1) { // page not found - track new page
        mem_page* p = new_page(align_backwards(addr, MEM_PAGE_SIZE));
        ((u16*)(p->data)) [addr % MEM_PAGE_SIZE / sizeof(u16)] = value;
    } else
        ((u16*)(memory.pages[page]->data)) [addr % MEM_PAGE_SIZE / sizeof(u16)] = value;
    return true;
}

bool write_u32(u64 addr, u32 value) {
    if (addr % sizeof(u32) != 0) return false; // unaligned access
    int page = binary_find_page(addr);
    if (page == -1) { // page not found - track new page
        mem_page* p = new_page(align_backwards(addr, MEM_PAGE_SIZE));
        ((u32*)(p->data)) [addr % MEM_PAGE_SIZE / sizeof(u32)] = value;
    } else
        ((u32*)(memory.pages[page]->data)) [addr % MEM_PAGE_SIZE / sizeof(u32)] = value;
    return true;
}
bool write_u64(u64 addr, u64 value) {
    if (addr % sizeof(u64) != 0) return false; // unaligned access
    int page = binary_find_page(addr);
    if (page == -1) { // page not found - track new page
        mem_page* p = new_page(align_backwards(addr, MEM_PAGE_SIZE));
        ((u64*)(p->data)) [addr % MEM_PAGE_SIZE / sizeof(u64)] = value;
    } else
        ((u64*)(memory.pages[page]->data)) [addr % MEM_PAGE_SIZE / sizeof(u64)] = value;
    return true;
}

u64 align_backwards(u64 ptr, u64 align) {
    u64 p = ptr - align + 1;
    u64 mod = p & (align - 1);
    if (mod != 0) {
        p += align - mod;
    }
    return p;
}

void load_image(FILE* bin) {
    fseek(bin, 0, SEEK_END);
    long bin_size = ftell(bin);
    fseek(bin, 0, SEEK_SET);

    i64 cursor = 0;
    // load pages in full blocks if possible
    for (; bin_size >= MEM_PAGE_SIZE; bin_size -= MEM_PAGE_SIZE) {
        mem_page* p = new_page(cursor);
        fread(p->data, MEM_PAGE_SIZE, 1, bin);
    }
    // load remaining in partial block
    if (bin_size > 0) {
        mem_page* p = new_page(cursor);
        fread(p->data, bin_size, 1, bin);
    }
}