#include "include/memory.h"

extern unsigned long end;
unsigned long heap_start = 0;

void init_memory() {
    heap_start = (unsigned long)&end;
    if (heap_start & 0xF) {
        heap_start = (heap_start & 0xFFFFFFFFFFFFFFF0) + 0x10;
    }
    
    struct memory_block* initial_block = (struct memory_block*)heap_start;
    initial_block->size = 4 * 1024 * 1024;
    initial_block->is_free = 1;
}

void* kmalloc(unsigned int size) {
    if (size == 0) return 0;
    
    if (size & 0xF) {
        size = (size & 0xFFFFFFF0) + 0x10;
    }
    
    unsigned long current_address = heap_start;
    
    while (1) {
        struct memory_block* current_block = (struct memory_block*)current_address;
        
        if (current_block->is_free && current_block->size >= size) {
            unsigned int needed_space = size + sizeof(struct memory_block);
            
            if (current_block->size > needed_space + 32) {
                unsigned long next_address = current_address + needed_space;
                struct memory_block* next_block = (struct memory_block*)next_address;
                next_block->size = current_block->size - needed_space;
                next_block->is_free = 1;
                
                current_block->size = size;
            }
            
            current_block->is_free = 0;
            return (void*)(current_address + sizeof(struct memory_block));
        }
        
        current_address += current_block->size + sizeof(struct memory_block);
        
        if (current_address >= heap_start + (4 * 1024 * 1024)) {
            break;
        }
    }
    
    return 0;
}

void kfree(void* ptr) {
    if (!ptr) return;
    
    unsigned long block_address = (unsigned long)ptr - sizeof(struct memory_block);
    struct memory_block* block = (struct memory_block*)block_address;
    block->is_free = 1;
}
