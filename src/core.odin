package perihelion

import "core:fmt"
import "core:os"

main :: proc() {
	if (len(os.args) != 2) {
		fmt.printf("usage: ./comet pathtoexecutable\n")
		return
	}

	file, readstatus := os.read_entire_file(os.args[1])
	if (!readstatus) {
		fmt.printf("failed to open file: {}\n", os.args[1])
		return
	}

	append(&memory, ..file[:])
	
	loop()
}

cpu_state : state

loop :: proc() {
	using register_names
	

	cpu_state.registers[pc] = 0xA00
	cpu_state.running = true



	//rde : int = s[]
	for (cpu_state.running == true) {
		if (cpu_state.cycle > 30) {
			interrupt(5)
		}

		cpu_state.cycle += 1
		
		i_info := ins_decode(readu32(cpu_state.registers[pc]))
		fmt.printf("cycle: %d\npc: 0x%8x sp: 0x%8x fp: 0x%8x\n", cpu_state.cycle, cpu_state.registers[pc], cpu_state.registers[sp], cpu_state.registers[fp])
		fmt.printf("ra: 0x%8x rb: 0x%8x rc: 0x%8x rd: 0x%8x\nre: 0x%8x rf: 0x%8x rg: 0x%8x rh: 0x%8x\nri: 0x%8x rj: 0x%8x rk: 0x%8x\n\n", 
			cpu_state.registers[ra], cpu_state.registers[rb], cpu_state.registers[rc], cpu_state.registers[rd],
			cpu_state.registers[re], cpu_state.registers[rf], cpu_state.registers[rg], cpu_state.registers[rh],
			cpu_state.registers[ri], cpu_state.registers[rj], cpu_state.registers[rk])

		/*fmt.printf("rde: %s rs1: %s rs2: %s \nimm: 0x%8x opcode: 0x%2x\n", 
		register_name_strings[i_info.rde], register_name_strings[i_info.rs1], 
		register_name_strings[i_info.rs2], i_info.imm, i_info.opcode)*/

		//actually do the instruction
		switch (i_info.opcode) {
			case 0x20: //load immediate family
				switch(i_info.func) {
					case 0: //LLI
						cpu_state.registers[i_info.rde] &= 0xFFFFFFFFFFFF0000
						cpu_state.registers[i_info.rde] |= cast(u64)i_info.imm
				}
			case 0x30: //ADDR
				cpu_state.registers[i_info.rde] = cpu_state.registers[i_info.rs1] + cpu_state.registers[i_info.rs2]
		
			case 0x63: //branching instructions
				switch(i_info.func) {
					case 0: //BRA
						cpu_state.registers[pc] = cast(u64)(cast(i64)cpu_state.registers[pc] + (cast(i64)cpu_state.registers[i_info.imm] - cast(i64)0xFFFFF))
				}
		}

		cpu_state.registers[pc] += 4
	}

}

//decode :: proc(address: u64) -> 

state :: struct {
	registers : [16]u64,
	running : bool,
	cycle: u64,
}

interrupt :: proc(intvalue: u8) {
	if (intvalue == 5) {
		cpu_state.running = false
	}
}

readu32 :: proc(address : u64) -> u32 {
	x : u32 = u32(read(address))
	x += 	  u32(read(address + 1)) << 8
	x += 	  u32(read(address + 2)) << 16
	x += 	  u32(read(address + 3)) << 24
	return x
}

read :: proc(address: u64) -> u8 {
	if (len(memory) <= int(address)) {
		return 0x00
	} else {
		return memory[address]
	}
}

write :: proc(address: u64, value: u8) {
	if (len(memory) < int(address)) {
		extramem := make([]u8, int(address) - len(memory))
		append(&memory, ..extramem[:])
	}

	memory[address] = value
}

memory: [dynamic]u8

register_names :: enum {
	rz,
	ra, rb, rc, rd,
	re, rf, rg, rh,
	ri, rj, rk,
	pc,
	sp, fp,
	st
}

register_name_strings := map[u8]string {
	0  = "rz",  
	1  = "ra",  
	2  = "rb",  
	3  = "rc",
	4  = "rd",
	5  = "re",
	6  = "rf",
	7  = "rg",
	8  = "rh",
	9  = "ri",
	10 = "rj",
	11 = "rk",
	12 = "pc",
	13 = "sp",
	14 = "fp",
	15 = "st"
}

