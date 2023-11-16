SRCPATHS = src/*.c
SRC = $(wildcard $(SRCPATHS))
OBJECTS = $(SRC:src/%.c=build/%.o)



EXECUTABLE_NAME = comet

ifeq ($(OS),Windows_NT)
	EXECUTABLE_NAME = comet.exe
endif

CC = clang

DEBUGFLAGS = -g -rdynamic -pg
ASANFLAGS = -fsanitize=undefined -fsanitize=address
DONTBEAFUCKINGIDIOT = -Werror -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-unused-result
CFLAGS = -O3 # shut the fuck up clang
SHUTTHEFUCKUP = -Wno-incompatible-pointer-types-discards-qualifiers -Wno-initializer-overrides

#MD adds a dependency file, .d to the directory. the line at the bottom
#forces make to rebuild, if any dependences need it.
#e.g if comet.h changes, it forces a rebuild
#if core.c changes, it only rebuilds.

build/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS) -MD $(SHUTTHEFUCKUP)

build: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE_NAME) $(CFLAGS) -MD

test: build
	@echo ""
	./$(EXECUTABLE_NAME) test/fib.bin -max-cycles:700000000 -bench

debug:
	$(DEBUGFLAGS) $(DONTBEAFUCKINGIDIOT)

test_gpu: build
	@./$(EXECUTABLE_NAME) test/gputest.bin -debug

clean:
	rm -f build/*

-include $(OBJECTS:.o=.d)