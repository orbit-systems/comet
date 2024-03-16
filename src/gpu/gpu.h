#pragma once
#define GPU_H

#include "../orbit.h"
#include <pthread.h>

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif

#include <SDL2/SDL.h>
#include "../GL/glew.h"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

typedef struct {
    u8 r;
    u8 g;
    u8 b;
} pixel;

typedef struct GPU_s {
    SDL_Window*    window;
    SDL_Renderer*  renderer;
    SDL_GLContext* gl_ctx;

    u64    framebuf_addr;
    pixel* frame;
    bool   is_drawing;
} GPU;

void *GPU_thread(void* argvp);

void gl_init();
void init_GPU();
void GPU_draw();
void GPU_receive(u64 data);