# Author: Phil Mansfield (mansfield@uchicago.edu)
#
# How to run:
# make - puts a library file in ./build
# make test - runs all tests and benchmarks in ./test. Assumes the existence
#              of a particular script called runTests.py in scripts/.
# make clean - removes temporary files and allows for a clean build.
#
# How to use for a new project:
# Go to the other commented areas in this file and do what they say.

#CC=clang

#CC=gcc
CFLAGS= -O2 -Wall -Wextra -Werror -std=c99 -pedantic -Wshadow -Wcast-qual -Wcast-align -Wundef -Wredundant-decls -Wmissing-include-dirs -g -D DEBUG

CC=icc
CFLAGS= -O2 -Wall -Wextra -Werror -std=c99 -pedantic -Wshadow -Wcast-qual -Wundef -Wmissing-include-dirs -g -D DEBUG

SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
HEADERS=$(patsubst %.c,%.h,$(SOURCES))

# Change this as neccesary. If you aren't making a library, the vast majority of
# this Makefile will be useless to you. Name will be build/*lib.a
LIB_NAME=minnow

TARGET=$(patsubst %,build/lib%.a,$(LIB_NAME))
SELF_FLAG=$(patsubst %,-l%,$(LIB_NAME))

SO_TARGET=$(patsubst, %.a,%.so,$(TARGET))

TEST_SOURCES=$(wildcard test/*.c)
TESTS=$(patsubst %.c,%,$(TEST_SOURCES))

# Location of libraries being used.
LIBRARIES=
# Flags of libraries being used.
LIBRARY_FLAGS=
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

src/base_seq.h: scripts/seq_gen.py Makefile resources/seq_base.h resources/seq_base.c
	python scripts/seq_gen.py c < resources/seq_base.c > src/base_seq.c
	python scripts/seq_gen.py h < resources/seq_base.h > src/base_seq.h

# Change this as needed for each object file. List dependencies that are
# not the corresponding .c and .h file.
src/seq.o: src/base_seq.h
%.o: %.c %.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDES_WITH_FLAG)

$(TARGET): build $(OBJECTS) $(HEADERS)
	ar rcs $@ $(OBJECTS)
	ranlib $@

$(SO_TARGET): $(CFLAGS) += -fPIC
$(SO_TARGET): build $(OBJECTS) $(HEADERS)
	$(CC) -shared -o $@ $(OBJECTS)

%_test: %_test.c $(TARGET)
	$(CC) $@.c -o $@ $(CFLAGS) -L build $(LIBRARIES) -I src $(INCLUDES) $(SELF_FLAG) $(LIBRARY_FLAGS)
%_bench: %_bench.c $(TARGET)
	$(CC) $@.c -o $@ $(CFLAGS) -L build $(LIBRARIES) -I src $(INCLUDES) $(SELF_FLAG) $(LIBRARY_FLAGS)

test: $(TESTS) scripts/run_tests.py $(TARGET)
	@python scripts/run_tests.py test/ test
	@python scripts/run_tests.py test/ bench

clean:
	rm -rf build/
	rm -f  src/*.o
	rm -f  test/*_test
	rm -f  test/*_bench
	rm -rf test/*.dSYM
	rm -f  src/seq_base.*
