package comet

// ╭───────╮
// │ comet │ the Aphelion ISA reference emulator
// ╰───────╯
// by kaylatheegg, spsandwichman

// using aphelion v0.2.2

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

    timer : time.Stopwatch,
}

comet : emulator_state

main :: proc() {

    // load arguments
    load_arguments()

    ram_image, readstatus := os.open(inpath)
    if readstatus != os.ERROR_NONE {
        die("Error while opening file \"%s\": OS Error %v\n", inpath, readstatus)
    }
    load_ram_image(ram_image)
    os.close(ram_image)

    if flag_benchmark {
        time.stopwatch_start(&comet.timer)
    }

    comet.win_thread = thread.create(win_thread_proc)
    comet.win_thread.id = 2
    comet.win_thread.init_context = context
    thread.start(comet.win_thread)
    defer thread.destroy(comet.win_thread)

    comet.gpu_thread = thread.create(gpu_thread_proc)
    comet.gpu_thread.id = 1
    comet.gpu_thread.init_context = context
    thread.start(comet.gpu_thread)
    defer thread.destroy(comet.gpu_thread)

    // fucking hilarious - translate slow dynamic map into hard array during initialization
    for key, value in dynamic_map_ins_formats {
        ins_formats[key] = value
    }

    comet.cpu.paused = flag_debug

    loop()

    if !thread.is_done(comet.gpu_thread) {
        thread.terminate(comet.gpu_thread, 0)
    }
    if !thread.is_done(comet.win_thread) {
        thread.terminate(comet.win_thread, 0)
    }
    
    if flag_benchmark {
        time.stopwatch_stop(&comet.timer)
        duration_s := time.duration_seconds(time.stopwatch_duration(comet.timer))
        duration_ms := time.duration_milliseconds(time.stopwatch_duration(comet.timer))
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

        if !comet.cpu.paused {

            do_cpu_cycle()
        
        } else if comet.cpu.step {
            do_cpu_cycle()
            comet.cpu.step = false
        }

        comet.cpu.running = !(flag_cycle_limit != 0 && (comet.cpu.cycle >= flag_cycle_limit))

        if thread.is_done(comet.gpu_thread) {
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