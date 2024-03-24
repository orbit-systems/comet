#pragma once
#define GPU_H

#include "../orbit.h"

#include <epoxy/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

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