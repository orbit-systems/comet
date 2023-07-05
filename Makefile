all: build

BUILD_INPATH = src/
BUILD_FLAGS = -o:speed -out:comet -no-bounds-check

build:
	@odin build $(BUILD_INPATH) $(BUILD_FLAGS)