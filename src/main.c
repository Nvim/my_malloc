#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define META_SIZE sizeof(s_Chunk)

// chunk's data is at least 8, to hold 2 ptrs: (TODO: make portable)
#define MIN_PAYLOAD 8
#define ALIGNMENT 8 // TODO: make portable
#define ALIGN_8(x) (((x) + 7) & ~7)

typedef struct {
  int size;
  void *next;
  void *prev;
  int free;
} s_Chunk;

s_Chunk *base = NULL; // beginning of heap
s_Chunk *last = NULL; // last chunk in heap

s_Chunk *extend_heap(s_Chunk *last, size_t size) {
  s_Chunk *chunk = sbrk(size);
  chunk->size = size;
  chunk->next = NULL;
  chunk->free = 0;
  if (last) {
    chunk->prev = last;
    last->next = chunk;
  }
  return chunk;
}

// 1st fit, O(n) all list
// TODO: implement free list 'hidden' in free chunks' data
s_Chunk *find_free_chunk(size_t size) {
  if (base == NULL) {
    return NULL;
  }
  s_Chunk *chunk = base;
  while (chunk != NULL && !(chunk->size >= size && chunk->free == 1)) {
    chunk = chunk->next;
  }
  return chunk;
}

// splits chunk in two, returns ptr to new chunk
// new chunk is free by default
s_Chunk *split_chunk(s_Chunk *chunk, size_t size) {
  // check if size is legal:
  if ((size + META_SIZE) >= (chunk->size - META_SIZE - MIN_PAYLOAD)) {
    return NULL;
  }
  s_Chunk *new = chunk + size + META_SIZE;
  new->size = size;
  new->prev = chunk;
  new->free = 1;
  chunk->next = new;

  return new;
}

void *my_malloc(size_t size) {
  size = ALIGN_8(size);
  s_Chunk *c = find_free_chunk(size);
  if (c == NULL) {
    void *new = extend_heap(last, size);
    last = new;
    if (base == NULL)
      base = new;
    void *payload = new + META_SIZE;
    return payload;
  }
  // TODO: split chunk if possible
  // if (c->size > size+META_SIZE+ALIGNMENT){
  //   s_Chunk *new = split_chunk(c, size);
  //   c->next = new;
  //   new->next = c+size;
  // }
  assert(c != NULL);
  return c;
}

int main() {
  void *ptr = my_malloc(12);
  assert(ptr != NULL);
  printf("%p\n", ptr);
  void *ptr2 = my_malloc(12);
  assert(ptr2 != NULL);
  printf("%p\n", ptr2);
  return EXIT_SUCCESS;
}
