#include "include/kernel.h"
#include "include/font.h"
#include "include/io.h"
#include "include/memory.h"
#include <stdarg.h>

extern unsigned char logo_data[];

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
int start_menu_open = 0;

int taskbar_height = 40;
int start_btn_width = 60;
int start_btn_height = 28;
int start_btn_x = 6;

int system_seconds = 0;
int system_minutes = 0;
unsigned char last_known_second = 0xFF;

int mouse_x = 512;
int mouse_y = 384;
int old_mouse_x = 512;
int old_mouse_y = 384;
unsigned char mouse_cycle = 0;
char mouse_byte[3];

extern int strcmp(const char *str1, const char *str2);
extern void process_command(void);

struct idt_entry {
  unsigned short low_offset;
  unsigned short selector;
  unsigned char zero;
  unsigned char flags;
  unsigned short high_offset;
} __attribute__((packed));

struct idt_ptr {
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

struct idt_entry idt[32];
struct idt_ptr idtp;

unsigned char keyboard_map[128] = {
    0,   27,   '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,   0xA9};

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

void draw_status_bar(void) {
  while (get_rtc_register(0x0A) & 0x80)
    ;

  unsigned char sec = get_rtc_register(0x00);
  unsigned char min = get_rtc_register(0x02);
  unsigned char hour = get_rtc_register(0x04);
  unsigned char register_b = get_rtc_register(0x0B);

  if (!(register_b & 0x04)) {
    sec = (sec & 0x0F) + ((sec / 16) * 10);
    min = (min & 0x0F) + ((min / 16) * 10);
    hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
  }

  if (!(register_b & 0x02) && (hour & 0x80)) {
    hour = ((hour & 0x7F) + 12) % 24;
  }

  int local_hour = (int)hour + 2;
  if (local_hour >= 24) {
    local_hour -= 24;
  }

  last_known_second = sec;

  draw_rect(screen_width - 210, 16, 200, 20, 0x001A1A24);
  int start_x = screen_width - 150;

  draw_char(start_x, 18, (local_hour / 10) + '0', 0x00FFFFFF);
  draw_char(start_x + 8, 18, (local_hour % 10) + '0', 0x00FFFFFF);
  draw_char(start_x + 16, 18, ':', 0x00FFFFFF);
  draw_char(start_x + 24, 18, ((int)min / 10) + '0', 0x00FFFFFF);
  draw_char(start_x + 32, 18, ((int)min % 10) + '0', 0x00FFFFFF);
  draw_char(start_x + 40, 18, ':', 0x00FFFFFF);
  draw_char(start_x + 48, 18, ((int)sec / 10) + '0', 0x00FFFFFF);
  draw_char(start_x + 56, 18, ((int)sec % 10) + '0', 0x00FFFFFF);
  draw_string(start_x + 70, 18, "CEST", 0x00005B9E);
}

void draw_desktop_and_taskbar(void) {
  clear_screen_gui(0x001A1A24);

  unsigned int taskbar_y = screen_height - taskbar_height;
  draw_rect(0, taskbar_y, screen_width, taskbar_height, 0x00101014);

  unsigned int button_y = taskbar_y + ((taskbar_height - start_btn_height) / 2);
  draw_rect(start_btn_x, button_y, start_btn_width, start_btn_height,
            0x00005B9E);

  draw_string(start_btn_x + 10, button_y + 6, "START", 0x00FFFFFF);
  draw_string(20, 20, "SECUREOS NEXT GEN", 0x00005B9E);

  draw_window(term_box_x, term_box_y, term_box_width, term_box_height,
              "CONSOLE TERMINAL");
  draw_status_bar();
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
      current_row = TERM_ROWS - 1;
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
      current_row = TERM_ROWS - 1;
    }
  }

  if (current_row < TERM_ROWS && current_col < TERM_COLS) {
    terminal_buffer[current_row][current_col] = c;
    current_col++;
  }
  redraw_terminal_text();
}

