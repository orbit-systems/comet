all: build

BUILD_INPATH = src/
BUILD_OUTPATH = comet

ifeq ($(OS),Windows_NT)
	BUILD_OUTPATH = comet.exe
endif

BUILD_FLAGS = -o:speed -out:$(BUILD_OUTPATH) -no-bounds-check

build:
	@odin build $(BUILD_INPATH) $(BUILD_FLAGS)

debug:
	@odin build $(BUILD_INPATH) -out:$(BUILD_OUTPATH) -no-bounds-check -debug -o:none

test: build
	@comet test/gputest.bin -debug

c_build:
	@clang csrc/core.c -o cometc -O3

c_test: build
	@cometc.exe test/fib.bin