package comet

import sdl2 "vendor:sdl2"
import "core:fmt"

gpu_width  :: 640
gpu_height :: 480

gpu :: struct {
    window : ^sdl2.Window,
    render : ^sdl2.Renderer,
    cpos   : [2]u16,
    ccolor : [3]u8,
}

gpu_init :: proc() -> gpu {

    newgpu := gpu{}

    newgpu.window = sdl2.CreateWindow("comet", sdl2.WINDOWPOS_CENTERED, sdl2.WINDOWPOS_CENTERED, gpu_width, gpu_height, sdl2.WINDOW_SHOWN)
    newgpu.render = sdl2.CreateRenderer(newgpu.window, -1, sdl2.RENDERER_SOFTWARE)

    return newgpu
}

gpu_process_command :: proc(gpu: gpu, command: u64) {
    switch u8(0xFF & command) {
    case 3:
        gpu_command_draw_pixel(gpu, command)
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        //fmt.printf("RENDER\n")
        sdl2.RenderPresent(gpu.render)
    }
}

gpu_command_draw_pixel :: proc(gpu: gpu, command: u64) {
    x := cast(i32) ((0xFFFF_0000_00_00_00_00 & command) >> 48);
    y := cast(i32) ((0x0000_FFFF_00_00_00_00 & command) >> 32);
    r := cast(u8)  ((0x0000_0000_FF_00_00_00 & command) >> 24);
    g := cast(u8)  ((0x0000_0000_00_FF_00_00 & command) >> 16);
    b := cast(u8)  ((0x0000_0000_00_00_FF_00 & command) >> 8);
    //fmt.printf("DRAW X: %d Y: %d\t RGB: %02x%02x%02x\n", x, y, r, g, b)
    sdl2.SetRenderDrawColor(gpu.render, r, g, b, 255)
    sdl2.RenderDrawPoint(gpu.render, x, y)

}