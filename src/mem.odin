package comet

import "core:os"
import "core:mem"
import "core:fmt"
import "core:thread"
import "core:intrinsics"

/*

writing this note so people (including me) understand this later.

    (transmute(^[PAGE_SIZE/size_of(T)]T) &(page_map[page_index].data))[(address % PAGE_SIZE)/size_of(T)]

this expression bytecasts a page's data array as an array of another datatype, allowing read/write functions to, 
say, read a u64 from the data array without separating it into multiple read/write_u8 calls.

let's break this down.

    transmute( ^[ PAGE_SIZE/size_of(T) ]T )   &( page_map[page_index].data )

T is the typeid of the value we are trying to get. each page's data has a size PAGE_SIZE. 
'PAGE_SIZE/size_of(T)' returns the length that the array needs to have in order to keep the 
array's overall size the same, so that the transmute() call works. 

' &( page_map[page_index].data ) ' returns the pointer to the page's data array. it is a pointer
so that Odin definitely uses the original page's data intead of creating a new one.

    (address % PAGE_SIZE)/size_of(T)

this returns the index for the transmuted array. 'address % PAGE_SIZE' translates the global address into an
index for the we index the new array correctly. This also indirectly forces aligned access.

*/


PAGE_SIZE :: 4096

mem_page :: struct {
    data : [PAGE_SIZE]u8,
    base : u64, // lowest address reachable by this page
}

page_map : [dynamic]^mem_page

align_backwards :: proc(ptr, align: u64) -> u64 {
    p := ptr - align + 1
    mod := p & (align - 1)
    if mod != 0 {
        p += align - mod
    }
    return p
}

find_page :: proc(address: u64) -> int {
    for p, i in page_map {
        // check if address lies within page
        if p.base <= address && address < (p.base + PAGE_SIZE) {
            return i
        }
    }
    return -1
}

read :: proc($T: typeid, address: u64) -> T where PAGE_SIZE % size_of(T) == 0 {
    when T != u8 { // remove extra logic that isnt needed for byte accesses
        if address % size_of(T) != 0 {
            // unaligned access interrupt
        }
    }

    page_index := find_page(address)
    if page_index == -1 {
        return 0
    }
    
    #no_bounds_check {
        when T != u8 { // remove extra logic that isnt needed for byte accesses
            return (transmute(^[PAGE_SIZE/size_of(T)]T) &(page_map[page_index].data))[(address % PAGE_SIZE)/size_of(T)]
        } else {
            return page_map[page_index].data[address % PAGE_SIZE]
        }
    }
}

write :: proc($T: typeid, address: u64, value: T) where PAGE_SIZE % size_of(T) == 0 {
    when T != u8 { // remove extra logic that isnt needed for byte accesses
        if address % size_of(T) != 0 {
            // unaligned access interrupt
        }
    }

    page_index := find_page(address)
    #no_bounds_check {
        if page_index == -1 { // page not found - allocate and track new memory page
            newpage := new(mem_page)
            newpage.base = align_backwards(address, PAGE_SIZE)
            append(&page_map, newpage)
            when T != u8 { // remove extra logic that isnt needed for byte accesses
                (transmute(^[PAGE_SIZE/size_of(T)]T) &newpage.data)[(address % PAGE_SIZE)/size_of(T)] = value
            } else {
                newpage.data[address % PAGE_SIZE] = value
            }
        } else { // page found - use found page
            when T != u8 { // remove extra logic that isnt needed for byte accesses
                (transmute(^[PAGE_SIZE/size_of(T)]T) &(page_map[page_index].data))[(address % PAGE_SIZE)/size_of(T)] = value
            } else {
                page_map[page_index].data[address % PAGE_SIZE] = value
            }
        }
    }
}

load_ram_image :: proc(file: os.Handle) {
    file_size, _ := os.file_size(file)
    
    // load pages in full blocks
    i : u64 = 0
    for ; file_size >= PAGE_SIZE; file_size -= PAGE_SIZE {
        newpage := new(mem_page)
        newpage.base = i
        os.read(file, newpage.data[:])
        append(&page_map, newpage)
        i += 4096
    }
    // load remaining in partial block
    if file_size > 0 {
        newpage := new(mem_page)
        newpage.base = i
        os.read(file, newpage.data[:file_size])
        append(&page_map, newpage)
    }

}

