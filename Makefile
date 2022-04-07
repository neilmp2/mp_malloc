# not using standard Makefile template because this makefile creates shared
# objects and weird stuff
CC = gcc
WARNINGS = -Wall -Wno-unused-result -Wno-unused-function #-Wextra -Werror -Wno-error=unused-parameter -Wmissing-declarations
CFLAGS_COMMON = $(WARNINGS) -std=c99 -D_GNU_SOURCE -lm
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O3
CFLAGS_DEBUG = $(CFLAGS_COMMON) -O0 -g -DDEBUG $(CS240)
OSX_SPECIFIC =

OS := $(shell uname)
ifeq ($(OS), Darwin)
OSX_SPECIFIC = -dynamiclib -flat_namespace
endif

TESTERS = $(patsubst %.c, %, $(wildcard testers/*.c))
SAMPLES = $(patsubst %.c, %, $(wildcard samples/*.c))

all: programs sharedObjects testers samples

programs: mreplace mstats mstats-libc

sharedObjects: alloc.so lib/mstats-alloc.so lib/mstats-libc-alloc.so lib/osx-sbrk-mmap-wrapper.so

testers: $(TESTERS)
samples: $(SAMPLES)

alloc.so: alloc.c
	$(CC) $^ $(CFLAGS_DEBUG) $(OSX_SPECIFIC) -o $@ -shared -fPIC -lm -ldl

lib/mstats-alloc.so: lib/mstats-alloc.c
	$(CC) $^ $(CFLAGS_DEBUG) $(OSX_SPECIFIC) -o $@ -shared -fPIC -lm -ldl

lib/mstats-libc-alloc.so: lib/mstats-alloc.c
	$(CC) $^ $(CFLAGS_DEBUG) $(OSX_SPECIFIC) -o $@ -shared -fPIC -lm -ldl -DUSE_LIBC_ALLOC


mreplace: mstats.c
	$(CC) $^ $(CFLAGS_DEBUG) -o $@ -ldl -lpthread

mstats: mstats.c
	$(CC) $< $(CFLAGS_RELEASE) -o $@ -ldl -lpthread -DSTATS_MODE

mstats-libc: mstats.c
	$(CC) $< $(CFLAGS_RELEASE) -o $@ -ldl -lpthread -DSTATS_MODE -DUSE_LIBC_ALLOC

lib/osx-sbrk-mmap-wrapper.so: lib/osx-sbrk-mmap-wrapper.c
	$(CC) $^ $(CFLAGS_DEBUG) -o $@ -shared -fPIC -lm

samples/%: samples/%.c
	$(CC) $^ $(CFLAGS_DEBUG) -Wno-unused-variable -o $@

# testers compiled in debug mode to prevent compiler from optimizing away the
# behavior we are trying to test
testers/%: testers/%.c
	$(CC) $^ $(CFLAGS_DEBUG) -o $@

.PHONY : clean
clean:
	-rm -rf *.o alloc.so mreplace mstats mstats-libc testers_exe/ lib/*.so $(SAMPLES) $(TESTERS)
	