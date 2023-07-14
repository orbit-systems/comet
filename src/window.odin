package comet

import mu "vendor:microui"
import "vendor:sdl2"
import ttf "vendor:sdl2/ttf"
import "core:fmt"
import "core:thread"
import "core:intrinsics"
import "core:strings"
import "core:os"

// TODO use sdl2.UpdateTexture instead of create and destroy texture


state := struct {
    mu_ctx: mu.Context,
    log_buf:         [1<<16]byte,
    log_buf_len:     int,
    log_buf_updated: bool,
    bg: mu.Color,
    
    atlas_texture: ^sdl2.Texture,
}{
    bg = {10, 10, 10, 255},
}

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

    if err := sdl2.Init({.VIDEO}); err != 0 {
        fmt.eprintln(err)
        return
    }
    defer sdl2.Quit()
    ttf.Init()

    // if debug enabled, generate larger resizable window - dont if not
    if flag_debug {
        comet.win.window = sdl2.CreateWindow("comet", sdl2.WINDOWPOS_CENTERED, sdl2.WINDOWPOS_CENTERED, 1500, 700, {.SHOWN, .RESIZABLE})
    } else {
        comet.win.window = sdl2.CreateWindow("comet", sdl2.WINDOWPOS_CENTERED, sdl2.WINDOWPOS_CENTERED, gpu_width, gpu_height, {.SHOWN})
    }
    comet.win.renderer = sdl2.CreateRenderer(comet.win.window, -1, {.PRESENTVSYNC, .ACCELERATED})

    icon := sdl2.LoadBMP("src/assets/comet.bmp")
    sdl2.SetWindowIcon(comet.win.window, icon)

    comet.win.gpu_buf_surface = surface_from_display_buffer()

    state.atlas_texture = sdl2.CreateTexture(comet.win.renderer, u32(sdl2.PixelFormatEnum.RGBA32), .TARGET, mu.DEFAULT_ATLAS_WIDTH, mu.DEFAULT_ATLAS_HEIGHT)
    assert(state.atlas_texture != nil)
    if err := sdl2.SetTextureBlendMode(state.atlas_texture, .BLEND); err != 0 {
        fmt.eprintln("sdl2.SetTextureBlendMode:", err)
        return
    }

    pixels := make([][4]u8, mu.DEFAULT_ATLAS_WIDTH*mu.DEFAULT_ATLAS_HEIGHT)
	for alpha, i in mu.default_atlas_alpha {
		pixels[i].rgb = 0xff
		pixels[i].a   = alpha
	}

    if err := sdl2.UpdateTexture(state.atlas_texture, nil, raw_data(pixels), 4*mu.DEFAULT_ATLAS_WIDTH); err != 0 {
		fmt.eprintln("SDL.UpdateTexture:", err)
		return
	}

    ctx := &state.mu_ctx
    mu.init(ctx)
    
    ctx.text_width = mu.default_atlas_text_width
    ctx.text_height = mu.default_atlas_text_height
    
    jbm : ^ttf.Font = nil
    jbm = ttf.OpenFont("src/assets/JetBrainsMono.ttf", 10)
    if jbm == nil {
        fmt.println("font cannot be loaded")
        return
    }
    defer ttf.CloseFont(jbm)


    main_loop: for {
        e : sdl2.Event
        for (sdl2.PollEvent(&e)) {
            #partial switch e.type {
            case .QUIT: 
                break main_loop
            case .MOUSEMOTION:
                mu.input_mouse_move(ctx, e.motion.x, e.motion.y)
            case .MOUSEWHEEL:
                mu.input_scroll(ctx, e.wheel.x * 30, e.wheel.y * -30)
            case .TEXTINPUT:
                mu.input_text(ctx, string(cstring(&e.text.text[0])))
                
            case .MOUSEBUTTONDOWN, .MOUSEBUTTONUP:
                fn := mu.input_mouse_down if e.type == .MOUSEBUTTONDOWN else mu.input_mouse_up
                switch e.button.button {
                case sdl2.BUTTON_LEFT:   fn(ctx, e.button.x, e.button.y, .LEFT)
                case sdl2.BUTTON_MIDDLE: fn(ctx, e.button.x, e.button.y, .MIDDLE)
                case sdl2.BUTTON_RIGHT:  fn(ctx, e.button.x, e.button.y, .RIGHT)
                }
                
            case .KEYDOWN, .KEYUP:
                if e.type == .KEYUP && e.key.keysym.sym == .ESCAPE {
                    sdl2.PushEvent(&sdl2.Event{type = .QUIT})
                }
                
                fn := mu.input_key_down if e.type == .KEYDOWN else mu.input_key_up
                
                #partial switch e.key.keysym.sym {
                case .LSHIFT:    fn(ctx, .SHIFT)
                case .RSHIFT:    fn(ctx, .SHIFT)
                case .LCTRL:     fn(ctx, .CTRL)
                case .RCTRL:     fn(ctx, .CTRL)
                case .LALT:      fn(ctx, .ALT)
                case .RALT:      fn(ctx, .ALT)
                case .RETURN:    fn(ctx, .RETURN)
                case .KP_ENTER:  fn(ctx, .RETURN)
                case .BACKSPACE: fn(ctx, .BACKSPACE)
                }
            } 
        }

        sdl2.SetRenderDrawColor(comet.win.renderer, 0, 0, 0, 255)
        sdl2.RenderClear(comet.win.renderer)

        // attempt to acquire gpu mutex IF display surface needs updating
        if comet.gpu.dbuf_needs_updating {

            // * just read from the display buffer without aqcuiring the mutex because its probably fine ???
            // * the worst that can happen is, this thread reads in the middle of a gpu_thread memcopy between buffers and the screen tears a bit
            // * which is incredibly unlikely, and worth it for the speedup
            // for !did_acquire(&(comet.gpu.mutex)) {
            //     thread.yield()
            // }

            // repopulate surface
            comet.win.gpu_buf_surface = surface_from_display_buffer()

            // window updated
            // * a little more unsafe but whatever
            comet.gpu.dbuf_needs_updating = false
            // return mutex
            // comet.gpu.mutex = false
        }

        comet.win.gpu_buf_texture = sdl2.CreateTextureFromSurface(
            comet.win.renderer,
            comet.win.gpu_buf_surface,
        )

        if flag_debug {
            mu.begin(ctx)
            all_windows(ctx)
            mu.end(ctx)
        
            render(ctx, comet.win.renderer, jbm)
        } else {
            @static src_rect_no_debug := sdl2.Rect{0, 0, gpu_width, gpu_height}
            @static dst_rect_no_debug := sdl2.Rect{0, 0, gpu_width, gpu_height}

            sdl2.RenderCopy(
                comet.win.renderer,
                comet.win.gpu_buf_texture,
                &src_rect_no_debug,
                &dst_rect_no_debug,
            )

            
        }
        sdl2.RenderPresent(comet.win.renderer)
        sdl2.DestroyTexture(comet.win.gpu_buf_texture)
        thread.yield()
        
    }
}

