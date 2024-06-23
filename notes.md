# Terminology

- Chunk/Block: piece of memory allocated by malloc
- Payload: data part of the chunk (malloc returns a void ptr to this)
- Header/Footer: structs present at the beginning/end of each chunk containing
metadata

# design

## Headers

- Header for metadata at the beginning of each chunk
- Size needs to be aligned 
- Can hold a next and a prev ptr
- Can instead hold only block size, and we go to next by adding the bytes

### Layout option #1

- size (int):    4 bytes
- next (void *): 8 bytes
- prev (void *): 8 bytes
- free (int):    4 bytes

=> total = 24 bytes -> aligned

Not using size_t for "size" for alignment purposes. Int is fine as i'll never
have a chunk of size 2147483647+ (2GB).

I could even go lower with unsigned types, uint16_t holds 65536 bytes (65KB),
but it will kill alignment. Same idea for "free". it's useless to go with char
as header will be 21 bytes.

### Layout option #2

- Size (int): 4 bytes

With the following assumptions:

- Header is duplicated as a footer at the end of the block
- The size of a block (including head & foot) is always 8 bytes aligned
- Eventual padding is added at the end of payload
- The value of "Size" in head/foot is the size of the whole block, not payload

The header is duplicated => 2*4 bytes = 8 bytes => aligned.
Since we know that size is aligned, it's always a multiple of 8. So the 
first 3 bits of size are always 0 -> no need to check them -> we can use
them for other purposes.
The free bit flag is stored here, and we retrieve it with bitmask.

No need to hold a reference to next/prev, we can compute them:
- Next: beginning of block + size
- Prev: beginning of block - 4 bytes will give us it's footer

The footer contains the size so we can go back up to the head if we need.

## Find-free algorithm

I'll go with the simplest: first-fit, but options exit:

- 1st-fit: it's in the name
- best-fit: O(n) over the whole heap to find the closest size to the requested
- next-fit: 1st fit, but start searching from the last allocated instead of
from the start

## Freeing

There are 2 main issues:

### Pointer manipulation

Problem:
- Checking if the provided ptr comes from 'malloc'
- Finding the block corresponding to the given ptr

Solutions to avoid O(n)-ing the whole list:
- Maintain a free-list which contains ptrs to free chunks only
- 'Hide' the free-list in the free chunks' payload -> 1 ptr to next free, 1 ptr
to previous free = 8 bytes (payload needs to be >= 8 bytes)

Using the 2nd option would mean:
- Setting a MIN_ALLOC_SIZE constant preventing user from allocating less than 8
bytes (dumb).
- Always allocating a payload of >= 8 bytes, and padding it if needed. A
malloc(1) call would use __24+8 = 32__ _(layout #1)_, or __8+8 = 16__ _(layout
#2)_ (dumb, but less)

### Coalescense

- Perform it for the whole heap occasionnaly?
- Perform it after every free?

=> after every free is easier: just do the block before and the one after

## Alloc syscalls

brk/sbrk look easier than mmap/munmap, I don't really understand the point 
of mmap.

# My implementation

## Key features

- Header-only double linked list.
- 1st-fit algorithm
- Free list in payloads of free chunks
- Force 8 bytes alignment (eventual padding at end of payload)
- coalesce on every free
- pointer validation (before freeing): O(n) for now
- brk/sbrk to add and remove space

## Small trick to allocate structs without memory allocator

- A struct is just a concatenation of all it's fields in memory
- I can allocate a pointer to a struct wherever I want
- So I can do it using the return value of sbrk:

```
struck s_Chunk *b = sbrk(0);
b->size = size;
b->next = NULL;
b->free = 0;
...
```

Just fill the fields and it works

## Functions

### find_free_chunk

- returns adress to the 1st found chunk big enough
- returns NULL if none

### extend_heap

- returns chunk to newly allocated memory
- takes current last chunk as a param to set it's 'next' ptr to the new

### split_chunk

- splits a chunk into two
- takes a 'size' param to know where to split

### malloc

- Align requested size
- If 1st time: extend heap and return new
- Call find_free_chunk  
- If found, try to split it, mark it as used and return it
- If not found, extend heap and return new


