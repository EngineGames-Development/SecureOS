#include "include/interrupts.h"
#include "include/io.h"
#include "include/kernel.h"
#include "include/timer.h"

struct idt_entry idt[256];
struct idt_ptr idtr;

extern void load_idt(unsigned int);
extern void irq0(void);
extern void irq1(void);
extern void irq9(void);

extern void exception0(void);
extern void exception13(void);
extern void exception14(void);

extern unsigned char keyboard_map[];
extern char input_buffer[];
extern int input_length;
extern int cursor_x;
extern int cursor_y;
extern volatile unsigned short *video_memory;

void set_idt_gate(unsigned char num, unsigned int base) {
  idt[num].base_low = (base & 0xFFFF);
  idt[num].base_high = (base >> 16) & 0xFFFF;
  idt[num].selector = 0x08;
  idt[num].zero = 0;
  idt[num].flags = 0x8E;
}

void init_pic(void) {
  outb(0x20, 0x11);
  outb(0xA0, 0x11);
  outb(0x21, 0x20);
  outb(0xA1, 0x28);
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);
  outb(0x21, 0x00);
  outb(0xA1, 0x00);
}

void init_interrupts(void) {
  idtr.limit = (sizeof(struct idt_entry) * 256) - 1;
  idtr.base = (unsigned int)&idt;

  for (int i = 0; i < 256; i++) {
    set_idt_gate(i, 0);
  }

  init_pic();

  set_idt_gate(0, (unsigned int)exception0);
  set_idt_gate(13, (unsigned int)exception13);
  set_idt_gate(14, (unsigned int)exception14);

  set_idt_gate(32, (unsigned int)irq0);
  set_idt_gate(33, (unsigned int)irq1);
  set_idt_gate(41, (unsigned int)irq9);

  load_idt((unsigned int)&idtr);
  asm volatile("sti");
}

void exception_handler(struct registers regs) {
  clear_screen();
  print_string("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
               "!!!!!!!!!!!!!!!!!\n");
  print_string("               S E C U R E   O S   -   C P U   E X C E P T I O "
               "N                \n");
  print_string("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
               "!!!!!!!!!!!!!!!!!\n\n");

  if (regs.int_no == 0) {
    print_string(" KERNEL PANIC: Division by Zero (ISR 0)\n");
  } else if (regs.int_no == 13) {
    print_string(" KERNEL PANIC: General Protection Fault (ISR 13)\n");
  } else if (regs.int_no == 14) {
    print_string(
        " KERNEL PANIC: Page Fault - Illegal Memory Access (ISR 14)\n");
  }

  print_string("\n Das System wurde aus Sicherheitsgruenden angehalten.\n");

  asm volatile("cli; hlt");
}

void irq_handler(struct registers regs) {

  if (regs.int_no < 32) {
    exception_handler(regs);
    return;
  }

  if (regs.int_no == 32) {
    timer_handler();
  }

  if (regs.int_no == 33) {
    unsigned char status = inb(0x64);
    if (status & 1) {
      unsigned char scancode = inb(0x60);
      if (!(scancode & 0x80)) {
        char c = keyboard_map[scancode];
        if (c != 0) {
          if (c == '\n') {
            input_buffer[input_length] = '\0';
            process_command();
          } else if (c == '\b') {
            if (input_length > 0) {
              input_length--;
              input_buffer[input_length] = '\0';
              if (cursor_x > 0)
                cursor_x--;
              else if (cursor_y > 0) {
                cursor_y--;
                cursor_x = 80 - 1;
              }
              video_memory[cursor_y * 80 + cursor_x] = ' ' | (0x0F << 8);
              update_hardware_cursor();
            }
          } else if (input_length < 63) {
            print_char(c);
            input_buffer[input_length] = c;
            input_length++;
          }
        }
      }
    }
  } else if (regs.int_no == 41) {
    print_string("\nPower-Button Signal empfangen. Fahre herunter...\n");
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);
    outw(0x604, 0x3400);
    asm volatile("cli; hlt");
  }

  if (regs.int_no >= 40) {
    outb(0xA0, 0x20);
  }
  outb(0x20, 0x20);
}
