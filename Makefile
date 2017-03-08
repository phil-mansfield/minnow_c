# Author: Phil Mansfield (mansfield@uchicago.edu)
#
# How to run:
# make - puts a library file in build/
# make test - runs all tests and benchmarks in ./test. Assumes the existence
#              of a particular script called runTests.py in scripts/.
# make clean - removes temporary files and allows for a clean build.
#
# This Makefile is designed so that it's easy(ish) to modify it for new
# projects. I wholeheartedly encourage you to do this. To use for a new
# project, go to the commented areas in this file and do what# # they say.

CLANG_CFLAGS=-Werror -Weverything -Wno-unused-macros -Wno-missing-prototypes -Wno-bad-function-cast -Wno-float-equal -Wno-padded -std=c99 -pedantic -O2 -g -D DEBUG

GCC_CFLAGS= -O2 -Wall -Wextra -Werror -std=c99 -pedantic -Wshadow -Wcast-qual -Wcast-align -Wundef -Wredundant-decls -Wmissing-include-dirs -g -D DEBUG

# icc generally generates too many remarks (small warnings), some of which are
# caused by very benign actions which are impossible to avoid. If you want to
# remove more remarks, you can add to the comma-separated list at the end of
# the flag set.
#
# Currently removed icc remarks:
# 1419 - Disallows forward declarations in source files.
# 1572 - Floating point equality and inequalities.
# 2259 - This is a well meaning warning which warns out non-explicit casts,
#        but it also watns about, e.g., subtracting ints.
ICC_CFLAGS= -O2 -g -D DEBUG -Werror -std=c99 -pedantic -w3 -wd1419,1572,2259

# Select the compiler and compiler flags that you want to use.
CC=clang
CFLAGS =$(CLANG_CFLAGS)

# Add/remove any additional files that aren't in src at the end of this list.
SOURCES=$(wildcard src/*.c) lz4/lib/lz4.c
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
HEADERS=$(patsubst %.c,%.h,$(SOURCES))

# Change this as neccesary. If you aren't making a library, the vast majority
# of this Makefile will be useless to you. Name will be build/*lib.a
LIB_NAME=minnow

TARGET=$(patsubst %,build/lib%.a,$(LIB_NAME))
SELF_FLAG=$(patsubst %,-l%,$(LIB_NAME))

SO_TARGET=$(patsubst, %.a,%.so,$(TARGET))

TEST_SOURCES=$(wildcard test/*.c)
TESTS=$(patsubst %.c,%,$(TEST_SOURCES))

# Location of libraries being used.
LIBRARIES=
# Flags of libraries being used.
LIBRARY_FLAGS=-lm
# Location of .h files which should be included.
INCLUDES=

ifneq ($(INCLUDES),)
	INCLUDES_WITH_FLAG=-I $(INCLUDES)
else
	INCLUDES_WITH_FLAG=$(INCLUDES)
endif

all: $(TARGET) $(SO_TARGET)

debug: CFLAGS += -g -D DEBUG
debug: all

debug_test: CFLAGS += -g -D DEBUG
debug_test: test

build:
	mkdir -p build/

src/base_seq.c: scripts/seq_gen.py Makefile resources/seq_base.c
	python scripts/seq_gen.py c < resources/seq_base.c > src/base_seq.c

src/base_seq.h: scripts/seq_gen.py Makefile resources/seq_base.h
	python scripts/seq_gen.py h < resources/seq_base.h > src/base_seq.h

# Extend this as needed for each object file. List dependencies that are
# not the corresponding .c and .h file.
src/seq.o: src/base_seq.h
lz4/lib/lz4.o:
	$(CC) -O3 -std=c99 -Wall -Wextra -c lz4/lib/lz4.c -o lz4/lib/lz4.o
src/compress_util.o:
	$(CC) -I lz4/lib $(CFLAGS) -c src/compress_util.c -o src/compress_util.o
%.o: %.c %.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDES_WITH_FLAG)

$(TARGET): build $(OBJECTS) $(HEADERS)
	ar rcs $@ $(OBJECTS)
	ranlib $@

$(SO_TARGET): $(CFLAGS) += -fPIC
$(SO_TARGET): build $(OBJECTS) $(HEADERS)
	$(CC) -shared -o $@ $(OBJECTS)

%_test: %_test.c $(TARGET)
	$(CC) $@.c -o $@ $(CFLAGS) -L build $(LIBRARIES) -I src $(SELF_FLAG) $(LIBRARY_FLAGS)
%_bench: %_bench.c $(TARGET)
	$(CC) $@.c -o $@ $(CFLAGS) -L build $(LIBRARIES) -I src $(SELF_FLAG) $(LIBRARY_FLAGS)

test: $(TESTS) scripts/run_tests.py $(TARGET)
	@python scripts/run_tests.py test/ test
	@python scripts/run_tests.py test/ bench

clean:
	rm -rf build/
	rm -f  $(OBJECTS)
	rm -f  test/*_test
	rm -f  test/*_bench
	rm -rf test/*.dSYM
	rm -f  src/seq_base.*
