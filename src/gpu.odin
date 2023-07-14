package comet

import sdl2 "vendor:sdl2"
import "core:fmt"
import "core:mem"
import "core:thread"
import "core:intrinsics"

gpu_width    :: 640
gpu_height   :: 480
gpu_channels :: 3

gpu_state :: struct {
    draw_buffer         : [gpu_width*gpu_height*gpu_channels]u8,
    display_buffer      : [gpu_width*gpu_height*gpu_channels]u8,
    dbuf_needs_updating : bool,

    cursor_pos     : [2]u16,
    cursor_color   : [gpu_channels]u8,
    command_buffer : [dynamic]u64,
    mutex          : bool,
}

gpu_thread_proc :: proc(t: ^thread.Thread) {

    main_loop: for {
        if thread.is_done(comet.win_thread) {
            break main_loop
        }
        for did_acquire(&(comet.gpu.mutex)) {
             // change this at some point
            if len(comet.gpu.command_buffer) == 0 {
                comet.gpu.mutex = false
                thread.yield()
                break
            }

            // execute all pending commands
            for len(comet.gpu.command_buffer) != 0 {
                //fmt.printf("%v\n", len(comet.gpu.command_buffer))
                com := comet.gpu.command_buffer[0]
                ordered_remove(&(comet.gpu.command_buffer), 0)
                gpu_process_command(com)
            }
            comet.gpu.mutex = false
        }
        thread.yield()
    }
}

gpu_process_command :: proc(command: u64) {
    //fmt.printf("GPUCOM::%16x\n", command)
    switch u8(0xFF & command) {
    case 3:
        gpu_command_draw_pixel(command)
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        mem.copy(&comet.gpu.display_buffer, &comet.gpu.draw_buffer, gpu_width*gpu_height*3)
        // signal to window thread that the buffer's display surface needs to be updated
        comet.gpu.dbuf_needs_updating = true
    }
}

gpu_command_draw_pixel :: proc(command: u64) {
    x := cast(u64) ((0xFFFF_0000_00_00_00_00 & command) >> 48);
    y := cast(u64) ((0x0000_FFFF_00_00_00_00 & command) >> 32);
    if x >= gpu_width || y >= gpu_height {
        //fmt.printf("GPU: PIXEL DRAW OUT OF BOUNDS (%d, %d)\n", x, y)
        return
    }
    //fmt.printf("GPU: PIXEL DRAW (%d, %d) - LOCATION %d\n", x, y, x * y * gpu_channels)
    r := cast(u8)  ((0x0000_0000_FF_00_00_00 & command) >> 24);
    g := cast(u8)  ((0x0000_0000_00_FF_00_00 & command) >> 16);
    b := cast(u8)  ((0x0000_0000_00_00_FF_00 & command) >> 8);
    // sdl2.SetRenderDrawColor(comet.gpu.render, r, g, b, 255)
    // sdl2.RenderDrawPoint(comet.gpu.render, x, y)
    // @static loc : u32 = 0
    // loc = (x + y * gpu_width) * gpu_channels
    comet.gpu.draw_buffer[(x + y * gpu_width) * gpu_channels    ] = r
    comet.gpu.draw_buffer[(x + y * gpu_width) * gpu_channels + 1] = g
    comet.gpu.draw_buffer[(x + y * gpu_width) * gpu_channels + 2] = b


}

did_acquire :: proc(m: ^bool) -> (acquired: bool) {
	res, ok := intrinsics.atomic_compare_exchange_strong(m, false, true)
	return ok && res == false
}