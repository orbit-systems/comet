package comet

// ╭───────╮
// │ comet │ the Aphelion ISA reference emulator
// ╰───────╯
// by kaylatheegg, spsandwichman

import "core:fmt"
import "core:os"
import "core:strings"
import "core:strconv"

main :: proc() {

    // load arguments
    {
        if len(os.args) < 2 {
            print_help()
            os.exit(0)
        }

        parsed_args : [dynamic]cmd_arg
        defer delete(parsed_args)

        for argument in os.args[1:] {
            split_arg := strings.split(argument, ":")
            if len(split_arg) == 1 {
                append(&parsed_args, cmd_arg{argument, ""})
            } else {
                append(&parsed_args, cmd_arg{split_arg[0], split_arg[1]})
            }
        }

        for argument, index in parsed_args {
            switch argument.key {
            case "-help":
                print_help()
                os.exit(0)
            case "-debug":
                ok := false
                flag_dbg_verbosity, ok = strconv.parse_int(argument.val)
                if !ok {
                    die("ERR: expected int, got \"%s\"\n", argument.val)
                }
            case "-no-color":
                flag_no_color = true
            case: // default
                if index == 0 && argument.key[0] != '-' {
                    inpath = os.args[1]
                    continue
                }
                die("ERR: invalid argument \"%s\"\n", argument.key)
            }
        }
    }

    file, readstatus := os.read_entire_file(inpath)
    if (!readstatus) {
        fmt.printf("failed to open file: {}\n", inpath)
        return
    }

    append(&memory, ..file[:])
    
    loop()
}

// init aphelion cpu state
cpu_state : aphelion_cpu_state

loop :: proc() {
    using register_names
    
    cpu_state.registers[pc] = 0xA00 // start at beginning of ram
    cpu_state.running = true

    for (cpu_state.running == true) {
        if (cpu_state.cycle > 100) {
            cpu_state.running = false
        }

        cpu_state.increment_next = true
        cpu_state.cycle += 1
        
        i_info := raw_decode(read_u32(cpu_state.registers[pc]))

        // print cpu state every cycle if debug level >= 1
        dbg(1, "\ncycle: %d\npc: 0x%8x st: 0x%8x sp: 0x%8x fp: 0x%8x\n", cpu_state.cycle, cpu_state.registers[pc], cpu_state.registers[st], cpu_state.registers[sp], cpu_state.registers[fp])
        dbg(1, "ra: 0x%8x rb: 0x%8x rc: 0x%8x rd: 0x%8x\nre: 0x%8x rf: 0x%8x rg: 0x%8x rh: 0x%8x\nri: 0x%8x rj: 0x%8x rk: 0x%8x\n", 
            cpu_state.registers[ra], cpu_state.registers[rb], cpu_state.registers[rc], cpu_state.registers[rd],
            cpu_state.registers[re], cpu_state.registers[rf], cpu_state.registers[rg], cpu_state.registers[rh],
            cpu_state.registers[ri], cpu_state.registers[rj], cpu_state.registers[rk])


        //actually do the instruction
        exec_instruction(&cpu_state, i_info)

        if cpu_state.increment_next {
            cpu_state.registers[pc] += 4
        }
    }

}

flag_dbg_verbosity  := -1
flag_no_color       := false
inpath              := ""

cmd_arg :: struct {
    key : string,
    val : string,
}