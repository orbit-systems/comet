package comet

import "core:fmt"

// i think there is a more efficient way to store this later but i am not smart enough to implement this now
memory: [dynamic]u8

read_u64 :: proc(address : u64) -> u64 {
    x : u64 = u64(read_u8(address))
    x += 	  u64(read_u8(address + 1)) << 8
    x += 	  u64(read_u8(address + 2)) << 16
    x += 	  u64(read_u8(address + 3)) << 24
    x += 	  u64(read_u8(address + 4)) << 32
    x += 	  u64(read_u8(address + 5)) << 40
    x += 	  u64(read_u8(address + 6)) << 48
    x += 	  u64(read_u8(address + 7)) << 56
    return x
}

read_u32 :: proc(address : u64) -> u32 {
    x : u32 = u32(read_u8(address))
    x += 	  u32(read_u8(address + 1)) << 8
    x += 	  u32(read_u8(address + 2)) << 16
    x += 	  u32(read_u8(address + 3)) << 24
    return x
}

read_u16 :: proc(address : u64) -> u16 {
    x : u16 = u16(read_u8(address))
    x += 	  u16(read_u8(address + 1)) << 8
    return x
}

read_u8 :: proc(address: u64) -> u8 {
    if (len(memory) <= int(address)) {
        return 0x00
    } else {
        return memory[address]
    }
}

write_u64 :: proc(address: u64, value: u64) {
    write_u8(address,     u8(value))
    write_u8(address + 1, u8(value >> 8))
    write_u8(address + 2, u8(value >> 16))
    write_u8(address + 3, u8(value >> 24))
    write_u8(address + 4, u8(value >> 32))
    write_u8(address + 5, u8(value >> 40))
    write_u8(address + 6, u8(value >> 48))
    write_u8(address + 7, u8(value >> 56))
}

write_u32 :: proc(address: u64, value: u32) {
    write_u8(address,     u8(value))
    write_u8(address + 1, u8(value >> 8))
    write_u8(address + 2, u8(value >> 16))
    write_u8(address + 3, u8(value >> 24))
}

write_u16 :: proc(address: u64, value: u16) {
    write_u8(address,     u8(value))
    write_u8(address + 1, u8(value >> 8))
}

write_u8 :: proc(address: u64, value: u8) {
    if address == 0x9FF {
        if flag_dbg_verbosity > 0 {
            fmt.print("CHAROUT: ")
            fmt.print(rune(value))
            fmt.print("\n")
        } else {
            fmt.print(rune(value))
        }
        return;
    }
    if (len(memory) <= int(address)) {
        // if memory written to has not been allocated, allocate more
        extramem := make([]u8, int(address) - len(memory))
        append(&memory, ..extramem[:])
    }

    memory[address] = value
}