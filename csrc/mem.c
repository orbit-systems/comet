#include <stdint.h>
#include "comet.h"
#pragma once

#define MEM_PAGE_SIZE 0x1000

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
    if (capacity == 0) capacity = 1; // the resize wont work if its 0 because it's multiplication

    memory = (page_map) {0, capacity, NULL};
    memory.pages = (mem_page**) malloc(sizeof(mem_page*) * capacity);
}

void new_page(u64 base) {
    // allocate new page
    mem_page* page = (mem_page*) malloc(sizeof(mem_page));
    page->base = base;

    // append page
    if (memory.len == memory.cap) {
        // resize if needed
        memory.cap *= 2;
        memory.pages = (mem_page**) realloc(memory.pages, sizeof(mem_page*) * memory.cap);
    }
    memory.pages[memory.len] = page;
    memory.len++;

    // sort page into page map
    for (size_t i = memory.len-1; i >= 0; i--) {
        if (i == 0 || memory.pages[i]->base > memory.pages[i-1]->base) {
            break;
        }
        temp_page_ptr = 
    }
    

}

u8  read_u8 (u64 addr) {TODO("memory");return 0;}
u16 read_u16(u64 addr) {TODO("memory");return 0;}
u32 read_u32(u64 addr) {TODO("memory");return 0;}
u64 read_u64(u64 addr) {TODO("memory");return 0;}

void write_u8 (u64 addr, u8  val) {TODO("memory");}
void write_u16(u64 addr, u16 val) {TODO("memory");}
void write_u32(u64 addr, u32 val) {TODO("memory");}
void write_u64(u64 addr, u64 val) {TODO("memory");}

void interrupt(aphelion_cpu_state* cpu, u8 code) {
    if (flag_halt_inv_op && code == 1) {
        cpu->running = false;
        return;
    }
    cpu->registers[r_pc] = read_u64(code*8);
}

u64 align_backwards(u64 ptr, u64 align) {
    u64 p = ptr - align + 1;
    u64 mod = p & (align - 1);
    if (mod != 0) {
        p += align - mod;
    }
    return p;
}