void print_int(int num) {
  char buf[12];
  int i = 10;
  buf[11] = '\0';

  if (num == 0) {
    print_string("0");
    return;
  }

  int is_negative = 0;
  if (num < 0) {
    is_negative = 1;
    num = -num;
  }

  while (num > 0 && i >= 0) {
    buf[i--] = (num % 10) + '0';
    num /= 10;
  }

  if (is_negative) {
    buf[i--] = '-';
  }

  print_string(&buf[i + 1]);
}

void print_float(float num, int decimals) {
  if (num < 0) {
    print_string("-");
    num = -num;
  }

  int int_part = (int)num;
  print_int(int_part);
  print_string(".");

  float diff = num - (float)int_part;

  for (int i = 0; i < decimals; i++) {
    diff *= 10.0f;
    int dec_digit = (int)diff;
    print_int(dec_digit);
    diff -= (float)dec_digit;
  }
}

void print_unsigned(unsigned int num) {
  char buf[12];
  int i = 10;
  buf[11] = '\0';

  if (num == 0) {
    print_string("0");
    return;
  }

  while (num > 0 && i >= 0) {
    buf[i--] = (num % 10) + '0';
    num /= 10;
  }

  print_string(&buf[i + 1]);
}

void print_string(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    print_char(str[i]);
  }
}

void printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  for (int i = 0; format[i] != '\0'; i++) {
    if (format[i] != '%') {
      print_char(format[i]);
      continue;
    }

    i++;
    if (format[i] == '\0') {
      break;
    }

    switch (format[i]) {
    case 'c': {
      char c = (char)va_arg(args, int);
      print_char(c);
      break;
    }
    case 's': {
      char *s = va_arg(args, char *);
      if (s == 0)
        s = "(null)";
      print_string(s);
      break;
    }
    case 'd':
    case 'i': {
      int d = va_arg(args, int);
      print_int(d);
      break;
    }
    case 'u': {
      unsigned int u = va_arg(args, unsigned int);
      print_unsigned(u);
      break;
    }
    case '%': {
      print_char('%');
      break;
    }
    default: {
      print_char('%');
      print_char(format[i]);
      break;
    }
    }
  }

  va_end(args);
}

void sleep_ms(int milliseconds) {
  for (int m = 0; m < milliseconds; m++) {
    for (int u = 0; u < 1000; u++) {
      asm volatile("outb %%al, $0x80" ::: "ax");
    }
  }
}

