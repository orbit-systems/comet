#pragma once
#define GPU_H

#include "../orbit.h"

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif

#include <SDL2/SDL.h>
#include "../GL/glew.h"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>

void *gpuThread(void* argvp);

void gpu_init();

extern bool drawGPUBuffer;
extern u64 GPUFrameBuffer;