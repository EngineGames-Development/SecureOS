#include "../include/timer.h"
#include "../include/io.h"
#include "../include/kernel.h"

unsigned int system_ticks = 0;

void init_timer(unsigned int frequency) {
  unsigned int divisor = 1193180 / frequency;

  outb(0x43, 0x36);

  unsigned char low = (unsigned char)(divisor & 0xFF);
  unsigned char high = (unsigned char)((divisor >> 8) & 0xFF);

  outb(0x40, low);
  outb(0x40, high);
}

void timer_handler() {
  system_ticks++;

  if (system_ticks % 100 == 0) {
    volatile unsigned short *menu_bar = (volatile unsigned short *)0xB8000;

    unsigned int seconds = system_ticks / 100;
    unsigned char digit1 = '0' + (seconds / 10) % 10;
    unsigned char digit2 = '0' + (seconds % 10);

    menu_bar[78] = digit1 | (0x2F << 8);
    menu_bar[79] = digit2 | (0x2F << 8);
  }
}
