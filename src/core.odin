package comet

// ╭───────╮
// │ comet │ the Aphelion ISA reference emulator
// ╰───────╯
// by kaylatheegg, spsandwichman

// using aphelion v0.2.1

// TODO - IO slows the emulator down - multithread!!!!!!!!

import "core:fmt"
import "core:os"
import "core:time"
import "core:strings"
import "core:strconv"
import "core:thread"

// init aphelion cpu state
cpu_state := aphelion_cpu_state{}
gpu := gpu_state{}

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
    
    gpu = gpu_init()

    gpu_thread := thread.create(gpu_thread_loop)
    gpu_thread.data = &gpu
    gpu_thread.id = 1
    thread.start(gpu_thread)

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

        //dbg(1, "current memory len 0x%X ", len(memory))
        dbg(1, "cycle %d ", cpu_state.cycle)

        if flag_dbg_verbosity >= 1 {
            set_style(ANSI.FG_Yellow)
            fmt.printf("@ %4x ", cpu_state.registers[register_names.pc])
            set_style(ANSI.FG_Default)
            print_asm(ins_info)
        }

        //actually do the instruction
        exec_instruction(&cpu_state, ins_info)

        if flag_dbg_verbosity >= 2 {
            print_registers(&cpu_state)
        }

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