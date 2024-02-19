SRCPATHS = src/*.c
SRC = $(wildcard $(SRCPATHS))
OBJECTS = $(SRC:src/%.c=build/%.o)



EXECUTABLE_NAME = comet

ifeq ($(OS),Windows_NT)
	EXECUTABLE_NAME = comet.exe
endif

CC = gcc
LD = gcc

DEBUGFLAGS = -g -rdynamic -pg
ASANFLAGS = -fsanitize=undefined -fsanitize=address
DONTBEAFUCKINGIDIOT = -Werror -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-unused-result
CFLAGS = -O3 -Wincompatible-pointer-types
SHUTTHEFUCKUP = -Wno-unknown-warning-option -Wno-incompatible-pointer-types-discards-qualifiers -Wno-initializer-overrides -Wno-discarded-qualifiers

#MD adds a dependency file, .d to the directory. the line at the bottom
#forces make to rebuild, if any dependences need it.
#e.g if comet.h changes, it forces a rebuild
#if core.c changes, it only rebuilds.

all: build

build/%.o: src/%.c
	@echo compiling $<
	@$(CC) -c -o $@ $< $(CFLAGS) -MD $(SHUTTHEFUCKUP)

build: $(OBJECTS)
	@echo linking with $(LD)
	@$(CC) $(OBJECTS) -o $(EXECUTABLE_NAME) $(CFLAGS) -MD
	@echo $(EXECUTABLE_NAME) built

test: build
	@echo ""
	./$(EXECUTABLE_NAME) test/fib.bin -max-cycles:700000000 -bench

debug:
	$(DEBUGFLAGS) $(DONTBEAFUCKINGIDIOT)

clean:
	@rm -rf build
	@mkdir build

printbuildinfo:
	@echo using $(CC) with flags $(CFLAGS)

new: clean printbuildinfo build

-include $(OBJECTS:.o=.d)