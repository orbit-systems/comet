SRCPATHS = csrc/*.c
CSRC = $(wildcard $(SRCPATHS))
OBJECTS = $(CSRC:.c=.o)

EXECUTABLE_NAME = comet

ifeq ($(OS),Windows_NT)
	EXECUTABLE_NAME = comet.exe
endif

CC = clang

DEBUGFLAGS = -g -rdynamic -pg
ASANFLAGS = -fsanitize=undefined -fsanitize=address
DONTBEAFUCKINGIDIOT = -Werror -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-unused-result


#MD adds a dependency file, .d to the directory. the line at the bottom
#forces make to rebuild, if any dependences need it.
#e.g if comet.h changes, it forces a rebuild
#if core.c changes, it only rebuilds.

%.o: %.c
	$(CC) -c -o $@ $< -O3 -MD

build: $(OBJECTS)
	@$(CC) $(OBJECTS) -o $(EXECUTABLE_NAME) -O3 -MD

test: build
	@./$(EXECUTABLE_NAME) test/fib.bin -max-cycles:300000000 -bench

debug:
	$(DEBUGFLAGS) $(DONTBEAFUCKINGIDIOT)

test_gpu: build
	@./$(EXECUTABLE_NAME) test/gputest.bin -debug

clean:
	rm -f csrc/*.o

-include $(OBJECTS:.o=.d)