instruction_info :: struct {
    opcode : u8,
    func   : u8,
    rde    : u8,
    rs1    : u8,
    rs2    : u8,
    imm    : u32,
}

ins_decode :: proc(ins: u32) -> (ins_dec: instruction_info) {

    ins_dec.opcode = cast(u8) (ins & 0xFF)
    current_fmt := ins_formats[ins_dec.opcode]

    switch ins_formats[ins_dec.opcode] {
        case instruction_fmt.R:
            ins_dec.rde  = cast(u8) (ins >> 28 & 0xF)
            ins_dec.rs1  = cast(u8) (ins >> 24 & 0xF)
            ins_dec.rs2  = cast(u8) (ins >> 20 & 0xF)
            ins_dec.imm  =          (ins >> 8  & 0xFFF)
        case instruction_fmt.M:
            ins_dec.rde  = cast(u8) (ins >> 28 & 0xF)
            ins_dec.rs1  = cast(u8) (ins >> 24 & 0xF)
            ins_dec.imm  =          (ins >> 8  & 0xFFFF)
        case instruction_fmt.F:
            ins_dec.rde  = cast(u8) (ins >> 28 & 0xF)
            ins_dec.func = cast(u8) (ins >> 24 & 0xF)
            ins_dec.imm  =          (ins >> 8  & 0xFFFF)
        case instruction_fmt.J:
            ins_dec.rde  = cast(u8) (ins >> 28 & 0xF)
            ins_dec.imm  =          (ins >> 8  & 0xFFFFF)
        case instruction_fmt.B:
            ins_dec.func = cast(u8) (ins >> 28 & 0xF)
            ins_dec.imm  =          (ins >> 8  & 0xFFFFF)
    }
    
    return
}

instruction_fmt :: enum {
    R, M, F, J, B,
}

ins_formats := map[u8]instruction_fmt{
    0x0A = instruction_fmt.B,
    0x10 = instruction_fmt.B,
    0x11 = instruction_fmt.M,
    0x12 = instruction_fmt.B,

    0x20 = instruction_fmt.F,
    0x20 = instruction_fmt.F,
    0x20 = instruction_fmt.F,
    0x20 = instruction_fmt.F,
    0x20 = instruction_fmt.F,
    0x20 = instruction_fmt.F,
    0x20 = instruction_fmt.F,
    0x20 = instruction_fmt.F,
    0x21 = instruction_fmt.M,
    0x22 = instruction_fmt.M,
    0x23 = instruction_fmt.M,
    0x24 = instruction_fmt.M,
    0x25 = instruction_fmt.M,
    0x26 = instruction_fmt.M,
    0x26 = instruction_fmt.M,

    0x30 = instruction_fmt.R,
    0x31 = instruction_fmt.M,
    0x32 = instruction_fmt.R,
    0x33 = instruction_fmt.M,
    0x34 = instruction_fmt.R,
    0x35 = instruction_fmt.M,
    0x36 = instruction_fmt.R,
    0x37 = instruction_fmt.M,
    0x38 = instruction_fmt.R,
    0x39 = instruction_fmt.M,
    0x3a = instruction_fmt.R,
    0x3b = instruction_fmt.M,

    0x40 = instruction_fmt.R,
    0x41 = instruction_fmt.M,
    0x42 = instruction_fmt.R,
    0x43 = instruction_fmt.M,
    0x44 = instruction_fmt.R,
    0x45 = instruction_fmt.M,
    0x46 = instruction_fmt.R,
    0x47 = instruction_fmt.M,
    0x48 = instruction_fmt.R,
    0x49 = instruction_fmt.M,
    0x4a = instruction_fmt.R,
    0x4b = instruction_fmt.M,
    0x4c = instruction_fmt.R,
    0x4d = instruction_fmt.M,

    0x50 = instruction_fmt.F,
    0x50 = instruction_fmt.F,
    0x50 = instruction_fmt.F,
    0x50 = instruction_fmt.F,
    0x50 = instruction_fmt.F,
    0x50 = instruction_fmt.F,
    0x50 = instruction_fmt.F,
    0x50 = instruction_fmt.F,

    0x60 = instruction_fmt.M,
    0x61 = instruction_fmt.M,
    0x62 = instruction_fmt.F,
    0x62 = instruction_fmt.F,
    0x64 = instruction_fmt.J,
    0x65 = instruction_fmt.J,

    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
    0x63 = instruction_fmt.B,
}