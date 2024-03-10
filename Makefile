SRCPATHS = src/*.c src/gpu/*.c
SRC = $(wildcard $(SRCPATHS))
OBJECTS = $(SRC:src/%.c=build/%.o)



EXECUTABLE_NAME = comet

CC = gcc
LD = gcc

DEBUGFLAGS = -ggdb -Og
ASANFLAGS = -fsanitize=undefined -fsanitize=address
DONTBEAFUCKINGIDIOT = -Werror -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-unused-result
CFLAGS = -O3 -flto -fno-strict-aliasing
SHUTTHEFUCKUP = -Wno-unknown-warning-option -Wno-incompatible-pointer-types-discards-qualifiers -Wno-initializer-overrides -Wno-discarded-qualifiers
LINK_FLAGS = -lm -flto -lpthread

ifeq ($(OS),Windows_NT)
	EXECUTABLE_NAME = comet.exe
	LINK_FLAGS += -lws2_32 -lmingw32 -L. -lSDL2 -lglew32 -lopengl32 -lWs2_32 -lole32 -lcomctl32 -lgdi32 -lcomdlg32 -lSDL2_image
	CFLAGS += -mconsole
else
	LINK_FLAGS += -lSDL2 -lGL -lGLEW -lSDL2_image
endif

all: build

build/%.o: */%.c
	@echo compiling $<
	@$(CC) -c -o $@ $< $(CFLAGS) -MD

build: $(OBJECTS)
	@echo linking with $(LD)
	@$(CC) $(OBJECTS) -o $(EXECUTABLE_NAME) $(LINK_FLAGS)
	@echo $(EXECUTABLE_NAME) built

test: build
	@echo ""
	./$(EXECUTABLE_NAME) test/example.bin -max-cycles:700000000 -bench

dbgbuild/%.o: */%.c
	@$(CC) -c -o $@ $< -Isrc/ -MD $(DEBUGFLAGS)

dbgbuild: $(OBJECTS)
	@$(LD) $(OBJECTS) -o $(EXECUTABLE_NAME) $(DEBUGFLAGS)

clean:
	@rm -rf build
	@mkdir build
	@mkdir build/gpu

printbuildinfo:
	@echo using $(CC) with flags $(CFLAGS)

new: clean printbuildinfo build

-include $(OBJECTS:.o=.d)
