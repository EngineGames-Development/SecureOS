#include "include/memory.h"

#define HEAP_SIZE (4 * 1024 * 1024)

extern unsigned long end;
unsigned long heap_start = 0;

void init_memory() {
  heap_start = (unsigned long)&end;
  heap_start = (heap_start + 15) & ~15UL;

  struct memory_block *initial_block = (struct memory_block *)heap_start;
  initial_block->size = HEAP_SIZE - sizeof(struct memory_block);
  initial_block->is_free = 1;
}

void *kmalloc(unsigned int size) {
  if (size == 0)
    return 0;

  size = (size + 15) & ~15UL;
  unsigned long current_address = heap_start;

  while (current_address < heap_start + HEAP_SIZE) {
    struct memory_block *current_block = (struct memory_block *)current_address;

    if (current_block->is_free && current_block->size >= size) {
      unsigned int needed_space = size + sizeof(struct memory_block);

      if (current_block->size >=
          needed_space + sizeof(struct memory_block) + 16) {
        unsigned long next_address = current_address + needed_space;
        struct memory_block *next_block = (struct memory_block *)next_address;

        next_block->size = current_block->size - needed_space;
        next_block->is_free = 1;

        current_block->size = size;
      }

      current_block->is_free = 0;
      return (void *)(current_address + sizeof(struct memory_block));
    }

    current_address += current_block->size + sizeof(struct memory_block);
  }

  return 0;
}

void kfree(void *ptr) {
  if (!ptr)
    return;

  unsigned long block_address =
      (unsigned long)ptr - sizeof(struct memory_block);
  struct memory_block *block = (struct memory_block *)block_address;
  block->is_free = 1;

  unsigned long current_address = heap_start;

  while (current_address < heap_start + HEAP_SIZE) {
    struct memory_block *current = (struct memory_block *)current_address;
    unsigned long next_address =
        current_address + current->size + sizeof(struct memory_block);

    if (next_address >= heap_start + HEAP_SIZE) {
      break;
    }

    struct memory_block *next = (struct memory_block *)next_address;

    if (current->is_free && next->is_free) {
      current->size += next->size + sizeof(struct memory_block);
    } else {
      current_address = next_address;
    }
  }
}

unsigned int get_free_memory_size(void) {
  unsigned int total_free = 0;

  unsigned int current_address = heap_start;

  while (1) {
    struct memory_block *current_block = (struct memory_block *)current_address;

    if (current_block->is_free) {
      total_free += current_block->size;
    }

    current_address += current_block->size + sizeof(struct memory_block);

    if (current_address >= heap_start + (4 * 1024 * 1024)) {
      break;
    }
  }

  return total_free;
}

void *memcpy(void *dest, const void *src, unsigned int n) {
  char *d = (char *)dest;
  const char *s = (const char *)src;
  for (unsigned int i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dest;
}

void *memset(void *s, int c, unsigned int n) {
  char *p = (char *)s;
  for (unsigned int i = 0; i < n; i++) {
    p[i] = (char)c;
  }
  return s;
}

void *memmove(void *dest, const void *src, unsigned int n) {
  char *d = (char *)dest;
  const char *s = (const char *)src;
  if (d < s) {
    for (unsigned int i = 0; i < n; i++)
      d[i] = s[i];
  } else {
    for (unsigned int i = n; i > 0; i--)
      d[i - 1] = s[i - 1];
  }
  return dest;
}
