package comet

// ╭───────╮
// │ comet │ the Aphelion ISA reference emulator
// ╰───────╯
// by kaylatheegg, spsandwichman

import "core:fmt"
import "core:os"
import "core:time"
import "core:strings"
import "core:strconv"

main :: proc() {

    // load arguments
    load_arguments()

    file, readstatus := os.read_entire_file(inpath)
    if (!readstatus) {
        fmt.printf("failed to open file: {}\n", inpath)
        return
    }

    append(&memory, ..file[:])

    overall_timer : time.Stopwatch
    if flag_benchmark {
        time.stopwatch_start(&overall_timer)
    }
    
    loop()
    if flag_benchmark {
        time.stopwatch_stop(&overall_timer)
        duration_s := time.duration_seconds(time.stopwatch_duration(overall_timer))
        duration_ms := time.duration_milliseconds(time.stopwatch_duration(overall_timer))
        cycles_per_sec := f64(cpu_state.cycle) / duration_s
        fmt.printf("overall time : %fs (%fms)\n", duration_s, duration_ms)
        fmt.printf("total cycles : %d\n", cpu_state.cycle)
        fmt.printf("cycles/sec   : %f\n", cycles_per_sec)
    }


}

// init aphelion cpu state
cpu_state : aphelion_cpu_state

loop :: proc() {
    using register_names
    
    cpu_state.registers[pc] = 0xA00 // start at beginning of ram
    cpu_state.running = true

    for (cpu_state.running == true) {
        cpu_state.cycle += 1
        
        raw_ins := read_u32(cpu_state.registers[pc])
        
        cpu_state.registers[st] &= 0x00000000FFFFFFFF
        cpu_state.registers[st] |= u64(raw_ins) << 32

        ins_info := raw_decode(raw_ins)

        // print cpu state every cycle if debug level >= 1
        dbg(1, "\ncycle: %d\npc: 0x%16x st: 0x%16x sp: 0x%16x fp: 0x%16x\n", cpu_state.cycle, cpu_state.registers[pc], cpu_state.registers[st], cpu_state.registers[sp], cpu_state.registers[fp])
        dbg(1, "ra: 0x%16x rb: 0x%16x rc: 0x%16x rd: 0x%16x\nre: 0x%16x rf: 0x%16x rg: 0x%16x rh: 0x%16x\nri: 0x%16x rj: 0x%16x rk: 0x%16x\n", 
            cpu_state.registers[ra], cpu_state.registers[rb], cpu_state.registers[rc], cpu_state.registers[rd],
            cpu_state.registers[re], cpu_state.registers[rf], cpu_state.registers[rg], cpu_state.registers[rh],
            cpu_state.registers[ri], cpu_state.registers[rj], cpu_state.registers[rk])


        //actually do the instruction
        exec_instruction(&cpu_state, ins_info)

        if cpu_state.increment_next {
            cpu_state.registers[pc] += 4
        }

        if flag_cycle_limit != 0 && (cpu_state.cycle >= flag_cycle_limit) {
            cpu_state.running = false
        }
    }

}

load_arguments :: proc() {
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
        case "-max-cycles":
            ok := false
            flag_cycle_limit, ok = strconv.parse_u64(argument.val)
            if !ok {
                die("ERR: expected int, got \"%s\"\n", argument.val)
            }
        case "-no-color":
            flag_no_color = true
        case "-halt-on-inv":
            flag_halt_inv_op = true
        case "-bench":
            flag_benchmark = true
        case: // default
            if index == 0 && argument.key[0] != '-' {
                inpath = os.args[1]
                continue
            }
            die("ERR: invalid argument \"%s\"\n", argument.key)
        }
    }
}

flag_dbg_verbosity  := -1
flag_cycle_limit    : u64 = 0
flag_no_color       := false
flag_halt_inv_op    := false
flag_benchmark      := false
inpath              := ""

cmd_arg :: struct {
    key : string,
    val : string,
}