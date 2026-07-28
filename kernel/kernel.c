#include "include/kernel.h"
#include "include/font.h"

#define NULL ((void *)0)
#define TERM_ROWS 21
#define TERM_COLS 72

unsigned int *framebuffer = NULL;
unsigned int screen_width = 1024;
unsigned int screen_height = 768;

int term_box_x = 212;
int term_box_y = 150;
int term_box_width = 600;
int term_box_height = 400;

char terminal_buffer[TERM_ROWS][TERM_COLS];
int current_row = 0;
int current_col = 0;

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

void draw_window(int x, int y, int w, int h, const char *title) {
  draw_rect(x, y, w, h, 0x0022222B);
  draw_rect(x + 2, y + 2, w - 4, 26, 0x001A1A24);
  draw_rect(x + 6, y + 34, w - 12, h - 40, 0x000A0A0F);
  draw_string(x + 12, y + 8, title, 0x00FFFFFF);
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
  draw_window(term_box_x, term_box_y, term_box_width, term_box_height,
              "CONSOLE TERMINAL");
}

void redraw_terminal_text(void) {
  draw_rect(term_box_x + 6, term_box_y + 34, term_box_width - 12,
            term_box_height - 40, 0x000A0A0F);
  for (int r = 0; r < TERM_ROWS; r++) {
    for (int c = 0; c < TERM_COLS; c++) {
      if (terminal_buffer[r][c] == '\0')
        continue;
      draw_char(term_box_x + 12 + (c * 8), term_box_y + 40 + (r * 16),
                terminal_buffer[r][c], 0x00FFFFFF);
    }
  }
}

void scroll_terminal(void) {
  for (int r = 0; r < TERM_ROWS - 1; r++) {
    for (int c = 0; c < TERM_COLS; c++) {
      terminal_buffer[r][c] = terminal_buffer[r + 1][c];
    }
  }
  for (int c = 0; c < TERM_COLS; c++) {
    terminal_buffer[TERM_ROWS - 1][c] = '\0';
  }
  current_row = TERM_ROWS - 1;
  current_col = 0;
}

void clear_screen(void) {
  for (int r = 0; r < TERM_ROWS; r++) {
    for (int c = 0; c < TERM_COLS; c++) {
      terminal_buffer[r][c] = '\0';
    }
  }
  current_row = 0;
  current_col = 0;
  draw_desktop_and_taskbar();
}

void print_char(char c) {
  if (c == '\n') {
    current_row++;
    current_col = 0;
    if (current_row >= TERM_ROWS) {
      scroll_terminal();
    }
    redraw_terminal_text();
    return;
  }
  if (c == '\b') {
    if (current_col > 0) {
      current_col--;
      terminal_buffer[current_row][current_col] = '\0';
      redraw_terminal_text();
    }
    return;
  }
  if (current_col >= TERM_COLS) {
    current_row++;
    current_col = 0;
    if (current_row >= TERM_ROWS) {
      scroll_terminal();
    }
  }
  terminal_buffer[current_row][current_col] = c;
  current_col++;
  redraw_terminal_text();
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

void trigger_kernel_panic_gui(const char *error_msg) {
  clear_screen_gui(0x00AA0000);
  draw_rect(50, 50, screen_width - 100, 40, 0x001A1A24);
  draw_string(70, 62, "!!! KERNEL PANIC !!!", 0x00FF0000);
  draw_string(50, 130, "A critical system error has occurred.", 0x00FFFFFF);
  draw_string(50, 160, "Error code:", 0x00FFFFFF);
  draw_string(150, 160, error_msg, 0x00FFFFFF);
  draw_string(50, 210, "The system has been halted to prevent damage.",
              0x00FFFFFF);
  while (1) {
    asm volatile("hlt");
  }
}

void start_graphics_terminal(unsigned int *multiboot_info) {
  (void)multiboot_info;
  framebuffer = (unsigned int *)0xFD000000;
  screen_width = 1024;
  screen_height = 768;
  clear_screen();
  print_string("SECUREOS READY.\n> ");
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
