#include "my_malloc.h"

s_Chunk *base = NULL; // beginning of heap

/**
 * @brief Extends heap by moving program break
 *
 * @param last  Last chunk in the heap (can be NULL)
 * @param size  Size of requested chunk's payload
 * @return s_Chunk*
 */
s_Chunk *extend_heap(s_Chunk *last, size_t size) {
  s_Chunk *chunk = sbrk(0);
  if (sbrk(META_SIZE + size) == (void *)-1) {
    fprintf(stderr, "Error: sbrk failed\n");
    return NULL;
  }
  chunk->size = size;
  chunk->next = NULL;
  chunk->prev = last;
  chunk->free = 0;
  if (last)
    last->next = chunk;
  return chunk;
}

/**
 * @brief Returns the 1st big enough chunk in the heap
 *        TODO: implement free list 'hidden' in free chunks' data
 *
 * @param last: Pointer to pointer because it will be set to 'chunk', which will
 * go out of scope
 * @param size: Size of the chunk's payload
 * @return s_Chunk*
 */
s_Chunk *find_free_chunk(s_Chunk **last, size_t size) {
  s_Chunk *chunk = base;
  while (chunk && (!(chunk->free) || (chunk->size < (int)size))) {
    *last = chunk;
    chunk = chunk->next;
  }
  return chunk;
}

/**
 * @brief Splits chunk in two if possible . new->free is initialized to 1
 *        TODO: test it
 *
 * @param chunk: chunk to split
 * @param size: size the old chunk should be (where to split from old)
 * @return returns ptr to new chunk.
 */
s_Chunk *split_chunk(s_Chunk *chunk, size_t size) {
  if (chunk->size < (int)(size + META_SIZE + MIN_PAYLOAD)) {
    return NULL;
  }
  int new_size = chunk->size - size - META_SIZE; // size of new chunk's payload
  s_Chunk *new = (s_Chunk *)((uint8_t *)chunk + size + META_SIZE);
  new->size = new_size;
  new->prev = chunk;
  new->free = 1;
  new->next = chunk->next;
  chunk->next = new;
  chunk->size = size;

  return new;
}

void *get_payload(s_Chunk *chunk) {
  if (!chunk)
    return NULL;
  return (void *)((uint8_t *)chunk + META_SIZE);
}

void *my_malloc(size_t size) {
  size = ALIGN_8(size);
  s_Chunk *new = NULL, *last = NULL;
  if (base) { // Look for a free chunk before extending
    last = base;
    new = find_free_chunk(&last, size);
    if (new) {
      // TODO: Try to split the chunk
      printf("[LOG] Alloc of size %zu - Using an existing chunk of size: %d\n",
             size, new->size);
      if (split_chunk(new, size)) {
        printf(
            "[LOG] Alloc of size %zu - Chunk has been split in sizes: %d, %d\n",
            size, new->size, new->next->size);
      }
      new->free = 0;
    } else { // Extend
      new = extend_heap(last, size);
    }
  } else { // 1st execution, just extend
    new = extend_heap(NULL, size);
    base = new;
  }
  return (void *)get_payload(new);
}

/**
 * @brief Free a pointer that was returned by my_malloc
 *        TODO: test it
 *
 * @param ptr
 */
void my_free(void *ptr) {
  s_Chunk *c = (s_Chunk *)((uint8_t *)ptr - META_SIZE);
  c->free = 1;
  s_Chunk *next = c->next;
  s_Chunk *prev = c->prev;
  if (next && next->free) {
    // coalesce with next 1st
    c->size += next->size + META_SIZE;
    c->next = next->next;
    if (c->next)
      c->next->prev = c;
  }
  if (prev && prev->free) {
    // coalesce with previous
    prev->size += c->size + META_SIZE;
    prev->next = c->next;
    if (prev->next)
      prev->next->prev = prev;
  }
}

/**
 *FOR TESTING
 */

void *get_base() { return base; }

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
    printf(
        "*Chunk #%d:\nFree: %d, Size: %d (0x%x), Adress: %p, Next: %p, Prev: "
        "%p \n",
        i, chunk->free, chunk->size, chunk->size, (void *)chunk,
        (void *)chunk->next, (void *)chunk->prev);
    chunk = chunk->next;
    i++;
  }
}
