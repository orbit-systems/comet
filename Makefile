# common config

BUILD_DIR = build
CORE = ccore

COMET_CORE_SRC_PATHS = \
	src/comet/*.c \
	src/$(CORE)/*.c

COMET_SRC = $(wildcard $(COMET_CORE_SRC_PATHS))
COMET_OBJECTS = $(COMET_SRC:src/%.c=$(BUILD_DIR)/%.o)

CC = gcc
LD = gcc

INCLUDEPATHS = -Isrc/comet -Icommon/include -Isrc/aphelion/ -Isrc/$(CORE)
ASANFLAGS = -fsanitize=undefined -fsanitize=address
CFLAGS = -std=gnu2x -fwrapv -fno-strict-aliasing
WARNINGS = \
	-Wall -Wimplicit-fallthrough -Wmaybe-uninitialized \
	-Wno-override-init -Wno-enum-compare -Wno-unused -Wno-enum-conversion -Wno-discarded-qualifiers -Wno-strict-aliasing

ALLFLAGS = $(CFLAGS) $(WARNINGS) -D$(CORE) -MD
OPT = -g3 -O0

LDFLAGS =


ifneq ($(OS),Windows_NT)
	CFLAGS += -rdynamic
endif

ifdef ASAN_ENABLE
	CFLAGS += $(ASANFLAGS)
	LDFLAGS += $(ASANFLAGS)
endif

# libcommon config
export COMMON_OUT_DIR=../$(BUILD_DIR)

.PHONY: all
all: comet common

$(BUILD_DIR)/%.o: src/%.c
	$(shell echo 1>&2 "Compiling $<")
	@$(CC) -c -o $@ $< -MD $(INCLUDEPATHS) $(ALLFLAGS) $(OPT)

.PHONY: comet
comet: bin/comet
bin/comet: bin/libcommon.a $(COMET_OBJECTS)
	@$(LD) $(LDFLAGS) $(COMET_OBJECTS) -o bin/comet -Lbin -lcommon

bin/libcommon.a:
	$(MAKE) -C common
	cp $(BUILD_DIR)/libcommon.a bin/libcommon.a

.PHONY: clean
clean:
	$(MAKE) -C common clean
	@rm -rf $(BUILD_DIR)/
	@rm -rf bin/
	@mkdir $(BUILD_DIR)/
	@mkdir bin/
	@mkdir -p $(dir $(COMET_OBJECTS))

-include $(COMET_OBJECTS:.o=.d)

# generate compile commands with bear if u got it!!! 
# very good highly recommended ʕ·ᴥ·ʔ
.PHONY: bear-gen-cc
bear-gen-cc: clean
	bear -- $(MAKE) all