render :: proc(ctx: ^mu.Context, renderer: ^sdl2.Renderer, jbm: ^ttf.Font) {

    render_texture :: proc(renderer: ^sdl2.Renderer, dst: ^sdl2.Rect, src: mu.Rect, color: mu.Color) {
        dst.w = src.w
        dst.h = src.h
        
        sdl2.SetTextureAlphaMod(state.atlas_texture, color.a)
        sdl2.SetTextureColorMod(state.atlas_texture, color.r, color.g, color.b)
        sdl2.RenderCopy(renderer, state.atlas_texture, &sdl2.Rect{src.x, src.y, src.w, src.h}, dst)
    }
    
    viewport_rect := &sdl2.Rect{}
    sdl2.GetRendererOutputSize(renderer, &viewport_rect.w, &viewport_rect.h)
    sdl2.RenderSetViewport(renderer, viewport_rect)
    sdl2.RenderSetClipRect(renderer, viewport_rect)
    sdl2.SetRenderDrawColor(renderer, state.bg.r, state.bg.g, state.bg.b, state.bg.a)
    sdl2.RenderClear(renderer)
    
    command_backing: ^mu.Command
    for variant in mu.next_command_iterator(ctx, &command_backing) {
        switch cmd in variant {
        case ^mu.Command_Text:
            
            if cmd.str == "\uf00d" {
                src_rect := sdl2.Rect{0, 0, gpu_width, gpu_height}
                dst_rect := sdl2.Rect{cmd.pos.x-2, cmd.pos.y-3, gpu_width, gpu_height}

                sdl2.RenderCopy(
                    comet.win.renderer,
                    comet.win.gpu_buf_texture,
                    &src_rect,
                    &dst_rect,
                )

                continue
            }
            
            // dst1 := sdl2.Rect{cmd.pos.x, cmd.pos.y, 0, 0}
            // for ch in cmd.str do if ch&0xc0 != 0x80 {
            //     r := min(int(ch), 127)
            //     src := mu.default_atlas[mu.DEFAULT_ATLAS_FONT + r]
            //     render_texture(renderer, &dst1, src, cmd.color)
            //     dst1.x += dst1.w
            // }

            text_cstr := strings.clone_to_cstring(cmd.str)
            defer delete(text_cstr)
            text_surface := ttf.RenderText(jbm, text_cstr, {cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a}, {0,0,0,255})
            dst := sdl2.Rect{cmd.pos.x, cmd.pos.y, text_surface.w, text_surface.h}
            src := sdl2.Rect{0, 0, text_surface.w, text_surface.h}
            texture := sdl2.CreateTextureFromSurface(renderer,text_surface)
            sdl2.SetTextureBlendMode(texture, .ADD)
            sdl2.RenderCopy(renderer, texture, &src, &dst)
            sdl2.FreeSurface(text_surface)
            sdl2.DestroyTexture(texture)
            

        case ^mu.Command_Rect:
            sdl2.SetRenderDrawColor(renderer, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a)
            sdl2.RenderFillRect(renderer, &sdl2.Rect{cmd.rect.x, cmd.rect.y, cmd.rect.w, cmd.rect.h})
        case ^mu.Command_Icon:
            src := mu.default_atlas[cmd.id]
            x := cmd.rect.x + (cmd.rect.w - src.w)/2
            y := cmd.rect.y + (cmd.rect.h - src.h)/2
            render_texture(renderer, &sdl2.Rect{x, y, 0, 0}, src, cmd.color)
        case ^mu.Command_Clip:
            sdl2.RenderSetClipRect(renderer, &sdl2.Rect{cmd.rect.x, cmd.rect.y, cmd.rect.w, cmd.rect.h})
        case ^mu.Command_Jump: 
            unreachable()
        }
    }
    // testtext := ttf.RenderText(jbm, "TEXT AAHHGHGH", {255,255,0,255}, {0,0,0,255})
    // testtexts := sdl2.CreateTextureFromSurface(renderer, testtext)
    // sdl2.RenderCopy(renderer, testtexts, &{0, 0, testtext.w, testtext.h}, &{0, 0, testtext.w, testtext.h})
}


