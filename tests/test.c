#include "../src/my_malloc.h"
#include "criterion/assert.h"
#include "criterion/internal/assert.h"
#include <criterion/criterion.h>
#include <stdlib.h>
#include <time.h>

// allocs a chunk and checks that attributes are valid
Test(alloc, single) {
  size_t size = 17;
  size_t aligned_size = ALIGN_8(size);
  cr_assert(aligned_size - size < 8);

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
#define NB_ALLOCS 15
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
  int j = 0;
  while (chunk->next != NULL) {
    cr_expect_eq(chunk->size, (int)ALIGN_8(sizes[j]),
                 "[%d] Expected %lu, got: %d (Requested: %d)", j,
                 ALIGN_8(sizes[j]), chunk->size, (int)sizes[j]);
    cr_assert_eq(chunk->free, 0, "[%d] Expected 0, got: %d", j, chunk->free);
    cr_expect_eq(chunk->prev, prev, "[%d] Expected %p, got: %p", j,
                 (void *)prev, (void *)chunk->prev);
    prev = chunk;
    chunk = chunk->next;
    j++;
  }
  cr_expect_eq(j, NB_ALLOCS - 1, "Expected %d, got: %d", NB_ALLOCS - 1, j);
  // heap_dump();
}

Test(free, base) {
  size_t sizes[] = {24, 48, 116};
  void *ptr = my_malloc(sizes[0]);
  void *to_free = my_malloc(sizes[1]);
  ptr = my_malloc(sizes[2]);

  s_Chunk *base = get_base();
  cr_assert_not_null(base);

  // heap_dump();
  // printf("\n\n");

  s_Chunk *chunk = base->next;
  cr_expect_eq(chunk->size, (int)ALIGN_8(sizes[1]));
  cr_expect_eq(chunk->free, 0);

  s_Chunk *tmp = (s_Chunk *)((uint8_t *)to_free - META_SIZE);
  cr_assert_eq(chunk, tmp, "Chunk: %p, tmp: %p", (void *)chunk, (void *)tmp);

  my_free(to_free);

  cr_expect_eq(chunk->size, (int)ALIGN_8(sizes[1]),
               "Expected %lu, got: %d (Requested: %d)", ALIGN_8(sizes[1]),
               chunk->size, (int)sizes[1]);
  cr_assert_eq(chunk->free, 1, "Expected 0, got: %d", chunk->free);
  cr_expect_eq(chunk->prev, base, "Expected %p, got: %p", (void *)base,
               (void *)chunk->prev);
}

Test(free, coalesce) {
  size_t sizes[] = {24, 32, 256, 64};
  void *ptr = my_malloc(sizes[0]);
  void *to_free = my_malloc(sizes[1]);
  void *to_free2 = my_malloc(sizes[2]);
  ptr = my_malloc(sizes[3]);

  s_Chunk *base = get_base();
  cr_assert_not_null(base);

  heap_dump();
  printf("\n");

  s_Chunk *chunk = base->next;
  s_Chunk *chunk2 = chunk->next;

  s_Chunk *last = chunk2->next;

  my_free(to_free);

  // Check that no coalescence has occured:
  cr_expect_eq(chunk->size, (int)ALIGN_8(sizes[1]),
               "Expected %lu, got: %d (Requested: %d)", ALIGN_8(sizes[1]),
               chunk->size, (int)sizes[1]);
  cr_assert_eq(chunk->free, 1, "Expected 1, got: %d", chunk->free);
  cr_expect_eq(chunk->prev, base, "Expected %p, got: %p", (void *)base,
               (void *)chunk->prev);
  cr_expect_eq(chunk->next, chunk2, "Expected %p, got: %p", (void *)chunk2,
               (void *)chunk->next);

  my_free(to_free2);

  /* Check coalescence: */
  cr_expect_eq(chunk->size,
               (int)ALIGN_8(sizes[1]) + (int)ALIGN_8(sizes[2]) + META_SIZE);
  cr_assert_eq(chunk->free, 1, "Expected 1, got: %d", chunk->free);
  cr_assert_eq(chunk->next, last, "Expected %p, got: %p", (void *)last,
               (void *)chunk->next);
  cr_assert_eq(last->prev, chunk, "Expected %p, got: %p", (void *)chunk,
               (void *)last->prev);

  /* This shouldn't have changed: */
  cr_expect_eq(chunk->prev, base, "Expected %p, got: %p", (void *)base,
               (void *)chunk->prev);

  printf("- Freed Chuk #1 and #2\n\n");
  heap_dump();
}
