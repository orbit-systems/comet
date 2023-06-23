package comet

import "core:os"
import "core:fmt"

die_exit_code :: 0

die :: proc(msg: string, args: ..any) {
    set_style(ANSI.FG_Red)
    set_style(ANSI.Bold)
    fmt.printf(msg, ..args)
    set_style(ANSI.Reset)
    os.exit(die_exit_code)
}

dbg :: proc(level: int, msg: string, args: ..any) {
    if flag_dbg_verbosity >= level do fmt.printf(msg, ..args)
}

print_help :: proc() {
    fmt.print("\nusage: comet (path) [flags]\n")
    fmt.print("\n-debug:[int]         debug info verbosity")
    fmt.print("\n-keep-dupl-inc       do not filter duplicate file includes")
    fmt.print("\n-halt-on-inv         halt on invalid opcode")
    fmt.print("\n-bench               output benchmarking information after execution is halted")
    fmt.print("\n-help                display this text\n\n")
}

set_style :: proc(code: ANSI) {
    if flag_no_color do return
    fmt.printf("\x1b[%dm", code)
}

ANSI :: enum {

    Reset       = 0,

    Bold        = 1,
    Dim         = 2,
    Italic      = 3,

    FG_Black    = 30,
    FG_Red      = 31,
    FG_Green    = 32,
    FG_Yellow   = 33,
    FG_Blue     = 34,
    FG_Magenta  = 35,
    FG_Cyan     = 36,
    FG_White    = 37,
    FG_Default  = 39,

    BG_Black    = 40,
    BG_Red      = 41,
    BG_Green    = 42,
    BG_Yellow   = 43,
    BG_Blue     = 44,
    BG_Magenta  = 45,
    BG_Cyan     = 46,
    BG_White    = 47,
    BG_Default  = 49,
}