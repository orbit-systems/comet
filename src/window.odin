package comet

import sdl2 "vendor:sdl2"
import "core:fmt"
import "core:thread"
import "core:intrinsics"

out_window_state :: struct {
    window          : ^sdl2.Window,
    render          : ^sdl2.Renderer,
}

win_thread_proc :: proc(t: ^thread.Thread) {
    comet.win.window = sdl2.CreateWindow("comet", sdl2.WINDOWPOS_CENTERED, sdl2.WINDOWPOS_CENTERED, gpu_width, gpu_height, sdl2.WINDOW_SHOWN)
    comet.win.render = sdl2.CreateRenderer(comet.win.window, -1, sdl2.RENDERER_SOFTWARE)

    icon := sdl2.LoadBMP("src/img/comet.bmp")
    sdl2.SetWindowIcon(comet.win.window, icon)

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
                // fmt.printf("%v\n", event.window.event)
                // if event.window.event == .LEAVE {
                //     intgpu.global_pause = true
                //     thread.yield()
                //     continue
                // }
                // if event.window.event == .ENTER {
                //     intgpu.global_pause = false
                // }
            case:
                continue
            }   
        }

        // attempt to acquire gpu mutex IF display surface needs updating
        if comet.gpu.dbuf_updated {

            // wait to acquire gpu mutex
            for !did_acquire(&(comet.gpu.mutex)) {
                thread.yield()
            }

            comet.gpu.dbuf_updated = false
            comet.gpu.mutex = false
        }

        sdl2.RenderPresent(comet.win.render)
        
    }

    
}