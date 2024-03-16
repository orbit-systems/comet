#include "gpu.h"
#include "../orbit.h"
#include "../comet.h"
#include "../mmu.h"

GPU gpu;

void init_GPU() {
    gpu.frame = malloc(SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(pixel));
    memset(gpu.frame, 0, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(pixel));
    
    SDL_Init( SDL_INIT_VIDEO );
    gpu.window = SDL_CreateWindow("comet", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    gpu.renderer = SDL_CreateRenderer(gpu.window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Surface* icon = IMG_Load("src/img/comet_icon.bmp");
    SDL_SetWindowIcon(gpu.window, icon);
    gl_init();
}

void *GPU_thread(void* argvp) {
    
    init_GPU();

    SDL_Event e;

    while (true) {
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
        

        if (gpu.is_drawing) {
            GPU_draw();
            gpu.is_drawing = false;
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

    gpu.gl_ctx = SDL_GL_CreateContext(gpu.window);

    if (gpu.gl_ctx == NULL) {
        printf("FUCK!: %s\n", SDL_GetError());
    }

    SDL_GL_MakeCurrent(gpu.window, gpu.gl_ctx);

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

    // while (comet.gpu.is_drawing) {
    //     sched_yield();
    // }

    gpu.framebuf_addr = data;
    memcpy(gpu.frame, (void*)(comet.mmu.memory + gpu.framebuf_addr), SCREEN_WIDTH*SCREEN_HEIGHT*3);

    memset(gpu.frame, 0xEF, 100);

    gpu.is_drawing = true;
}

const float gpu_surface_vertices[16] = {
   -1.0f,  1.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 1.0f, 0.0f,
   -1.0f, -1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 1.0f
};
const int gpu_surface_elements[6] = {
    0, 1, 2,
    1, 3, 2
};

void GPU_draw() {

    int screen_buff_word;
    float dot_width = 1; //(float)SCREEN_WIDTH / (float)(80 * 8);
    float dot_height = 1; //(float)SCREEN_HEIGHT / (float)(50 * 16);

    //stream texture data to gpu

    /*
     0--1
     | /|
     |/ |
     2--3
     */

    SDL_GL_SwapWindow(gpu.window);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(gpu_program);

    glBindTexture(GL_TEXTURE_2D, gpu_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, gpu.frame);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glBindVertexArray(gpu_vao);
    glBindBuffer(GL_ARRAY_BUFFER, gpu_vbo); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(gpu_surface_vertices), gpu_surface_vertices, GL_DYNAMIC_DRAW);	

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gpu_surface_elements), gpu_surface_elements, GL_DYNAMIC_DRAW);
    
    glDrawElements(GL_TRIANGLES, sizeof(gpu_surface_elements)/sizeof(gpu_surface_elements[0]), GL_UNSIGNED_INT, 0);

    return;
}