u8_slider :: proc(ctx: ^mu.Context, val: ^u8, lo, hi: u8) -> (res: mu.Result_Set) {
    mu.push_id(ctx, uintptr(val))
    
    @static tmp: mu.Real
    tmp = mu.Real(val^)
    res = mu.slider(ctx, &tmp, mu.Real(lo), mu.Real(hi), 0, "%.0f", {.ALIGN_CENTER})
    val^ = u8(tmp)
    mu.pop_id(ctx)
    return
}

write_log :: proc(str: string) {
    // state.log_buf_len += copy(state.log_buf[state.log_buf_len:], str)
    // state.log_buf_len += copy(state.log_buf[state.log_buf_len:], "\n")
    // state.log_buf_updated = true
}

read_log :: proc() -> string {
    return string(state.log_buf[:state.log_buf_len])
}
reset_log :: proc() {
    state.log_buf_updated = true
    state.log_buf_len = 0
}

all_windows :: proc(ctx: ^mu.Context) {
	
	if mu.window(ctx, "registers", {0, 0, 600, 115}, {.NO_CLOSE, .NO_RESIZE, .EXPANDED, .NO_SCROLL}) {
        using register_names
		mu.layout_row(ctx, {600}, 0)
		ctx.style.padding = 3

		text := fmt.aprintf(
            "pc: 0x%16x  st: 0x%16x  sp: 0x%16x  fp: 0x%16x",
            comet.cpu.registers[pc], comet.cpu.registers[st], comet.cpu.registers[sp], comet.cpu.registers[fp],
        )
		mu.text(ctx, text)

        text = fmt.aprintf(
            "ra: 0x%16x  rb: 0x%16x  rc: 0x%16x  rd: 0x%16x",
            comet.cpu.registers[ra], comet.cpu.registers[rb], comet.cpu.registers[rc], comet.cpu.registers[rd],
        )
		mu.text(ctx, text)

        text = fmt.aprintf(
            "re: 0x%16x  rf: 0x%16x  rg: 0x%16x  rh: 0x%16x",
            comet.cpu.registers[re], comet.cpu.registers[rf], comet.cpu.registers[rg], comet.cpu.registers[rh],
        )
		mu.text(ctx, text)

        text = fmt.aprintf(
            "ri: 0x%16x  rj: 0x%16x  rk: 0x%16x",
            comet.cpu.registers[ri], comet.cpu.registers[rj], comet.cpu.registers[rk],
        )
		mu.text(ctx, text)

		delete(text)
		//mu.layout_end_column(ctx)
	}
    

    if mu.window(ctx, "gpu out", {602, 0, gpu_width, gpu_height}, mu.Options{.NO_CLOSE, .NO_RESIZE, .EXPANDED, .NO_FRAME}) {
        mu.layout_begin_column(ctx)

        // * jank that tells the render function to copy the buffer here
		mu.text(ctx, "\uf00d")
		//delete(text)
		mu.layout_end_column(ctx)
	}
}

