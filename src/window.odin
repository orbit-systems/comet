package comet

import sdl2 "vendor:sdl2"
import "core:fmt"
import "core:thread"
import "core:intrinsics"

out_window_state :: struct {
    window          : ^sdl2.Window,
    renderer        : ^sdl2.Renderer,
    
    gpu_buf_surface : ^sdl2.Surface,
    gpu_buf_texture : ^sdl2.Texture,
}

surface_from_display_buffer :: #force_inline proc() -> ^sdl2.Surface {
    return sdl2.CreateRGBSurfaceFrom(
        &comet.gpu.display_buffer,
        gpu_width, 
        gpu_height,
        gpu_channels * 8,
        gpu_width * gpu_channels,
        0x0000FF,
        0x00FF00,
        0xFF0000,
        0,
    )
}

win_thread_proc :: proc(t: ^thread.Thread) {
    comet.win.window = sdl2.CreateWindow("comet", sdl2.WINDOWPOS_CENTERED, sdl2.WINDOWPOS_CENTERED, gpu_width, gpu_height, sdl2.WINDOW_SHOWN)
    comet.win.renderer = sdl2.CreateRenderer(comet.win.window, -1, sdl2.RENDERER_SOFTWARE)

    icon := sdl2.LoadBMP("src/img/comet.bmp")
    sdl2.SetWindowIcon(comet.win.window, icon)

    comet.win.gpu_buf_surface = surface_from_display_buffer()

    for {
        event : sdl2.Event
        for (sdl2.PollEvent(&event)) {
            //fmt.printf("%v\n", event.type)
            #partial switch event.type {
            case .QUIT:
                return
            case .KEYDOWN:
                if event.key.keysym.sym == .ESCAPE {
                    return
                }
            case .WINDOWEVENT:

            case:
                continue
            }   
        }

        // attempt to acquire gpu mutex IF display surface needs updating
        if comet.gpu.dbuf_needs_updating {

            // wait to acquire gpu mutex
            for !did_acquire(&(comet.gpu.mutex)) {
                thread.yield()
            }

            // repopulate surface and textures
            comet.win.gpu_buf_surface = surface_from_display_buffer()
            comet.win.gpu_buf_texture = sdl2.CreateTextureFromSurface(
                comet.win.renderer,
                comet.win.gpu_buf_surface,
            )

            // window updated
            comet.gpu.dbuf_needs_updating = false
            // return mutex
            comet.gpu.mutex = false
        }

        @static src_rect := sdl2.Rect{0, 0, gpu_width, gpu_height}
        @static dst_rect := sdl2.Rect{}

        // TODO recalculate position based on position of corresponding microui window
        dst_rect = sdl2.Rect{0, 0, gpu_width, gpu_height}

        sdl2.RenderCopy(
            comet.win.renderer,
            comet.win.gpu_buf_texture,
            &src_rect,
            &dst_rect,
        )

        sdl2.RenderPresent(comet.win.renderer)
        
    }

    
}