void trigger_kernel_panic_gui(const char *error_msg) {
  asm volatile("cli");
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

void exception_divide_by_zero(void) {
  trigger_kernel_panic_gui("DIVISION_BY_ZERO_EXCEPTION");
}

void init_idt(void) {
  unsigned int base = (unsigned int)exception_divide_by_zero;
  idt[0].low_offset = base & 0xFFFF;
  idt[0].selector = 0x08;
  idt[0].zero = 0;
  idt[0].flags = 0x8E;
  idt[0].high_offset = (base >> 16) & 0xFFFF;

  idtp.limit = (sizeof(struct idt_entry) * 32) - 1;
  idtp.base = (unsigned int)&idt;
  asm volatile("lidt (%0)" : : "r"(&idtp));
}

unsigned char get_rtc_register(int reg) {
  asm volatile("outb %0, $0x70" : : "a"((unsigned char)reg));
  unsigned char val;
  asm volatile("inb $0x71, %0" : "=a"(val));
  return val;
}

void draw_start_menu(void) {
  int menu_x = 6;
  int menu_w = 200;
  int menu_h = 250;
  int menu_y = screen_height - 40 - menu_h - 6;
  if (start_menu_open) {
    draw_rect(menu_x, menu_y, menu_w, menu_h, 0x0022222B);
    draw_rect(menu_x + 2, menu_y + 2, menu_w - 4, menu_h - 4, 0x001A1A24);

    draw_string(menu_x + 15, menu_y + 20, "--- APPS ---", 0x00005B9E);
    draw_string(menu_x + 15, menu_y + 60, "1. Terminal", 0x00FFFFFF);
    draw_string(menu_x + 15, menu_y + 90, "2. Settings", 0x00888888);
    draw_string(menu_x + 15, menu_y + 120, "3. Games", 0x00888888);

    draw_rect(menu_x + 10, menu_y + 160, menu_w - 20, 2, 0x0022222B);
    draw_string(menu_x + 15, menu_y + 180, "Press ESC to close", 0x00888888);
  } else {
    draw_rect(menu_x, menu_y, menu_w, menu_h, 0x001A1A24);

    redraw_terminal_text();
    draw_window(term_box_x, term_box_y, term_box_width, term_box_height,
                "CONSOLE TERMINAL");
    redraw_terminal_text();
  }
}

int sin_table[36] = {0,    44,   87,   128,  164,  196,  221,  240,  252,
                     256,  252,  240,  221,  196,  164,  128,  87,   44,
                     0,    -44,  -87,  -128, -164, -196, -221, -240, -252,
                     -256, -252, -240, -221, -196, -164, -128, -87,  -44};

int cos_table[36] = {256,  252,  240,  221,  196,  164,  128,  87,   44,
                     0,    -44,  -87,  -128, -164, -196, -221, -240, -252,
                     -256, -252, -240, -221, -196, -164, -128, -87,  -44,
                     0,    44,   87,   128,  164,  196,  221,  240,  252};

void draw_bootscreen_animation(void) {
  clear_screen_gui(0x000A0A0F);

  int center_x = screen_width / 2;
  int center_y = screen_height / 2;

  int img_w = 64;
  int img_h = 64;
  int start_x = center_x - (img_w / 2);
  int start_y = center_y - (img_h / 2) - 60;

  unsigned int raw_idx = 0;

  for (int y = 0; y < img_h; y++) {
    for (int x = 0; x < img_w; x++) {
      unsigned char r = logo_data[raw_idx];
      unsigned char g = logo_data[raw_idx + 1];
      unsigned char b = logo_data[raw_idx + 2];

      unsigned int color = (r << 16) | (g << 8) | b;
      draw_pixel(start_x + x, start_y + y, color);

      raw_idx += 4;
    }
  }

  int anim_center_y = center_y + 60;
  int radius = 35;
  for (int step = 0; step < 36; step++) {
    int x_offset = (radius * cos_table[step]) / 256;
    int y_offset = (radius * sin_table[step]) / 256;

    draw_rect(center_x + x_offset - 2, anim_center_y + y_offset - 2, 5, 5,
              0x00005B9E);
    sleep_ms(1000);
  }
}

void mouse_wait(unsigned char type) {
  unsigned int timeout = 100000;
  if (type == 0) {
    while (timeout--) {
      if ((char)(get_rtc_register(0x64) & 1) == 1)
        return;
    }
  } else {
    while (timeout--) {
      if ((char)(get_rtc_register(0x64) & 2) == 0)
        return;
    }
  }
}

void mouse_write(unsigned char a) {
  mouse_wait(1);
  asm volatile("outb %0, $0x64" : : "a"((unsigned char)0xD4));
  mouse_wait(1);
  asm volatile("outb %0, $0x60" : : "a"(a));
}

void init_mouse(void) {
  unsigned char status;

  mouse_wait(1);
  asm volatile("outb %0, $0x64" : : "a"((unsigned char)0xA8));
  mouse_wait(1);
  asm volatile("outb %0, $0x64" : : "a"((unsigned char)0x20));
  mouse_wait(0);
  asm volatile("inb $0x60, %0" : "=a"(status));

  status |= 2;
  mouse_wait(1);
  asm volatile("outb %0, $0x64" : : "a"((unsigned char)0x60));
  mouse_wait(1);
  asm volatile("outb %0, $0x60" : : "a"(status));

  mouse_write(0xF4);
}

void draw_mouse_pointer(int x, int y, unsigned int color) {
  draw_pixel(x, y, color);
  draw_pixel(x + 1, y, color);
  draw_pixel(x, y + 1, color);
  draw_pixel(x + 2, y, color);
  draw_pixel(x + 1, y + 1, color);
  draw_pixel(x, y + 2, color);
  draw_pixel(x + 3, y, color);
  draw_pixel(x, y + 3, color);
  draw_pixel(x + 4, y, color);
  draw_pixel(x + 2, y + 2, color);
  draw_pixel(x, y + 4, color);
  draw_pixel(x + 5, y, color);
  draw_pixel(x, y + 5, color);
  draw_pixel(x + 6, y, color);
  draw_pixel(x + 3, y + 3, color);
  draw_pixel(x, y + 6, color);
}

void init_fpu(void) {
  unsigned int cr0;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 &= ~(1 << 2);
  cr0 |= (1 << 1);
  asm volatile("mov %0, %%cr0" : : "r"(cr0));

  unsigned int cr4;
  asm volatile("mov %%cr4, %0" : "=r"(cr4));
  cr4 |= (3 << 9);
  asm volatile("mov %0, %%cr4" : : "r"(cr4));

  asm volatile("finit");
}

void play_beep(int frequency, int duration) {
  unsigned int divisor = 1193180 / frequency;

  outb(0x43, 0xB6);

  outb(0x42, divisor & 0xFF);
  outb(0x42, (divisor >> 8) & 0xFF);

  unsigned int tmp = inb(0x61);
  outb(0x61, tmp | 3);

  sleep_ms(duration);

  tmp = inb(0x61);
  outb(0x61, tmp & ~3);
}

void start_graphics_terminal(unsigned int *multiboot_info) {
  (void)multiboot_info;
  framebuffer = (unsigned int *)0xFD000000;
  screen_width = 1024;
  screen_height = 768;

  draw_bootscreen_animation();

  init_idt();
  init_memory();
  init_fpu();
  init_mouse();
  play_beep(440, 1000);

  unsigned char dummy = 0;

  while (1) {
    unsigned char status = 0;
    asm volatile("inb $0x64, %0" : "=a"(status));
    if (status & 1) {
      asm volatile("inb $0x60, %0" : "=a"(dummy));
    } else {
      break;
    }
  }
  (void)dummy;

  clear_screen();
  print_string("SECUREOS READY.\n> ");
  int shift_pressed = 0;
  int caps_lock = 0;

  draw_status_bar();

  draw_mouse_pointer(mouse_x, mouse_y, 0x00FFFFFF);

  while (1) {
    unsigned char current_sec = get_rtc_register(0x00);
    if (current_sec != last_known_second) {
      draw_status_bar();
      draw_mouse_pointer(mouse_x, mouse_y, 0x00FFFFFF);
    }

    unsigned char status = 0;
    asm volatile("inb $0x64, %0" : "=a"(status));

    if (status & 1) {
      if ((status & 0x20) != 0) {
        unsigned char data = 0;
        asm volatile("inb $0x60, %0" : "=a"(data));

        mouse_byte[mouse_cycle] = data;
        mouse_cycle++;

        if (mouse_cycle == 3) {
          mouse_cycle = 0;

          if ((mouse_byte[0] & 0x08) == 0) {
            continue;
          }

          int btn_top = (screen_height - taskbar_height) +
                        ((taskbar_height - start_btn_height) / 2);
          int btn_bottom = btn_top + start_btn_height;
          int btn_left = start_btn_x;
          int btn_right = start_btn_x + start_btn_width;

          int menu_x = 6;
          int menu_w = 200;
          int menu_h = 250;
          int menu_y = screen_height - taskbar_height - menu_h - 6;

          for (int wy = old_mouse_y - 2; wy < old_mouse_y + 10; wy++) {
            for (int wx = old_mouse_x - 2; wx < old_mouse_x + 10; wx++) {
              if (wx >= 0 && wx < (int)screen_width && wy >= 0 &&
                  wy < (int)screen_height) {
                unsigned int repair_color = 0x001A1A24;

                if (wx >= term_box_x && wx <= term_box_x + term_box_width &&
                    wy >= term_box_y && wy <= term_box_y + term_box_height) {
                  repair_color = 0x000A0A0F;
                } else if (start_menu_open && wx >= menu_x &&
                           wx <= (menu_x + menu_w) && wy >= menu_y &&
                           wy <= (menu_y + menu_h)) {
                  if (wx < menu_x + 2 || wx > (menu_x + menu_w) - 2 ||
                      wy < menu_y + 2 || wy > (menu_y + menu_h) - 2) {
                    repair_color = 0x0022222B;
                  } else {
                    repair_color = 0x001A1A24;
                  }
                } else if (wy >= (int)screen_height - taskbar_height) {
                  if (wx >= btn_left && wx <= btn_right && wy >= btn_top &&
                      wy <= btn_bottom) {
                    repair_color = 0x00005B9E;
                  } else {
                    repair_color = 0x00101014;
                  }
                }

                draw_pixel(wx, wy, repair_color);
              }
            }
          }

          int move_x = (int)mouse_byte[1];
          int move_y = (int)mouse_byte[2];

          if (mouse_byte[0] & 0x10)
            move_x |= 0xFFFFFF00;
          if (mouse_byte[0] & 0x20)
            move_y |= 0xFFFFFF00;

          mouse_x += move_x / 2;
          mouse_y -= move_y / 2;

          if (mouse_x < 0)
            mouse_x = 0;
          if (mouse_y < 0)
            mouse_y = 0;
          if (mouse_x >= (int)screen_width)
            mouse_x = screen_width - 1;
          if (mouse_y >= (int)screen_height)
            mouse_y = screen_height - 1;

          if ((mouse_byte[0] & 1) != 0) {
            if (mouse_x >= btn_left && mouse_x <= btn_right &&
                mouse_y >= btn_top && mouse_y <= btn_bottom) {
              start_menu_open = !start_menu_open;
              draw_start_menu();
              sleep_ms(150);
            }
          }

          draw_mouse_pointer(mouse_x, mouse_y, 0x00FFFFFF);

          old_mouse_x = mouse_x;
          old_mouse_y = mouse_y;
        }
      } else {
        unsigned char scancode = 0;
        asm volatile("inb $0x60, %0" : "=a"(scancode));

        if (scancode == 0xFA || scancode == 0xFE) {
          continue;
        }

        if (scancode == 0x2A || scancode == 0x36) {
          shift_pressed = 1;
          continue;
        } else if (scancode == 0xAA || scancode == 0xB6) {
          shift_pressed = 0;
          continue;
        } else if (scancode == 0x3A) {
          caps_lock = !caps_lock;
          continue;
        }

        if (!(scancode & 0x80)) {
          if (scancode == 0x01) {
            start_menu_open = !start_menu_open;
            draw_start_menu();
            draw_mouse_pointer(mouse_x, mouse_y, 0x00FFFFFF);
            continue;
          }
          char c = keyboard_map[scancode];
          if (c != 0) {
            if (c >= 'a' && c <= 'z') {
              if (shift_pressed ^ caps_lock) {
                c = c - 32;
              }
            } else if (shift_pressed) {
              if (c == '1') {
                c = '!';
              } else if (c == '2') {
                c = '"';
              } else if (c == '3') {
                c = 39;
              } else if (c == '4') {
                c = '$';
              } else if (c == '5') {
                c = '%';
              } else if (c == '6') {
                c = '&';
              } else if (c == '7') {
                c = '/';
              } else if (c == '8') {
                c = '(';
              } else if (c == '9') {
                c = ')';
              } else if (c == '0') {
                c = '=';
              } else if (c == '-') {
                c = '_';
              } else if (c == '=') {
                c = '+';
              } else if (c == '/') {
                c = '*';
              }
            }

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
            draw_mouse_pointer(mouse_x, mouse_y, 0x00FFFFFF);
            sleep_ms(10);
          }
        }
      }
    }
  }
}
