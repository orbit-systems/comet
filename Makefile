SRCPATHS = src/*.c
SRC = $(wildcard $(SRCPATHS))
OBJECTS = $(SRC:src/%.c=build/%.o)



EXECUTABLE_NAME = comet

ifeq ($(OS),Windows_NT)
	EXECUTABLE_NAME = comet.exe
endif

CC = gcc
LD = gcc

DEBUGFLAGS = -g -O0
ASANFLAGS = -fsanitize=undefined -fsanitize=address
DONTBEAFUCKINGIDIOT = -Werror -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-unused-result
CFLAGS = -O3 -Wincompatible-pointer-types -fno-strict-aliasing
SHUTTHEFUCKUP = -Wno-unknown-warning-option -Wno-incompatible-pointer-types-discards-qualifiers -Wno-initializer-overrides -Wno-discarded-qualifiers

all: build

build/%.o: src/%.c
	@echo compiling $<
	@$(CC) -c -o $@ $< $(CFLAGS) -MD

build: $(OBJECTS)
	@echo linking with $(LD)
	@$(CC) $(OBJECTS) -o $(EXECUTABLE_NAME) -lc -lm -MD
	@echo $(EXECUTABLE_NAME) built

test: build
	@echo ""
	./$(EXECUTABLE_NAME) test/example.bin -max-cycles:700000000 -bench

dbgbuild/%.o: src/%.c
	@$(CC) -c -o $@ $< -Isrc/ -MD $(DEBUGFLAGS)

dbgbuild: $(OBJECTS)
	@$(LD) $(OBJECTS) -o $(EXECUTABLE_NAME) $(DEBUGFLAGS)

clean:
	@rm -rf build
	@mkdir build

printbuildinfo:
	@echo using $(CC) with flags $(CFLAGS)

new: clean printbuildinfo build

-include $(OBJECTS:.o=.d)
