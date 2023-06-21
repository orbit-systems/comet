all: build

BUILD_INPATH = src/
BUILD_FLAGS = -o:speed -out:comet

build:
	@odin build $(BUILD_INPATH) $(BUILD_FLAGS)