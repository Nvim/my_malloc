#include "../src/my_malloc.h"
#include "criterion/assert.h"
#include "criterion/internal/assert.h"
#include <criterion/criterion.h>
#include <stdlib.h>
#include <time.h>

void setup(size_t size, s_Chunk *ptr) {
  void *p = my_malloc(size);
  ptr = (s_Chunk *)((uint8_t *)ptr - META_SIZE);
}

// allocs a chunk and checks that attributes are valid
Test(alloc, single) {
  size_t size = 17;
  size_t aligned_size = ALIGN_8(size);

  void *ptr = my_malloc(size);
  s_Chunk *chunk = (s_Chunk *)((uint8_t *)ptr - META_SIZE);

  cr_assert_not_null(chunk);
  cr_assert_eq(chunk->size, aligned_size);
  cr_assert_eq(chunk->free, 0);
  cr_assert_null(chunk->next, "Expected NULL, got: %p", chunk->next);
  cr_assert_null(chunk->prev, "Expected NULL, got: %p", chunk->prev);
}

Test(alloc, multiple) {
  size_t size = 17;
  size_t aligned_size = ALIGN_8(size);
  void *ptr = my_malloc(size);

  size_t size2 = 32;
  size_t aligned_size2 = ALIGN_8(size2);
  ptr = my_malloc(size2);

  // heap_dump();

  s_Chunk *chunk = get_base();
  s_Chunk *first = chunk;
  cr_assert_not_null(chunk);
  while (chunk->next != NULL) {
    chunk = chunk->next;
  }
  cr_assert_eq(chunk->size, aligned_size2, "Expected %lu, got: %d",
               aligned_size, chunk->size);
  cr_assert_eq(chunk->free, 0);
  cr_assert_null(chunk->next, "Expected NULL, got: %p", chunk->next);
  cr_assert_eq(chunk->prev, first, "Expected %p, got: %p", first, chunk->prev);
}

Test(alloc, alot) {
#define NB_ALLOCS 4
  size_t sizes[NB_ALLOCS];
  srand(time(0));
  void *ptr;
  int i;
  for (i = 0; i < NB_ALLOCS; i++) {
    size_t size = (rand() % 256) + 1;
    sizes[i] = size;
    ptr = my_malloc(size);
  }

  s_Chunk *chunk = get_base();
  cr_assert_not_null(chunk);

  s_Chunk *prev = NULL;
  i = 0;
  while (chunk->next != NULL) {
    cr_assert_eq(chunk->size, ALIGN_8(sizes[i]), "Expected %lu, got: %d",
                 ALIGN_8(sizes[i]), chunk->size);
    cr_assert_eq(chunk->free, 0, "Expected 0, got: %d", chunk->free);
    cr_assert_eq(chunk->prev, prev, "Expected %p, got: %p", (void *)chunk->prev,
                 (void *)prev);
    prev = chunk;
    chunk = chunk->next;
    i++;
  }
}
