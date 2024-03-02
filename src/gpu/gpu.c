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
#define FONT_BUFF 0xF000
#define SCREEN_BUFF 0xD000


emulator comet;

void *gpuThread(void* argvp) {
	if (isGpuInit == 0) {
		gpu_init();
		isGpuInit = 1;
	}
	int running = 1;

	SDL_Event e;

	while (running) {
		if ( SDL_PollEvent(&e) ) {
			if (e.type == SDL_QUIT)
				break;
		} 
		
		

		SDL_SetRenderDrawColor(gpu_renderer, 0, 0, 0, 0xFF);
		SDL_RenderClear(gpu_renderer);

		gpu_draw();

		//SDL_RenderCopy(gpu_renderer, gpu_framebuf_tex, NULL, NULL);

		SDL_RenderPresent(gpu_renderer);
	}

	comet.cpu.running = 0;


}

void gpu_draw() {
	int screen_buff_word;
	float dot_width = 1; //(float)SCREEN_WIDTH / (float)(80 * 8);
	float dot_height = 1; //(float)SCREEN_HEIGHT / (float)(50 * 16);

	for (int i = 0; i < 50; i++) {
		//printf("\n");
		for (int j = 0; j < 80; j++) {
			//;
			phys_read_u16(SCREEN_BUFF + (i * 80 + j) * 2, &screen_buff_word);
			u8 character = screen_buff_word & 0xFF;
			//printf("%02x", character);
			//printf("\nprinting char: %c\n", character);
			for (int k = 0; k < 16; k++) {
				u8 char_slice = 0;
				phys_read_u8(FONT_BUFF + (16*character + (15 - k)), &char_slice);
				//printf("%02x", char_slice);
				for (int l = 0; l < 8; l++) {
					if ((char_slice & (1 << (7-l)))) {
						//printf("#");
						SDL_SetRenderDrawColor(gpu_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
					} else {
						//printf("-");
						SDL_SetRenderDrawColor(gpu_renderer, 0, 0, 0, 0xFF);
					}
					SDL_RenderFillRect(gpu_renderer, &(SDL_Rect){(j*8 + l) * dot_width, (i*16 + k) * dot_height, dot_width, dot_height});
				}
				//printf("\n");
			}

			//read_u64(0x2000 + character * 16, &font_buff_word)
		}
	}
}


void gpu_init() {
	gpu_window = SDL_CreateWindow("Aphelion GPU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	gpu_renderer = SDL_CreateRenderer(gpu_window, -1, SDL_RENDERER_ACCELERATED);
	gpu_framebuf = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	//SDL_FillRect(gpu_framebuf, NULL, 0x000000FF);
	//gpu_framebuf_tex = SDL_CreateTextureFromSurface(gpu_renderer, gpu_framebuf);
}


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

+:

---##--- 18
---##--- 18
---##--- 18
######## FF
######## FF
---##--- 18
---##--- 18
---##--- 18
181818FFFF181818

*/

