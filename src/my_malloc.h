#define _DEFAULT_SOURCE
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define META_SIZE sizeof(s_Chunk)

// chunk's data is at least 8, to hold 2 ptrs: (TODO: make portable)
#define MIN_PAYLOAD 8
#define ALIGNMENT 8 // TODO: make portable
#define ALIGN_8(x) (((x) + 7) & ~7)

typedef struct s_Chunk {
  int size;
  int free;
  struct s_Chunk *next;
  struct s_Chunk *prev;
} s_Chunk;

s_Chunk *extend_heap(s_Chunk *last, size_t size);
s_Chunk *find_free_chunk(size_t size);
s_Chunk *split_chunk(s_Chunk *chunk, size_t size);
void my_free(void *ptr);
void *my_malloc(size_t size);
void *get_base();
void *get_last();
void heap_dump();
