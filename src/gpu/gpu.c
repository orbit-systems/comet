#include "gpu.h"
#include "../orbit.h"
#include "../comet.h"
#include "../mmu.h"

SDL_Window* gpu_window;
SDL_Renderer* gpu_renderer;

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500
#define FONT_BUFF 0xF000
#define SCREEN_BUFF 0xD000

u64 gpu_colours[16] = {
	0x000000FF, 0x0000AAFF, 0x00AA00FF, 0x00AAAAFF,
	0xAA0000FF, 0xAA00AAFF, 0xAA5500FF, 0xAAAAAAFF,
	0x555555FF, 0x5555FFFF, 0x55FF55FF, 0x55FFFFFF,
	0xFF5555FF, 0xFF55FFFF, 0xFFFF55FF, 0xFFFFFFFF
};

typedef union {
	u32 colour;
	struct {
		u8 a;
		u8 b;
		u8 g;
		u8 r;
	};
} RGBA;

bool drawGPUBuffer = false;
u64 GPUFrameBuffer = 0;

void *gpuThread(void* argvp) {
	gpu_init();
	int running = 1;

	SDL_Event e;

	while (running) {
		if ( SDL_PollEvent(&e) ) {
			if (e.type == SDL_QUIT)
				break;
		} 
		
		if (comet.cpu.running == false) {
			break;
		}
		

		if (drawGPUBuffer) {
			SDL_SetRenderDrawColor(gpu_renderer, 0, 0, 0, 0xFF);
			SDL_RenderClear(gpu_renderer);

			gpu_draw();

			SDL_RenderPresent(gpu_renderer);
			drawGPUBuffer = false;
		}

		sched_yield();
	}

	comet.cpu.running = 0;

	SDL_DestroyWindow(gpu_window);
	SDL_DestroyRenderer(gpu_renderer);


}

void GPU_receive(u64 data) {

	GPUFrameBuffer = data;
	drawGPUBuffer = true;

	// force the GPU thread to run
	sched_yield();
}

void gpu_draw() {
	int screen_buff_word;
	float dot_width = 1; //(float)SCREEN_WIDTH / (float)(80 * 8);
	float dot_height = 1; //(float)SCREEN_HEIGHT / (float)(50 * 16);

	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		for (int j = 0; j < SCREEN_WIDTH; j++) {
			u8 red, green, blue;
			phys_read_u8(GPUFrameBuffer + (i * SCREEN_WIDTH + j) * 3 + 0, &red);
			phys_read_u8(GPUFrameBuffer + (i * SCREEN_WIDTH + j) * 3 + 1, &green);
			phys_read_u8(GPUFrameBuffer + (i * SCREEN_WIDTH + j) * 3 + 2, &blue);
			SDL_SetRenderDrawColor(gpu_renderer, red, green, blue, 0xFF);
			SDL_RenderDrawPoint(gpu_renderer, j, i);
		}
	}

	return;

	for (int i = 0; i < 50; i++) {
		//printf("\n");
		for (int j = 0; j < 80; j++) {
			phys_read_u16(SCREEN_BUFF + (i * 80 + j) * 2, &screen_buff_word);
			for (int k = 0; k < 16; k++) {
				//printf("%02x", char_slice);
				for (int l = 0; l < 8; l++) {
					gpu_use_colour(screen_buff_word, l, k);
					SDL_RenderFillRect(gpu_renderer, &(SDL_Rect){(j*8 + l) * dot_width, (i*16 + k) * dot_height, dot_width, dot_height});
				}
				//printf("\n");
			}

			//read_u64(0x2000 + character * 16, &font_buff_word)
		}
	}
}

void gpu_use_colour(u16 vga_char, u8 bit, u8 slice) {
	u8 character = vga_char & 0x00FF;
	u8 char_slice = 0;
	u8 attrib = (vga_char & 0xFF00) >> 8;
	int fgColour = attrib & 0b00001111;
	int bgColour = (attrib & 0b01110000) >> 4;
	int blink = (attrib & 0b10000000 ? 1 : 0);
	phys_read_u8(FONT_BUFF + (16*character + (15 - slice)), &char_slice);

	RGBA fgColourE = (RGBA){.colour = gpu_colours[fgColour]};
	RGBA bgColourE = (RGBA){.colour = gpu_colours[bgColour]};

	if (char_slice & (1 << (7 - bit))) {
		//we use fgColour
		SDL_SetRenderDrawColor(gpu_renderer, fgColourE.r, fgColourE.g, fgColourE.b, fgColourE.a);
	} else {
		//we use bgColour
		SDL_SetRenderDrawColor(gpu_renderer, bgColourE.r, bgColourE.g, bgColourE.b, bgColourE.a);
	}

	
}


void gpu_init() {
	gpu_window = SDL_CreateWindow("Aphelion GPU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	gpu_renderer = SDL_CreateRenderer(gpu_window, -1, SDL_RENDERER_ACCELERATED);
}