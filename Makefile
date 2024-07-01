CC=gcc
CFLAGS=-std=c11 -pedantic -Wall -Wextra -Wvla -fPIC
DEBUG_FLAGS=-ggdb
EXTRA_FLAGS=-Werror

LINK_FLAGS=-lcriterion ./libmy_malloc.so

SRCS=src/my_malloc.c
TEST_SRCS := $(wildcard tests/*.c)

OBJS=my_malloc.o

LIB=libmy_malloc.so
LIB_DEBUG=libmy_malloc_debug.so

TEST_BINS := $(TEST_SRCS:tests/%.c=%)

all: $(LIB) $(TEST_BINS)

tests: $(TEST_BINS)
	for test in $(TEST_BINS) ; do ./$$test ; done

lib: $(LIB)
# debug: $(LIB_DEBUG)

# makes main library:
$(LIB): $(OBJS)
	$(CC) -shared -o $(LIB) $(OBJS) $(DEBUG_FLAGS)

$(OBJS): $(SRCS)
	$(CC) -c $(CFLAGS) $(DEBUG_FLAGS) $(SRCS) -o $(OBJS)

# same with debug symbols
# $(LIB_DEBUG): $(SRCS)
# 	$(CC) -shared $(CFLAGS) $(DEBUG_FLAGS) $(SRCS) -o $(LIB_DEBUG)

# makes all tests from the tests/ folder
%: tests/%.c $(LIB)
	$(CC) $< $(DEBUG_FLAGS) $(LINK_FLAGS) -o $@

clean: 
	rm -rf *.o *.so $(TEST_BINS)

