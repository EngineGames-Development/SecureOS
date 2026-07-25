#include "include/kernel.h"
#include "include/gdt.h"
#include "include/interrupts.h"
#include "include/io.h"
#include "include/string.h"
#include "include/timer.h"

volatile unsigned short *video_memory = (volatile unsigned short *)0xB8000;
const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;
int cursor_x = 0;
int cursor_y = 0;
char input_buffer[64];
int input_length = 0;

void update_hardware_cursor() {
  unsigned short position = cursor_y * VGA_WIDTH + cursor_x;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(position & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void clear_screen() {
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
    video_memory[i] = 0x0720;
  }
  cursor_x = 0;
  cursor_y = 0;
  update_hardware_cursor();
}

void scroll() {
  for (int y = 1; y < VGA_HEIGHT; y++) {
    for (int x = 0; x < VGA_WIDTH; x++) {
      video_memory[(y - 1) * VGA_WIDTH + x] = video_memory[y * VGA_WIDTH + x];
    }
  }
  for (int x = 0; x < VGA_WIDTH; x++) {
    video_memory[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = 0x0720;
  }
  cursor_y = VGA_HEIGHT - 1;
}

void print_char(char c) {
  if (c == '\n') {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= VGA_HEIGHT)
      scroll();
    update_hardware_cursor();
    return;
  }
  int index = cursor_y * VGA_WIDTH + cursor_x;
  video_memory[index] = c | (0x0F << 8);
  cursor_x++;
  if (cursor_x >= VGA_WIDTH) {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= VGA_HEIGHT)
      scroll();
  }
  update_hardware_cursor();
}

void print_string(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    print_char(str[i]);
  }
}

void sleep_ms(int milliseconds) {
  for (int m = 0; m < milliseconds; m++) {
    for (int u = 0; u < 1000; u++) {
      outb(0x80, 0);
    }
  }
}

unsigned char keyboard_map[128] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};

void kernel_main(void) {
  clear_screen();

  print_string("\n\n\n");
  print_string("                     ====================================\n");
  print_string("                              S E C U R E   O S          \n");
  print_string("                     ====================================\n\n");
  print_string("                              Booting Kernel...          \n\n");
  print_string(
      "                     [                                    ] 0%");

  cursor_y = 8;
  cursor_x = 22;
  update_hardware_cursor();

  for (int i = 1; i <= 10; i++) {
    sleep_ms(3000);
    print_string("===");
    int alter_x = cursor_x;
    cursor_x = 59;
    update_hardware_cursor();
    if (i == 1)
      print_string("10%");
    if (i == 3)
      print_string("30%");
    if (i == 5)
      print_string("50%");
    if (i == 8)
      print_string("80%");
    if (i == 10)
      print_string("100%");
    cursor_x = alter_x;
    update_hardware_cursor();
  }

  sleep_ms(3000);
  clear_screen();

  print_string("SecureOS Terminal v0.5 (True Event-Driven via IDT)\n");
  print_string("Gib 'help' oder 'clear' ein.\n\n> ");
  init_gdt();
  init_interrupts();
  init_timer(100);

  while (1) {
    asm volatile("hlt");
  }
}
