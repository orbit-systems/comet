package comet

import "core:fmt"
import "core:thread"
import "core:intrinsics"

// i think there is a more efficient way to store this later but i am not smart enough to implement this now
//memory: [dynamic]u8

PAGE_SIZE :: 4096

mem_page :: struct {
    data : [PAGE_SIZE]byte,
    base : u64,         // lowest address reachable by this page
}

page_map : [dynamic]^mem_page

find_page :: proc(address: u64) -> int {
    for p, i in page_map {
        // check if address lies within page
        if p.base <= address && address < (p.base + PAGE_SIZE) {
            return i
        }
    }
    return -1
}

read :: proc($T: typeid, address: u64) -> T {
    if address % size_of(T) != 0 {
        // page fault interrupt
    }

    page := find_page(address)
    
    if page == -1 {
        return 0
    }
    
    return (transmute(^[PAGE_SIZE/size_of(T)]T) &(page_map[page].data))[(address % PAGE_SIZE)/size_of(T)]
}

write :: proc($T: typeid, address: u64, value: T) {
    if address % size_of(T) != 0 {
        //page fault interrupt
    }

    page_index := find_page(address)

    if page_index == -1 {   // page not found - allocate and track new memory page
        newpage := new(mem_page)
        append(&page_map, newpage)
        (transmute(^[PAGE_SIZE/size_of(T)]T) &newpage.data)[(address % PAGE_SIZE)/size_of(T)] = value
    } else {                // page found - use found page
        (transmute(^[PAGE_SIZE/size_of(T)]T) &(page_map[page_index].data))[(address % PAGE_SIZE)/size_of(T)] = value
        
    }
}

read_u8 :: proc(address: u64) -> u8 {
    page := find_page(address)
    
    if page == -1 {
        return 0
    }
    
    return page_map[page].data[address % PAGE_SIZE]
}

read_u16 :: proc(address: u64) -> u16 {
    if address % size_of(u16) != 0 {
        // page fault interrupt
    }

    page := find_page(address)
    
    if page == -1 {
        return 0
    }
    
    return (transmute(^[PAGE_SIZE/size_of(u16)]u16) &(page_map[page].data))[(address % PAGE_SIZE)/size_of(u16)]
}

read_u32 :: proc(address: u64) -> u32 {
    if address % 4 != size_of(u32) {
        //page fault interrupt
    }

    page := find_page(address)
    
    if page == -1 {
        return 0
    }

    return (transmute(^[PAGE_SIZE/size_of(u32)]u32) &(page_map[page].data))[(address % PAGE_SIZE)/size_of(u32)]
}

read_u64 :: proc(address: u64) -> u64 {
    if address % size_of(u64) != 0 {
        //page fault interrupt
    }

    page := find_page(address)
    
    if page == -1 {
        return 0
    }

    return (transmute(^[PAGE_SIZE/size_of(u64)]u64) &(page_map[page].data))[(address % PAGE_SIZE)/size_of(u64)]
}

write_u8 :: proc(address: u64, value: u8) {
    page_index := find_page(address)

    if page_index == -1 {   // page not found - allocate and track new memory page
        newpage := new(mem_page)
        append(&page_map, newpage)
        newpage.data[address % PAGE_SIZE] = value
    } else {                // page found - use found page
        page_map[page_index].data[address % PAGE_SIZE] = value
    }
}

write_u16 :: proc(address: u64, value: u16) {
    if address % size_of(u16) != 0 {
        //page fault interrupt
    }

    page_index := find_page(address)

    if page_index == -1 {   // page not found - allocate and track new memory page
        newpage := new(mem_page)
        append(&page_map, newpage)
        (transmute(^[PAGE_SIZE/size_of(u16)]u16) &newpage.data)[(address % PAGE_SIZE)/size_of(u16)] = value
    } else {                // page found - use found page
        (transmute(^[PAGE_SIZE/size_of(u16)]u16) &(page_map[page_index].data))[(address % PAGE_SIZE)/size_of(u16)] = value
        
    }
}

write_u32 :: proc(address: u64, value: u32) {
    if address % size_of(u32) != 0 {
        //page fault interrupt
    }

    page_index := find_page(address)

    if page_index == -1 {   // page not found - allocate and track new memory page
        newpage := new(mem_page)
        append(&page_map, newpage)
        (transmute(^[PAGE_SIZE/size_of(u32)]u32) &newpage.data)[(address % PAGE_SIZE)/size_of(u32)] = value
    } else {                // page found - use found page
        (transmute(^[PAGE_SIZE/size_of(u32)]u32) &(page_map[page_index].data))[(address % PAGE_SIZE)/size_of(u32)] = value
        
    }
}

write_u64 :: proc(address: u64, value: u64) {
    if address % 2 != size_of(u64) {
        //page fault interrupt
    }

    page_index := find_page(address)

    if page_index == -1 {   // page not found - allocate and track new memory page
        newpage := new(mem_page)
        append(&page_map, newpage)
        (transmute(^[PAGE_SIZE/size_of(u64)]u64) &newpage.data)[(address % PAGE_SIZE)/size_of(u64)] = value
    } else {                // page found - use found page
        (transmute(^[PAGE_SIZE/size_of(u64)]u64) &(page_map[page_index].data))[(address % PAGE_SIZE)/size_of(u64)] = value
        
    }
}