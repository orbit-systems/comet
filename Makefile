SRCPATHS = \
	src/*.c \
	src/devices/*.c \

COMPONENTS = \
	gpu \

CC = gcc
LD = gcc

EXECUTABLE_NAME = comet
LIBRARY_FLAGS = -lGL -lSDL2 -lSDL2_image -lGLEW -lm

INCLUDEPATHS = -Isrc/ -Isrc/devices
DEBUGFLAGS = -pg -g 
ASANFLAGS = -fsanitize=undefined -fsanitize=address
OPT = -O2
CFLAGS += -Wincompatible-pointer-types -Wno-discarded-qualifiers -Wno-deprecated-declarations -Wreturn-type
CFLAGS += $(LIBRARY_FLAGS) 


COMPONENTS := $(shell echo $(COMPONENTS) | tr A-Z a-z)
SRCPATHS += $(foreach component, $(COMPONENTS), src/devices/$(component)/*.c) 
CFLAGS += $(foreach component, $(COMPONENTS), -Isrc/devices/$(component))
CFLAGS += $(foreach component, $(COMPONENTS), -D$(component)_component)
SRC = $(wildcard $(SRCPATHS))
OBJECTS = $(SRC:src/%.c=build/%.o)

ECHO = echo

ifeq ($(OS),Windows_NT)
	EXECUTABLE_NAME = $(EXECUTABLE_NAME).exe
else
	ECHO = /usr/bin/echo
	# JANK FIX FOR SANDWICH'S DUMB ECHO ON HIS LINUX MACHINE
endif

FILE_NUM = 0

build/%.o: src/%.c
	$(eval FILE_NUM=$(shell echo $$(($(FILE_NUM)+1))))
	$(shell $(ECHO) 1>&2 -e "\e[0m[\e[32m$(FILE_NUM)/$(words $(SRC))\e[0m]\t Compiling \e[1m$<\e[0m")
	@$(CC) -c -o $@ $< $(INCLUDEPATHS) $(CFLAGS) $(OPT)

build: $(OBJECTS)
	@echo Linking with $(LD)...
	@$(LD) $(OBJECTS) -o $(EXECUTABLE_NAME) $(CFLAGS)
	@echo Successfully built: $(EXECUTABLE_NAME)

debug: CFLAGS += $(DEBUGFLAGS)
debug: OPT = -O0
debug: build

clean:
	@rm -rf build/
	@mkdir build/
	@mkdir -p $(dir $(OBJECTS))

cleanbuild: clean build

-include $(OBJECTS:.o=.d)