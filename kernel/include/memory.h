#ifndef MEMORY_H
#define MEMORY_H

struct memory_block {
  unsigned int size;
  unsigned int is_free;
};

void init_memory(void);
void *kmalloc(unsigned int size);
void kfree(void *ptr);

#endif
