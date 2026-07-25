#include "include/gdt.h"

struct gdt_entry gdt[5];
struct gdt_ptr gdt_record;

extern void gdt_flush(unsigned int);

void set_gdt_gate(int num, unsigned int base, unsigned int limit,
                  unsigned char access, unsigned char gran) {
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;

  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = (limit >> 16) & 0x0F;

  gdt[num].granularity |= gran & 0xF0;
  gdt[num].access = access;
}

void init_gdt() {
  gdt_record.limit = (sizeof(struct gdt_entry) * 5) - 1;
  gdt_record.base = (unsigned int)&gdt;

  set_gdt_gate(0, 0, 0, 0, 0);
  set_gdt_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
  set_gdt_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
  set_gdt_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
  set_gdt_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

  gdt_flush((unsigned int)&gdt_record);
}
