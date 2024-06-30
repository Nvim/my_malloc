#include "../src/my_malloc.h"
#include <stdlib.h>
#include <time.h>

int main() {
  size_t size = 17;
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
  assert(chunk != NULL);

  s_Chunk *prev = NULL;
  int j = 0;
  while (chunk->next != NULL) {
    prev = chunk;
    chunk = chunk->next;
    j++;
  }
  printf("Expected iterations: %d, Got: %d  -  (", NB_ALLOCS, j);
  for (i = 0; i < NB_ALLOCS; i++) {
    printf("%zu ", sizes[i]);
  }
  printf(")\n");
  // cr_expect_eq(j, NB_ALLOCS, "Expected %d, got: %d", NB_ALLOCS, j);
  heap_dump();
}
