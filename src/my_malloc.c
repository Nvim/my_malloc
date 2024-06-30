#include "my_malloc.h"

s_Chunk *base = NULL; // beginning of heap
s_Chunk *last = NULL; // last chunk in heap

s_Chunk *extend_heap(s_Chunk *last, size_t size) {
  s_Chunk *chunk = sbrk(0);
  // if ((void *)((uint8_t *)last + last->size + META_SIZE) != (void *)chunk) {
  //   fprintf(stderr, "ERR: expected: %p, got: %p", (void *)chunk,
  //           (void *)((uint8_t *)last + last->size + META_SIZE));
  // }
  if (sbrk(META_SIZE + size) == (void *)-1) {
    return NULL;
  }
  chunk->size = size;
  chunk->next = NULL;
  chunk->prev = NULL;
  chunk->free = 0;
  if (last) {
    chunk->prev = last; // TODO debug this
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
  while (chunk != NULL && !(chunk->free) && (chunk->size < (int)size)) {
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
  new->next = chunk->next;
  chunk->next = new;

  return new;
}

void my_free(void *ptr) {
  s_Chunk *c = (s_Chunk *)((uint8_t *)ptr - META_SIZE);
  if (!(base && c >= base && c <= last)) {
    printf("my_free: Invalid pointer\n");
    return;
  }
  c->free = 1;
  s_Chunk *next = c->next;
  s_Chunk *prev = c->prev;
  if (next && next->free) {
    // coalesce with next 1st
    c->size += next->size;
    c->next = next->next;
  }
  if (prev && prev->free) {
    // coalesce with previous
    prev->size += c->size;
    prev->next = c->next;
  }
}

void *get_payload(s_Chunk *chunk) {
  return (void *)((uint8_t *)chunk + META_SIZE);
}

void *my_malloc(size_t size) {
  size = ALIGN_8(size);
  s_Chunk *new = find_free_chunk(size);
  if (new == NULL) {
    new = extend_heap(last, size);
    last = new;
    if (base == NULL) {
      base = new;
    }
    return get_payload(new);
  } else {

    // TODO: split chunk if possible
    // if (c->size > size+META_SIZE+ALIGNMENT){
    //   s_Chunk *new = split_chunk(c, size);
    //   c->next = new;
    //   new->next = c+size;
    // }
    printf("You shouldn't see this (%zu)\n", size);
    last = new;
    new->free = 0;
    uint8_t *payload = (uint8_t *)new + META_SIZE;
    return (void *)payload;
  }
}

void *get_base() { return base; }

void *get_last() { return last; }

void heap_dump() {
  printf("*Heap Dump*\n");
  if (base == NULL) {
    printf("\tEmpty!\n");
    return;
  }
  printf("\tBase address: %p\n", (void *)base);
  s_Chunk *chunk = base;
  int i = 0;
  while (chunk != NULL) {
    printf("*Chunk #%d:\nFree: %d, Size: %d (%x), Adress: %p, Next: %p, Prev: "
           "%p \n",
           i, chunk->free, chunk->size, chunk->size, (void *)chunk,
           (void *)chunk->next, (void *)chunk->prev);
    chunk = chunk->next;
    i++;
  }
}
