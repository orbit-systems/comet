#include "gpu.h"
#include "../orbit.h"
#include "../comet.h"
#include "../mmu.h"

SDL_Window* gpu_window;
SDL_Renderer* gpu_renderer;
SDL_GLContext* gpu_gl_context;

// u64 gpu_colours[16] = {
// 	0x000000FF, 0x0000AAFF, 0x00AA00FF, 0x00AAAAFF,
// 	0xAA0000FF, 0xAA00AAFF, 0xAA5500FF, 0xAAAAAAFF,
// 	0x555555FF, 0x5555FFFF, 0x55FF55FF, 0x55FFFFFF,
// 	0xFF5555FF, 0xFF55FFFF, 0xFFFF55FF, 0xFFFFFFFF
// };


#define SCREEN_WIDTH 700
#define SCREEN_HEIGHT 700
// #define FONT_BUFF 0xF000
// #define SCREEN_BUFF 0xD000

bool gpu_is_drawing = false;
u64 gpu_framebuf = 0;

void *gpu_thread(void* argvp) {
	gpu_init();
	int running = 1;

	SDL_Event e;

	while (running) {
		if ( SDL_PollEvent(&e) ) {
			switch (e.type) {
			case SDL_QUIT: 
				comet.cpu.running = false;
				break;
			case SDL_TEXTINPUT:
				printf("TEXT: %s\n", e.text.text);
				TODO("text input");

			default: 
				// printf("EVENT\n");
				break;
			}
		} 

		if (comet.cpu.running == false) {
			break;
		}
		

		if (true) {
			// printf("SHITFUCKGARBAGETRUCK\n");

			gpu_draw();

			gpu_is_drawing = false;
		}

		sched_yield();
	}

	comet.cpu.running = false;
}

int gpu_vao;
int gpu_vbo;
int gpu_ebo;
int gpu_texture;
int gpu_program;

void gl_init() {
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	gpu_gl_context = SDL_GL_CreateContext(gpu_window);

	if (gpu_gl_context == NULL) {
		printf("FUCK!: %s\n", SDL_GetError());
	}

	SDL_GL_MakeCurrent(gpu_window, gpu_gl_context);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		printf("GLEW initialisation failure, error: %s\n", glewGetErrorString(err));
		exit(-1);
	}

	glEnable(GL_TEXTURE_2D);

	//create gpu shader

	char* vertex_shader[] = {
			"#version 330 core\n",
			"in vec2 position;\n",
			"in vec2 texcoord;\n",
			"out vec2 Texcoord;\n",
			"void main() {\n",
    		"	gl_Position = vec4(position, 0.0, 1.0);\n",
    		"	Texcoord = texcoord;\n",
			"}\n"};

	char* fragment_shader[] = {
	"#version 330 core\n",
	"in vec2 Texcoord;\n",
	"out vec4 FragColor;\n",
	"uniform sampler2D tex;\n",
	"void main() \n{",
 	"	FragColor = texture(tex, Texcoord);\n",
	"}\n"
	};


	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 8, (const GLchar**)vertex_shader, NULL);
	glCompileShader(vertexShaderID);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
	if (success != GL_TRUE) {
		glGetShaderInfoLog(vertexShaderID, 512, NULL, infoLog);
		*(strchr(infoLog, '\n')) = ' '; //removes the newline opengl shoves in
		printf("VS failure: %s\n", infoLog);
		exit(-1);
	}

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 7, (const GLchar**)fragment_shader, NULL);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
	if (success != GL_TRUE) {
		glGetShaderInfoLog(fragmentShaderID, 512, NULL, infoLog);
		*(strchr(infoLog, '\n')) = ' '; //removes the newline opengl shoves in
		printf("FS failure: %s\n", infoLog);
		exit(-1);
	}


	gpu_program = glCreateProgram();
	glAttachShader(gpu_program, vertexShaderID);
	glAttachShader(gpu_program, fragmentShaderID);
	glLinkProgram(gpu_program);


	glViewport(0,0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	glGenTextures(1, &gpu_texture);
	glBindTexture(GL_TEXTURE_2D, gpu_texture);

	glGenVertexArrays(1, &gpu_vao); 
	glBindVertexArray(gpu_vao);

	glGenBuffers(1, &gpu_vbo); 
	glBindBuffer(GL_ARRAY_BUFFER, gpu_vbo);

	glGenBuffers(1, &gpu_ebo); 

	GLint posAttrib = glGetAttribLocation(gpu_program, "position");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0); 
	glEnableVertexAttribArray(posAttrib);
	
	GLint texAttrib = glGetAttribLocation(gpu_program, "texcoord");
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float))); //here
	glEnableVertexAttribArray(texAttrib);

	glClearColor(0.f, 0.f, 0.f, 1.0f);
	SDL_GL_SetSwapInterval(0);
}

void GPU_receive(u64 data) {


	gpu_framebuf = data;
	gpu_is_drawing = true;

	// (running on the CPU thread) force the GPU thread to run
	sched_yield();
}


void gpu_draw() {

	int screen_buff_word;
	float dot_width = 1; //(float)SCREEN_WIDTH / (float)(80 * 8);
	float dot_height = 1; //(float)SCREEN_HEIGHT / (float)(50 * 16);

	//stream texture data to gpu

	float vertices[16] = {
		-1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 1.0f
	};
	int elements[6] = {
		0, 1, 2,
		1, 3, 2
	};

	/*
	 0--1
	 | /|
	 |/ |
	 2--3
	 */

	SDL_GL_SwapWindow(gpu_window);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(gpu_program);

	glBindTexture(GL_TEXTURE_2D, gpu_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, ((u8*)comet.mmu.memory + gpu_framebuf));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glBindVertexArray(gpu_vao);
	glBindBuffer(GL_ARRAY_BUFFER, gpu_vbo); 
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), vertices, GL_DYNAMIC_DRAW);	

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(int), elements, GL_DYNAMIC_DRAW);
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	return;
	/* VGA text mode stuff that im scared to delete in case i want it later
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		for (int j = 0; j < SCREEN_WIDTH; j++) {
			u8 red, green, blue;
			phys_read_u8(gpu_framebuf + (i * SCREEN_WIDTH + j) * 3 + 0, &red);
			phys_read_u8(gpu_framebuf + (i * SCREEN_WIDTH + j) * 3 + 1, &green);
			phys_read_u8(gpu_framebuf + (i * SCREEN_WIDTH + j) * 3 + 2, &blue);
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
	*/
}

/*
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
*/

void gpu_init() {
	SDL_Init( SDL_INIT_VIDEO );
	gpu_window = SDL_CreateWindow("comet", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
	gpu_renderer = SDL_CreateRenderer(gpu_window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Surface* icon = IMG_Load("src/img/comet_icon.bmp");
	SDL_SetWindowIcon(gpu_window, icon);
	gl_init();
}