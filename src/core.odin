package comet

// ╭───────╮
// │ comet │ the Aphelion ISA reference emulator
// ╰───────╯
// by kaylatheegg, spsandwichman

// using aphelion v0.2.2

// TODO - IO slows the emulator down - multithread!!!!!!!!

import sdl2 "vendor:sdl2"
import "core:fmt"
import "core:os"
import "core:time"
import "core:strings"
import "core:strconv"
import "core:thread"
import "core:intrinsics"

emulator_state :: struct {
    cpu : aphelion_cpu_state,
    gpu : gpu_state,
    win : out_window_state,
    
    gpu_thread : ^thread.Thread,
    win_thread : ^thread.Thread,
}

comet : emulator_state

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

    comet.gpu_thread = thread.create(gpu_thread_proc)
    comet.gpu_thread.id = 1
    comet.gpu_thread.init_context = context
    thread.start(comet.gpu_thread)
    defer thread.destroy(comet.gpu_thread)

    comet.win_thread = thread.create(win_thread_proc)
    comet.win_thread.id = 2
    comet.win_thread.init_context = context
    thread.start(comet.win_thread)
    defer thread.destroy(comet.win_thread)

    // fucking hilarious - translate slow dynamic map into hard array during initialization
    for key, value in dynamic_map_ins_formats {
        ins_formats[key] = value
    }


    loop()
    if flag_benchmark {
        time.stopwatch_stop(&overall_timer)
        duration_s := time.duration_seconds(time.stopwatch_duration(overall_timer))
        duration_ms := time.duration_milliseconds(time.stopwatch_duration(overall_timer))
        cycles_per_sec := f64(comet.cpu.cycle) / duration_s
        fmt.printf("overall time : %fs (%fms)\n", duration_s, duration_ms)
        fmt.printf("total cycles : %d\n", comet.cpu.cycle)
        fmt.printf("cycles/sec   : %f\n", cycles_per_sec)
    }
}

loop :: proc() {
    using register_names
    
    comet.cpu.registers[pc] = 0xA00 // start at beginning of ram
    comet.cpu.running = true

    for comet.cpu.running {

        comet.cpu.cycle += 1
        
        @static raw_ins : u32 = 0
        raw_ins = read_u32(comet.cpu.registers[pc])
        
        comet.cpu.registers[st] &= 0x00000000FFFFFFFF
        comet.cpu.registers[st] |= u64(raw_ins) << 32

        ins_info := raw_decode(raw_ins)

        //dbg(1, "current memory len 0x%X ", len(memory))
        // dbg(1, "cycle %d ", comet.cpu.cycle)

        // if flag_dbg_verbosity >= 1 {
        //     set_style(ANSI.FG_Yellow)
        //     fmt.printf("@ %4x ", comet.cpu.registers[register_names.pc])
        //     set_style(ANSI.FG_Default)
        //     print_asm(ins_info)
        // }

        //actually do the instruction
        exec_instruction(&comet.cpu, ins_info)

        // if flag_dbg_verbosity >= 2 {
        //     print_registers(&comet.cpu)
        // }

        comet.cpu.registers[pc] += 4 * transmute(u64)(comet.cpu.increment_next)

        comet.cpu.running = !(flag_cycle_limit != 0 && (comet.cpu.cycle >= flag_cycle_limit))

        if thread.is_done(comet.gpu_thread) || thread.is_done(comet.win_thread) {
            //fmt.println("MAIN: DESTROY GPU THREAD")
            return
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
            flag_debug = true
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

flag_debug          := false
flag_cycle_limit    : u64 = 0
flag_no_color       := false
flag_halt_inv_op    := false
flag_benchmark      := false
inpath              := ""

cmd_arg :: struct {
    key : string,
    val : string,
}