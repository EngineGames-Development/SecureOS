#include "include/kernel.h"
#include "include/font.h"

#define NULL ((void *)0)

unsigned int *framebuffer = NULL;
unsigned int screen_width = 1024;
unsigned int screen_height = 768;

int cursor_x = 20;
int cursor_y = 60;

char input_buffer[64];
int input_length = 0;

extern int strcmp(const char *str1, const char *str2);
extern void process_command(void);

unsigned char keyboard_map[128] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};

void draw_pixel(unsigned int x, unsigned int y, unsigned int color) {
  if (x >= screen_width || y >= screen_height || framebuffer == NULL)
    return;
  framebuffer[y * screen_width + x] = color;
}

void draw_rect(unsigned int start_x, unsigned int start_y, unsigned int width,
               unsigned int height, unsigned int color) {
  for (unsigned int y = start_y; y < start_y + height; y++) {
    for (unsigned int x = start_x; x < start_x + width; x++) {
      draw_pixel(x, y, color);
    }
  }
}

void clear_screen_gui(unsigned int color) {
  for (unsigned int i = 0; i < screen_width * screen_height; i++) {
    framebuffer[i] = color;
  }
}

void draw_desktop_and_taskbar(void) {
  clear_screen_gui(0x001A1A24);
  unsigned int taskbar_height = 40;
  unsigned int taskbar_y = screen_height - taskbar_height;
  draw_rect(0, taskbar_y, screen_width, taskbar_height, 0x00101014);

  unsigned int button_width = 60;
  unsigned int button_height = 28;
  unsigned int button_x = 6;
  unsigned int button_y = taskbar_y + ((taskbar_height - button_height) / 2);
  draw_rect(button_x, button_y, button_width, button_height, 0x00005B9E);

  draw_string(button_x + 10, button_y + 6, "START", 0x00FFFFFF);
  draw_string(20, 20, "SECUREOS NEXT GEN", 0x00005B9E);
}

void clear_screen(void) {
  draw_desktop_and_taskbar();
  cursor_x = 20;
  cursor_y = 60;
}

void print_char(char c) {
  if (c == '\n') {
    cursor_x = 20;
    cursor_y += 16;
    return;
  }
  if (c == '\b') {
    if (cursor_x > 20) {
      cursor_x -= 8;
      draw_rect(cursor_x, cursor_y, 8, 16, 0x001A1A24);
    }
    return;
  }
  draw_char(cursor_x, cursor_y, c, 0x00FFFFFF);
  cursor_x += 8;
  if (cursor_x >= (int)screen_width - 20) {
    cursor_x = 20;
    cursor_y += 16;
  }
}

void print_string(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    print_char(str[i]);
  }
}

void sleep_ms(int milliseconds) {
  for (int m = 0; m < milliseconds; m++) {
    for (int u = 0; u < 1000; u++) {
      asm volatile("outb %%al, $0x80" ::: "ax");
    }
  }
}

void start_graphics_terminal(unsigned int *multiboot_info) {
  (void)multiboot_info;
  framebuffer = (unsigned int *)0xFD000000;
  screen_width = 1024;
  screen_height = 768;

  draw_desktop_and_taskbar();

  print_string("Terminal geladen.\n> ");

  while (1) {
    unsigned char status = 0;
    asm volatile("inb $0x64, %0" : "=a"(status));
    if (status & 1) {
      unsigned char scancode = 0;
      asm volatile("inb $0x60, %0" : "=a"(scancode));
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
              print_char('\b');
            }
          } else if (input_length < 63) {
            print_char(c);
            input_buffer[input_length] = c;
            input_length++;
          }
        }
      }
    }
    asm volatile("hlt");
  }
}