// * here lies old non-polymorphic functions. im keeping these around if they're needed later for some reason

// read_u8 :: proc(address: u64) -> u8 {
//     page := find_page(address)
    
//     if page == -1 {
//         return 0
//     }
    
//     return page_map[page].data[address % PAGE_SIZE]
// }

// read_u16 :: proc(address: u64) -> u16 {
//     if address % size_of(u16) != 0 {
//         // page fault interrupt
//     }

//     page := find_page(address)
    
//     if page == -1 {
//         return 0
//     }
    
//     return (transmute(^[PAGE_SIZE/size_of(u16)]u16) &(page_map[page].data))[(address % PAGE_SIZE)/size_of(u16)]
// }

// read_u32 :: proc(address: u64) -> u32 {
//     if address % 4 != size_of(u32) {
//         //page fault interrupt
//     }

//     page := find_page(address)
    
//     if page == -1 {
//         return 0
//     }

//     return (transmute(^[PAGE_SIZE/size_of(u32)]u32) &(page_map[page].data))[(address % PAGE_SIZE)/size_of(u32)]
// }

// read_u64 :: proc(address: u64) -> u64 {
//     if address % size_of(u64) != 0 {
//         //page fault interrupt
//     }

//     page := find_page(address)
    
//     if page == -1 {
//         return 0
//     }

//     return (transmute(^[PAGE_SIZE/size_of(u64)]u64) &(page_map[page].data))[(address % PAGE_SIZE)/size_of(u64)]
// }

// write_u8 :: proc(address: u64, value: u8) {
//     page_index := find_page(address)

//     if page_index == -1 {   // page not found - allocate and track new memory page
//         newpage := new(mem_page)
//         append(&page_map, newpage)
//         newpage.data[address % PAGE_SIZE] = value
//     } else {                // page found - use found page
//         page_map[page_index].data[address % PAGE_SIZE] = value
//     }
// }

// write_u16 :: proc(address: u64, value: u16) {
//     if address % size_of(u16) != 0 {
//         //page fault interrupt
//     }

//     page_index := find_page(address)

//     if page_index == -1 {   // page not found - allocate and track new memory page
//         newpage := new(mem_page)
//         append(&page_map, newpage)
//         (transmute(^[PAGE_SIZE/size_of(u16)]u16) &newpage.data)[(address % PAGE_SIZE)/size_of(u16)] = value
//     } else {                // page found - use found page
//         (transmute(^[PAGE_SIZE/size_of(u16)]u16) &(page_map[page_index].data))[(address % PAGE_SIZE)/size_of(u16)] = value
        
//     }
// }

// write_u32 :: proc(address: u64, value: u32) {
//     if address % size_of(u32) != 0 {
//         //page fault interrupt
//     }

//     page_index := find_page(address)

//     if page_index == -1 {   // page not found - allocate and track new memory page
//         newpage := new(mem_page)
//         append(&page_map, newpage)
//         (transmute(^[PAGE_SIZE/size_of(u32)]u32) &newpage.data)[(address % PAGE_SIZE)/size_of(u32)] = value
//     } else {                // page found - use found page
//         (transmute(^[PAGE_SIZE/size_of(u32)]u32) &(page_map[page_index].data))[(address % PAGE_SIZE)/size_of(u32)] = value
        
//     }
// }

// write_u64 :: proc(address: u64, value: u64) {
//     if address % 2 != size_of(u64) {
//         //page fault interrupt
//     }

//     page_index := find_page(address)

//     if page_index == -1 {   // page not found - allocate and track new memory page
//         newpage := new(mem_page)
//         append(&page_map, newpage)
//         (transmute(^[PAGE_SIZE/size_of(u64)]u64) &newpage.data)[(address % PAGE_SIZE)/size_of(u64)] = value
//     } else {                // page found - use found page
//         (transmute(^[PAGE_SIZE/size_of(u64)]u64) &(page_map[page_index].data))[(address % PAGE_SIZE)/size_of(u64)] = value
        
//     }
// }