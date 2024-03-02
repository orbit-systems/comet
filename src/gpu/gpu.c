#include "gpu.h"
#include "../orbit.h"
#include "../comet.h"
#include "../mmu.h"

int isGpuInit = 0;

SDL_Window* gpu_window;
SDL_Renderer* gpu_renderer;
SDL_Surface* gpu_framebuf;
SDL_Texture* gpu_framebuf_tex;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800


void *gpuThread(void* argvp) {
	if (isGpuInit == 0) {
		gpu_init();
	}
	int running = 1;

	SDL_Event e;

	while (running) {
		if ( SDL_PollEvent(&e) ) {
			if (e.type == SDL_QUIT)
				break;
		} 
		
		gpu_draw();

		SDL_RenderClear(gpu_renderer);

		SDL_RenderCopy(gpu_renderer, gpu_framebuf_tex, NULL, NULL);

		SDL_RenderPresent(gpu_renderer);
	}

	comet.cpu.running = 0;


}

void gpu_draw() {
	int screen_buff_word;
	int dot_width = SCREEN_WIDTH / (80 * 8);
	int dot_height = SCREEN_HEIGHT / (50 * 16);
	//printf("\n");


	//do a hex dump of the font area
	for (int i = 0; i < 4096; i++) {
		u8 byte = 0;
		read_u8(0xE000 + i, &byte);
		printf("%02x", byte);
		if (i % (80 * 8) == 0) printf("\n");
	}
	printf("\n");
	return;


	for (int i = 0; i < 50; i++) {
		//printf("\n");
		for (int j = 0; j < 80; j++) {
			//;
			phys_read_u16(0x1000 + (i * 80 + j) * 2, &screen_buff_word);
			u8 character = screen_buff_word & 0xFF;
			//printf("%02x", character);
			printf("\nprinting char: %c\n", character);
			for (int k = 0; k < 16; k++) {
				u8 char_slice = 0;
				phys_read_u8(0x2000 + (character + k), &char_slice);
				for (int l = 0; l < 8; l++) {
					if ((char_slice & 1 << l) == 1) {
						printf("#");
						SDL_FillRect(gpu_framebuf, &(SDL_Rect){(j + l) * dot_width, (i + k) * dot_height, dot_width, dot_height}, 0xFFFFFFFF);
					} else {
						printf("-");
						SDL_FillRect(gpu_framebuf, &(SDL_Rect){(j + l) * dot_width, (i + k) * dot_height, dot_width, dot_height}, 0x000000FF);
					}
				}
				printf("\n");
			}

			//read_u64(0x2000 + character * 16, &font_buff_word)
		}
	}
}


void gpu_init() {
	gpu_window = SDL_CreateWindow("Aphelion GPU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	gpu_renderer = SDL_CreateRenderer(gpu_window, -1, SDL_RENDERER_ACCELERATED);
	gpu_framebuf = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_FillRect(gpu_framebuf, NULL, 0x000000FF);
	gpu_framebuf_tex = SDL_CreateTextureFromSurface(gpu_renderer, gpu_framebuf);
}