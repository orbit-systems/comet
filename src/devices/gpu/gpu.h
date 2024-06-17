#pragma once
#define GPU_H

#include "orbit.h"

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif

#include <SDL2/SDL.h>
#include "../../GL/glew.h"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>

extern bool gpu_is_drawing;
extern u64  gpu_framebuf;

typedef union {
	u32 colour;
	struct {
		u8 b;
		u8 g;
		u8 r;
	};
} RGB;

void *gpu_thread(void* argvp);

void gpu_init();
void gpu_draw();
void GPU_receive(u64 data);