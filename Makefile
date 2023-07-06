all: build

BUILD_INPATH = src/
BUILD_OUTPATH = comet

ifeq ($(OS),Windows_NT)
	BUILD_OUTPATH = comet.exe
endif

BUILD_FLAGS = -o:speed -out:$(BUILD_OUTPATH) -no-bounds-check

build:
	@odin build $(BUILD_INPATH) $(BUILD_FLAGS)