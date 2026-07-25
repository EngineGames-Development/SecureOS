#ifndef INTERRUPTS_H
#define INTERRUPTS_H

struct idt_entry {
  unsigned short base_low;
  unsigned short selector;
  unsigned char zero;
  unsigned char flags;
  unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

struct registers {
  unsigned int ds;
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
  unsigned int int_no, err_code;
  unsigned int eip, cs, eflags, useresp, ss;
};

void init_interrupts(void);

#endif
